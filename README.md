https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

**The directions below are made in case the ```build.ps1``` or ```build.sh``` files do not work. If you are on MacOS/Linux, running ```./build.sh``` should work. If you are on Windows, hopefully (I pray I truly hope) that ```.\build.ps1``` should work.**

Instructions to build and flash:
Make a build folder in the src directory (yes, it should be outside, yes, I will fix)
cd into this build folder
run "```cmake ..```" (This only needs to be run once, in theory)
run "```make```" (This needs to be run each time you want to flash, this rebuilds the project)
run "```openocd -f interface/stlink.cfg -f target/stm32h7x_dual_bank.cfg -c "program titan.elf verify reset exit"```" (note that you need a quote on the end, so the double quote at the end of that line is intentional).  

Instructions to make and run tests for memory allocator:
Make sure ./test/test_alloc.c exists, and that you are in root folder (NOT src or build)
Run ```gcc -std=c18 -Wall -Wextra ./src/internal/alloc.c ./test/test_alloc.c -o src/build/test_alloc```
* This will make an executable ``test_alloc`` in ``./src/build/``. You do not have to run ```cmake ..``` before doing this.

Then run ```./src/build/test_alloc```
* Still working on cleaning up output, but [OK] means it passed, [FAIL] means failure. The failures are summarized at the bottom (hopefully will have better output later).