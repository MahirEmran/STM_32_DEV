Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not $IsWindows) {
    Write-Host "build.ps1 must be run on Windows (use build.sh on macOS/Linux)."
    exit 1
}

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $repoRoot
try {
    $openOcdCmd = @(
        "openocd",
        "-f", "interface/stlink.cfg",
        "-f", "target/stm32h7x_dual_bank.cfg",
        "-c", 'program titan.elf verify reset exit'
    )

    $usbResetBin = Join-Path $repoRoot "stm_usb\stm_usb_win.exe"
    if (-not (Test-Path $usbResetBin)) {
        Write-Host "USB reset binary not found: $usbResetBin"
        exit 2
    }

    function Invoke-UsbReset {
        Write-Host "Running USB reset ($usbResetBin)..."
        & $usbResetBin
        if ($LASTEXITCODE -ne 0) {
            Write-Host "USB reset failed (rc=$LASTEXITCODE). Aborting."
            exit 3
        }
    }

    function Invoke-Stm32CliRecovery {
        $cliBin = (Get-Command "STM32_Programmer_CLI.exe" -ErrorAction SilentlyContinue)?.Source
        if (-not $cliBin) {
            $cliBin = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
        }
        if (-not (Test-Path $cliBin)) {
            Write-Host "STM32_Programmer_CLI.exe not found at $cliBin"
            return $false
        }

        Write-Host "Found STM32 CLI: $cliBin"
        $steps = @(
            @{Args=@("-c","port=SWD","-d","0"); Error="CLI connect failed"},
            @{Args=@("-c","port=SWD","-d","0","-srst_only"); Error="CLI soft-reset failed"},
            @{Args=@("-c","port=SWD","-d","0","-e","all"); Error="CLI erase failed"},
            @{Args=@("-c","port=SWD","-d","0","-dis"); Error="CLI disconnect failed"}
        )

        foreach ($step in $steps) {
            & $cliBin $step.Args
            if ($LASTEXITCODE -ne 0) {
                Write-Host "$($step.Error) (rc=$LASTEXITCODE)"
                return $false
            }
        }
        return $true
    }

    Invoke-UsbReset

    Push-Location "src"
    try {
        New-Item -ItemType Directory -Path "build" -Force | Out-Null
        Push-Location "build"
        try {
            Write-Host "Running cmake .."
            & cmake ..
            if ($LASTEXITCODE -ne 0) {
                Write-Host "cmake failed"
                exit 20
            }

            Write-Host "Running make"
            & make
            if ($LASTEXITCODE -ne 0) {
                Write-Host "make failed"
                exit 21
            }

            $maxAttempts = 3
            $flashSuccess = $false

            for ($attempt = 1; $attempt -le $maxAttempts; $attempt++) {
                Write-Host "Flashing with OpenOCD (attempt $attempt/$maxAttempts)..."
                $openocdOutputFile = [System.IO.Path]::GetTempFileName()
                & $openOcdCmd[0] $openOcdCmd[1..($openOcdCmd.Count - 1)] *> $openocdOutputFile
                $openocdRc = $LASTEXITCODE
                $outputRaw = Get-Content -Path $openocdOutputFile -Raw
                Remove-Item $openocdOutputFile -ErrorAction SilentlyContinue
                $output = ($outputRaw -replace "`r","") -replace "`e\[[0-9;]*m",""

                if ($output -match "timeout waiting for algorithm") {
                    Write-Host "Timeout waiting for algorithm. Attempting automated recovery..."
                    if (-not (Invoke-Stm32CliRecovery)) {
                        Write-Host "Automated recovery failed."
                        break
                    }

                    Invoke-UsbReset
                    Write-Host "Recovery via STM32CubeProgrammer CLI complete — retrying OpenOCD flash..."
                    continue
                }

                if ($output -match "checksum mismatch") {
                    Write-Host "OpenOCD reported checksum mismatch. Running USB reset and retrying..."
                    Invoke-UsbReset
                    continue
                }

                if ($openocdRc -eq 0) {
                    Write-Host "Flash successful."
                    $flashSuccess = $true
                    break
                }
            }

            if (-not $flashSuccess) {
                Write-Host "OpenOCD did not succeed after $maxAttempts attempts."
                exit 32
            }
        } finally {
            Pop-Location
        }
    } finally {
        Pop-Location
    }
} finally {
    Pop-Location
}
