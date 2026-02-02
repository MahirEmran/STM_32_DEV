/**
 * This file is part of the Titan Flight Computer Project
 * Copyright (c) 2025 UW SARP
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * @file peripheral/actuator.c
 * @authors GitHub Copilot
 * @brief Driver implementation for MAX22216/MAX22217 solenoid/actuator controller.
 */

#include "peripheral/actuator.h"
#include "peripheral/gpio.h"

#define MAX22216_SPI_DUMMY_DATA 0x0000
#define MAX22216_SPI_RW_BIT     0x80

#define MAX22216_CH_STRIDE      0x0E
#define MAX22216_CH0_BASE       0x09

#define MAX22216_CH_REG_DC_L2H   0x00
#define MAX22216_CH_REG_DC_H     0x01
#define MAX22216_CH_REG_DC_L     0x02
#define MAX22216_CH_REG_TIME_L2H 0x03
#define MAX22216_CH_REG_CTRL0    0x04
#define MAX22216_CH_REG_CTRL1    0x05

#define MAX22216_IMONITOR_CH0    0x45
#define MAX22216_IMONITOR_CH1    0x50
#define MAX22216_IMONITOR_CH2    0x57
#define MAX22216_IMONITOR_CH3    0x60

static inline bool max22216_channel_valid(max22216_channel_t channel) {
  return channel >= MAX22216_CHANNEL_0 && channel < MAX22216_CHANNEL_COUNT;
}

static inline uint8_t max22216_channel_base(max22216_channel_t channel) {
  return (uint8_t)(MAX22216_CH0_BASE + (MAX22216_CH_STRIDE * channel));
}

static inline uint8_t max22216_channel_reg(max22216_channel_t channel,
                                           uint8_t offset) {
  return (uint8_t)(max22216_channel_base(channel) + offset);
}

static inline uint8_t max22216_imonitor_reg(max22216_channel_t channel) {
  switch (channel) {
  case MAX22216_CHANNEL_0:
    return MAX22216_IMONITOR_CH0;
  case MAX22216_CHANNEL_1:
    return MAX22216_IMONITOR_CH1;
  case MAX22216_CHANNEL_2:
    return MAX22216_IMONITOR_CH2;
  case MAX22216_CHANNEL_3:
    return MAX22216_IMONITOR_CH3;
  default:
    return MAX22216_IMONITOR_CH0;
  }
}

static enum ti_errc_t max22216_spi_transfer(max22216_t *dev, uint8_t addr,
                                            bool write, uint16_t data_in,
                                            uint16_t *data_out,
                                            uint8_t *status_out) {
  if (dev == NULL)
    return TI_ERRC_INVALID_ARG;

  uint8_t tx[3] = {0};
  uint8_t rx[3] = {0};

  tx[0] = (uint8_t)((write ? MAX22216_SPI_RW_BIT : 0x00) | (addr & 0x7F));
  tx[1] = (uint8_t)(data_in >> 8);
  tx[2] = (uint8_t)(data_in & 0xFF);

  if (dev->spi_device.gpio_pin != 0) {
    tal_set_pin(dev->spi_device.gpio_pin, 0);
  }

  struct spi_sync_transfer_t transfer = {
      .device = dev->spi_device,
      .source = tx,
      .dest = rx,
      .size = sizeof(tx),
      .timeout = 1000000,
      .read_inc = true,
  };

  enum ti_errc_t errc = spi_transfer_sync(&transfer);

  if (dev->spi_device.gpio_pin != 0) {
    tal_set_pin(dev->spi_device.gpio_pin, 1);
  }

  if (errc != TI_ERRC_NONE) {
    return errc;
  }

  if (status_out != NULL) {
    *status_out = rx[0];
  }
  if (data_out != NULL) {
    *data_out = (uint16_t)((rx[1] << 8) | rx[2]);
  }

  return TI_ERRC_NONE;
}

