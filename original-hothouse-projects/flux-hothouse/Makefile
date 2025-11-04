# Project Name
TARGET = ambien_flux

# Sources
CPP_SOURCES = ambien_flux.cpp hothouse.cpp

# Library Locations (git repository structure)
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Compiler optimization
OPT = -O3

# Uncomment for QSPI boot (production mode - requires bootloader)
# APP_TYPE = BOOT_QSPI

# Comment out for SRAM boot (development mode - no bootloader needed)
# Leave commented for initial development
# APP_TYPE = BOOT_SRAM
