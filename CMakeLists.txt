cmake_minimum_required(VERSION 3.11)
# Project name
project("avr-hex")

# Product filename
set(PRODUCT_NAME avr-hex)

## AVR Chip Configuration
set(F_CPU 8000000UL)

# CPU, you can find the list here:
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
set(MCU attiny84)

# Default Baudrate for UART, read avr include/util/setbaud.h for usage
# set(BAUD 9600)

# The programmer to use, read avrdude manual for list
set(PROG_TYPE usbasp)

# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://www.engbedded.com/fusecalc/
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=attiny85
set(L_FUSE 0xE2)
set(H_FUSE 0xDF)
set(E_FUSE 0xFF)
set(STD_LOCK_BIT 0xFF)
set(REL_LOCK_BIT 0xFC)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Pass defines to compiler
add_definitions(
    -DF_CPU=${F_CPU}
#    -DBAUD=${BAUD}
)
# mmcu MUST be passed to bot the compiler and linker, this handle the linker
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

add_compile_options(
    -mmcu=${MCU}  # MCU
    -std=gnu99    # GNU99 C
    -O2           # optimize
    -pedantic
    -Wall         # enable warnings
    -Wno-main
    -Wundef
    -Wfatal-errors
    # -Wl,--relax,--gc-sections
    -g
    # -gdwarf-2
    # a few optimizations
    -funsigned-char
    -funsigned-bitfields
    -fpack-struct
    -fshort-enums
    -ffunction-sections
    -fdata-sections
    #
    # -fno-split-wide-types
    # -fno-tree-scev-cprop
    #
    # C
    # -Wstrict-prototypes
)

# Load all files in src folder
file(GLOB SRC_FILES "src/*.c") 

# Create one target
add_executable(${PRODUCT_NAME} ${SRC_FILES})

# Rename the output to .elf as we will create multiple files
set_target_properties(${PRODUCT_NAME} PROPERTIES OUTPUT_NAME ${PRODUCT_NAME}.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip ${PRODUCT_NAME}.elf DEPENDS ${PRODUCT_NAME})

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.hex DEPENDS strip)

# Transform binary into hex file, this is the eeprom part (empty if you don't use eeprom static variables)
add_custom_target(eeprom avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.eep DEPENDS strip)

# Upload the firmware with avrdude
add_custom_target(upload avrdude -c ${PROG_TYPE} -p ${MCU} -U flash:w:${PRODUCT_NAME}.hex DEPENDS hex)

# Print stats on compiled hex
add_custom_target(stats avr-size -C --mcu=${MCU} --format=avr ${PRODUCT_NAME}.elf DEPENDS strip)

# Upload the eeprom with avrdude
add_custom_target(upload_eeprom avrdude -c ${PROG_TYPE} -p ${MCU} -U eeprom:w:${PRODUCT_NAME}.eep DEPENDS eeprom)

# Burn fuses
add_custom_target(fuses avrdude -B16 -c ${PROG_TYPE} -p ${MCU} -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${STD_LOCK_BIT}:m)
add_custom_target(lock avrdude -B16 -c ${PROG_TYPE} -p ${MCU} -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m -U efuse:w:${E_FUSE}:m -U lock:w:${REL_LOCK_BIT}:m)

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PRODUCT_NAME}.hex;${PRODUCT_NAME}.eeprom;${PRODUCT_NAME}.lst")
