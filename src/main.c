
#include "peripheral/gpio.h"
#include "peripheral/watchdog.h"
#include "internal/alloc.h"
#include "peripheral/uart.h"
#include "peripheral/pwm.h"
#include "peripheral/spi.h"
#include "internal/mmio.h"
#include "internal/led.h"
#include "peripheral/systick.h"

#define USR_BUTTON 9
#define GREEN_LED 49
#define YELLOW_LED 139
#define RED_LED 74


/**
 * delay is # of clock cycles, not a time unit
 */
void delay(int delay) {
  for (int i = 0; i < delay; i++) {
    asm("nop");
  }
}

extern uint32_t __heap_start;
// void test_spi() {
//     led_init(GREEN);
//     led_init(RED);
//     systick_init();

//     uint8_t instance = 2;

//     spi_config_t config = {
//         .clk_pin = 98,
//         .mosi_pin = 75, // B15
//         .miso_pin = 74,
//         .data_size = 8,
//         .mode = 0,
//         .baudrate_prescaler = 2,
//         .first_bit = 0,
//         .priority = 0,
//         .mutex_timeout = 0
//     };

//     spi_init(instance, &config);

//     asm("BKPT #0");

//     uint8_t tx_byte = 0;
//     uint8_t rx_byte = 0;

//     spi_device_t device = {
//         .gpio_pin = 0x0,
//         .instance = 2,
//     };

//     struct spi_sync_transfer_t transfer = {
//         .device = device,
//         .source = &tx_byte,
//         .dest = &rx_byte,
//         .size = 1,
//         .timeout = 10000000,
//         .read_inc = false,
//     };

//     asm("BKPT #0");

//     while (1) {
//         for (int i = 1; i < 255; i++) {
//             tx_byte = i; 
//             spi_transfer_sync(&transfer); 
            
//             if (rx_byte == i - 1) {
//                 toggle_led(GREEN);
//             } else {
//                 toggle_led(RED);
//             }
//             systick_delay(1000); 
//         }
//     }
// }

void test_uart(){
    uart_config_t config;
    uart_channel_t channel = UART8;
    uart_parity_t parity = UART_PARITY_DISABLED;
    uart_datalength_t data_length = UART_DATALENGTH_8;
    config.channel = channel;
    config.parity = parity;
    config.data_length = data_length;
    config.baud_rate = 9600;
    config.clk_freq = 4000000; // 60 MHz

    // UART 1-3, 6 is fine (maybe)
    // UART 4-5, probably 7/8 isn't working (maybe)

    // asm("BKPT #0");
    int n = uart_init(&config, (void*) (0), (void*) (0), (void*) (0));
    asm("BKPT #0");
    // int n = 1; 

    // UART TX TEST (channels 4, 5, 7, 8)
    systick_init();
    led_init(GREEN);
    led_init(YELLOW);
    led_init(RED);
          
    uint8_t tx_num = 0xCC;
    led_countdown(1);
    while (1){
        uart_write_blocking(channel, &tx_num, 1);
    }
    
    // for (int data = 55; data < 255; data += 3) {
    //     tx_num = data;
        
    //     uart_write_blocking(channel, &tx_num, 1);
        
    //     asm("BKPT #0");
    // }
    



    // uart_write_blocking(channel, nums, 10);
    // asm("BKPT #0");
    // testing reading one byte
    // uint8_t buff[10];
    // int reading = uart_read_blocking(channel, buff, 10);
    // asm("BKPT #0");

    // // multiple byte
    // uint8_t buff[10];
    // uart_read_blocking(channel, buff, 10);
    // asm("BKPT #0");
}