static enum ti_errc_t max22216_update_reg(max22216_t *dev, uint8_t addr,
                                          uint16_t mask, uint16_t value) {
  uint16_t reg_val = 0;
  enum ti_errc_t errc = max22216_read_reg(dev, addr, &reg_val, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;
  reg_val = (uint16_t)((reg_val & ~mask) | (value & mask));
  return max22216_write_reg(dev, addr, reg_val, NULL);
}

enum ti_errc_t max22216_init(max22216_t *dev, const max22216_config_t *config) {
  if (dev == NULL || config == NULL)
    return TI_ERRC_INVALID_ARG;

  if (config->enable_crc) {
    return TI_ERRC_INVALID_ARG;
  }

  enum ti_errc_t errc = spi_init(config->spi_device.instance,
                                 (spi_config_t *)&config->spi_config);
  if (errc != TI_ERRC_NONE)
    return errc;

  errc = spi_device_init(config->spi_device);
  if (errc != TI_ERRC_NONE)
    return errc;

  dev->spi_device = config->spi_device;
  dev->enable_pin = config->enable_pin;
  dev->fault_pin = config->fault_pin;
  dev->stat0_pin = config->stat0_pin;
  dev->stat1_pin = config->stat1_pin;
  dev->crc_en_pin = config->crc_en_pin;
  dev->enable_crc = config->enable_crc;

  if (dev->enable_pin != 0) {
    tal_enable_clock(dev->enable_pin);
    tal_set_mode(dev->enable_pin, 1);
    tal_set_pin(dev->enable_pin, 0);
  }

  if (dev->fault_pin != 0) {
    tal_enable_clock(dev->fault_pin);
    tal_set_mode(dev->fault_pin, 0);
    tal_pull_pin(dev->fault_pin, 1);
  }

  if (dev->stat0_pin != 0) {
    tal_enable_clock(dev->stat0_pin);
    tal_set_mode(dev->stat0_pin, 0);
    tal_pull_pin(dev->stat0_pin, 0);
  }

  if (dev->stat1_pin != 0) {
    tal_enable_clock(dev->stat1_pin);
    tal_set_mode(dev->stat1_pin, 0);
    tal_pull_pin(dev->stat1_pin, 0);
  }

  if (dev->crc_en_pin != 0) {
    tal_enable_clock(dev->crc_en_pin);
    tal_set_mode(dev->crc_en_pin, 1);
    tal_set_pin(dev->crc_en_pin, 0);
  }

  return TI_ERRC_NONE;
}

enum ti_errc_t max22216_set_enable(max22216_t *dev, bool enable) {
  if (dev == NULL || dev->enable_pin == 0)
    return TI_ERRC_INVALID_ARG;
  tal_set_pin(dev->enable_pin, enable ? 1 : 0);
  return TI_ERRC_NONE;
}

enum ti_errc_t max22216_write_reg(max22216_t *dev, uint8_t addr,
                                 uint16_t value, uint8_t *status_out) {
  return max22216_spi_transfer(dev, addr, true, value, NULL, status_out);
}

enum ti_errc_t max22216_read_reg(max22216_t *dev, uint8_t addr,
                                uint16_t *value, uint8_t *status_out) {
  enum ti_errc_t errc = max22216_spi_transfer(dev, addr, false,
                                              MAX22216_SPI_DUMMY_DATA, NULL,
                                              NULL);
  if (errc != TI_ERRC_NONE)
    return errc;

  return max22216_spi_transfer(dev, addr, false, MAX22216_SPI_DUMMY_DATA,
                               value, status_out);
}

enum ti_errc_t max22216_set_active(max22216_t *dev, bool active) {
  uint16_t mask = (uint16_t)(1u << MAX22216_GLOBAL_CFG_ACTIVE_POS);
  uint16_t value = (uint16_t)((active ? 1u : 0u)
                              << MAX22216_GLOBAL_CFG_ACTIVE_POS);
  return max22216_update_reg(dev, MAX22216_REG_GLOBAL_CFG, mask, value);
}

enum ti_errc_t max22216_set_pwm_master(max22216_t *dev, uint8_t f_pwm_m) {
  if (f_pwm_m > 0x0F)
    return TI_ERRC_INVALID_ARG;
  uint16_t mask = (uint16_t)MAX22216_GLOBAL_CTRL_F_PWM_M_MSK;
  uint16_t value = (uint16_t)(f_pwm_m << MAX22216_GLOBAL_CTRL_F_PWM_M_POS);
  return max22216_update_reg(dev, MAX22216_REG_GLOBAL_CTRL, mask, value);
}

enum ti_errc_t max22216_configure_channel(max22216_t *dev,
                                          max22216_channel_t channel,
                                          const max22216_channel_config_t *cfg) {
  if (dev == NULL || cfg == NULL)
    return TI_ERRC_INVALID_ARG;
  if (!max22216_channel_valid(channel))
    return TI_ERRC_INVALID_ARG;

  uint8_t dc_l2h = max22216_channel_reg(channel, MAX22216_CH_REG_DC_L2H);
  uint8_t dc_h = max22216_channel_reg(channel, MAX22216_CH_REG_DC_H);
  uint8_t dc_l = max22216_channel_reg(channel, MAX22216_CH_REG_DC_L);
  uint8_t time_l2h = max22216_channel_reg(channel, MAX22216_CH_REG_TIME_L2H);
  uint8_t ctrl0 = max22216_channel_reg(channel, MAX22216_CH_REG_CTRL0);
  uint8_t ctrl1 = max22216_channel_reg(channel, MAX22216_CH_REG_CTRL1);

  enum ti_errc_t errc = max22216_write_reg(dev, dc_l2h, cfg->dc_l2h, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;
  errc = max22216_write_reg(dev, dc_h, cfg->dc_h, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;
  errc = max22216_write_reg(dev, dc_l, cfg->dc_l, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;
  errc = max22216_write_reg(dev, time_l2h, cfg->time_l2h, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;

  uint16_t ctrl0_val = 0;
  ctrl0_val |= (uint16_t)((cfg->ctrl_mode & 0x3)
                          << MAX22216_CFG_CTRL0_CTRL_MODE_POS);
  ctrl0_val |= (uint16_t)((cfg->hhf_enable ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_HHF_EN_POS);
  ctrl0_val |= (uint16_t)((cfg->open_load_enable ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_OL_EN_POS);
  ctrl0_val |= (uint16_t)((cfg->h2l_enable ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_H2L_EN_POS);
  ctrl0_val |= (uint16_t)((cfg->ramp_down ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_RDWE_POS);
  ctrl0_val |= (uint16_t)((cfg->ramp_mid ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_RMDE_POS);
  ctrl0_val |= (uint16_t)((cfg->ramp_up ? 1u : 0u)
                          << MAX22216_CFG_CTRL0_RUPE_POS);
  ctrl0_val |= (uint16_t)(cfg->ramp);

  uint16_t ctrl1_val = 0;
  ctrl1_val |= (uint16_t)((cfg->high_side ? 1u : 0u)
                          << MAX22216_CFG_CTRL1_HSNLS_POS);
  ctrl1_val |= (uint16_t)((cfg->pwm_div & 0x3)
                          << MAX22216_CFG_CTRL1_F_PWM_POS);
  ctrl1_val |= (uint16_t)((cfg->t_blank & 0x3)
                          << MAX22216_CFG_CTRL1_T_BLANK_POS);
  ctrl1_val |= (uint16_t)((cfg->slew_rate & 0x3)
                          << MAX22216_CFG_CTRL1_SLEW_POS);
  ctrl1_val |= (uint16_t)((cfg->gain & 0x3)
                          << MAX22216_CFG_CTRL1_GAIN_POS);
  ctrl1_val |= (uint16_t)((cfg->snsf & 0x3)
                          << MAX22216_CFG_CTRL1_SNSF_POS);

  errc = max22216_write_reg(dev, ctrl0, ctrl0_val, NULL);
  if (errc != TI_ERRC_NONE)
    return errc;

  return max22216_write_reg(dev, ctrl1, ctrl1_val, NULL);
}

enum ti_errc_t max22216_set_channel_enable(max22216_t *dev,
                                           max22216_channel_t channel,
                                           bool enable) {
  if (dev == NULL)
    return TI_ERRC_INVALID_ARG;
  if (!max22216_channel_valid(channel))
    return TI_ERRC_INVALID_ARG;

  uint16_t mask = (uint16_t)(1u << MAX22216_GLOBAL_CTRL_CNTL_POS(channel));
  uint16_t value = (uint16_t)((enable ? 1u : 0u)
                              << MAX22216_GLOBAL_CTRL_CNTL_POS(channel));
  return max22216_update_reg(dev, MAX22216_REG_GLOBAL_CTRL, mask, value);
}

enum ti_errc_t max22216_read_status(max22216_t *dev, uint16_t *status,
                                   uint8_t *status_out) {
  if (status == NULL)
    return TI_ERRC_INVALID_ARG;
  return max22216_read_reg(dev, MAX22216_REG_STATUS, status, status_out);
}

enum ti_errc_t max22216_read_fault(max22216_t *dev, uint16_t *fault0,
                                  uint16_t *fault1, uint8_t *status_out) {
  if (fault0 == NULL || fault1 == NULL)
    return TI_ERRC_INVALID_ARG;

  enum ti_errc_t errc = max22216_read_reg(dev, MAX22216_REG_FAULT0, fault0,
                                          status_out);
  if (errc != TI_ERRC_NONE)
    return errc;
  return max22216_read_reg(dev, MAX22216_REG_FAULT1, fault1, status_out);
}

enum ti_errc_t max22216_read_i_monitor(max22216_t *dev,
                                      max22216_channel_t channel,
                                      uint16_t *i_monitor,
                                      uint8_t *status_out) {
  if (i_monitor == NULL)
    return TI_ERRC_INVALID_ARG;
  if (!max22216_channel_valid(channel))
    return TI_ERRC_INVALID_ARG;

  return max22216_read_reg(dev, max22216_imonitor_reg(channel), i_monitor,
                           status_out);
}