void test_pwm() {
    // test diff instances (channel)
        // struct ti_pwm_config_t pwm_config = {
        //         .instance = 3,
        //         .channel = 1, 

        //         .freq = 40, 
        //         .clock_freq = 2000000,
        //         .duty = 500,
        //     };
    WRITE_FIELD(RCC_D2CFGR, RCC_D2CFGR_D2PPREx[1], 0b111);
    WRITE_FIELD(RCC_D2CFGR, RCC_D2CFGR_D2PPREx[2], 0b111);
    WRITE_FIELD(RCC_D1CFGR, RCC_D1CFGR_HPRE, 0b1001);
    // enum ti_errc_t my_err;
    // enum ti_errc_t* errc = &my_err;
    asm("BKPT #0");
    // while (true) {
    //     ti_set_pwm(pwm_config, errc);
    //     enum ti_errc_t err = *errc;
    //     // asm("BKPT #0");
    //     // pwm_config.duty += dir;
    //     // if (pwm_config.duty == 1000 || pwm_config.duty == 0) {
    //     //     dir *= -1;
    //     // }
    //     delay(1000);
    // } 
    // for (int i = 2; i <= 5; i+=3) {
    //     for (int j = 1; j <= 4; j++) {

    //             struct ti_pwm_config_t pwm_config = {
    //             .instance = i,
    //             .channel = j, 

    //             .freq = 40, 
    //             .clock_freq = 2000000,
    //             .duty = 500,
    //         };
        
    //         //int32_t dir = 1;
    //         enum ti_errc_t my_err;
    //         enum ti_errc_t* errc = &my_err;

            
    //         // while (true) {
    //         for (int k = 0; k < 40; k++) {
    //             ti_set_pwm(pwm_config, errc);
    //             enum ti_errc_t err = *errc;
     
    //             // asm("BKPT #0");
    //             // pwm_config.duty += dir;
    //             // if (pwm_config.duty == 1000 || pwm_config.duty == 0) {
    //             //     dir *= -1;
    //             // }
    //             delay(1000);
    //         } 
    //         asm("BKPT #0");
    //     }
            
    // }
    struct ti_pwm_config_t pwm_config = {
        .instance = 2, 
        .channel = 1,
        .freq = 40,
        .duty = 500,
        .clock_freq = 2000000,
    };

    enum ti_errc_t err;
    enum ti_errc_t* errc = &err;

    while (true) {
        ti_set_pwm(pwm_config, errc);
        enum ti_errc_t err = *errc;

        delay(1000);
    }
}

//Stuff for blinky
#define RCC_BASE 0x40023800
#define GPIOA_BASE 0x40020000
#define GPIOC_BASE 0x40020800

typedef struct {
    volatile uint32_t DUMMY[12];
    volatile uint32_t AHB1ENR; //offset: 0x30
}RCC_t;

typedef struct {
    volatile uint32_t MODER; //offset 0x00
    volatile uint32_t OTYPER; //offset 0x04
    volatile uint32_t DUMMY[2];
    volatile uint32_t PUPDR; //offset 0x0C
    volatile uint32_t IDR; //offset 0x10
    volatile uint32_t ODR; //offset 0x14
}GPIO_t;

#define RCC ((RCC_t *) RCC_BASE)
#define GPIOA ((GPIO_t *) GPIOA_BASE)
#define GPIOC ((GPIO_t *) GPIOC_BASE)



void blinky() {
    //Enable GPIOA and GPIOC clock
    RCC->AHB1ENR |= 1U << 0;
    RCC->AHB1ENR |= 1U << 2;

    //Set pin PC13 to input
    GPIOC->MODER &= ~(1U << 26);
    GPIOC->MODER &= ~(1U << 27);

    //Pull up pin PC13 (push-button)
    GPIOC->PUPDR |= 1U << 26;
    GPIOC->PUPDR &= ~(1U << 27);

    //Set pin PA5 to output
    GPIOA->MODER |= 1U << 10;
    GPIOA->MODER &= ~(1U << 11);

    while (1) {
        // If button is pressed (IDR bit 13 goes to 0)
        if (!(GPIOC->IDR & (1 << 13))) {
            GPIOA->ODR ^= (1U << 5); // Toggle LED
            for(int i=0; i<100000; i++); // Delay so you can see the blink
        } else {
            GPIOA->ODR &= ~(1U << 5); // Turn off if not pressed
        }
    }
}

void _start() {

    // struct ti_pwm_config_t pwm_config = {
    //     .channel = 3,
    //     .alt_num = 2,
    //     .pin = 49,
    //     .freq = 100,
    //     .duty = 0,
    // };

    // int32_t dir = 1;
    // enum ti_errc_t my_err;
    // enum ti_errc_t* errc = &my_err;
    // asm("BKPT #0");
    // while (true) {
    //     ti_set_pwm(3, pwm_config, errc);
    //     enum ti_errc_t err = *errc;
    //     // asm("BKPT #0");
    //     pwm_config.duty += dir;
    //     if (pwm_config.duty == 1000 || pwm_config.duty == 0) {
    //         dir *= -1;
    //     }
    //     delay(1000);
    // }

    // test_pwm_first_principles();
    // tal_enable_clock(GREEN_LED);
    // tal_enable_clock(RED_LED);
    // tal_enable_clock(YELLOW_LED);
    // tal_enable_clock(USR_BUTTON);


    // tal_set_mode(GREEN_LED, 1);
    // tal_set_mode(RED_LED, 1);
    // tal_set_mode(YELLOW_LED, 1);
    // tal_set_mode(USR_BUTTON, 0);
    
    // HEAP_START = &__heap_start;
    // int heap_status = init_heap();

    // if(heap_status != 1){
    //     asm("BKPT #0"); // heap init failure 
    // }

    // tal_set_pin(YELLOW_LED, 1);
    

    //test_pwm();
    test_uart();
    //test_spi();
    //blinky();
}