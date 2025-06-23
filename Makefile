# Compiler Settings
CC = gcc

# Compiler Standard Build Flags
CFLAGS_BASE = -std=c99 -Wall -Wextra -Wpedantic -Werror

# Debug Build Flags (includes sanitizers and debugging options)
CFLAGS_DEBUG = $(CFLAGS_BASE) -g -O0 -fno-inline -fno-omit-frame-pointer \
               -fwrapv -fdiagnostics-show-option \
               -fsanitize=undefined,leak,address

# Production Build Flags (optimized for performance and security)
CFLAGS_PROD  = $(CFLAGS_BASE) -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=3 \
               -fPIE -pie -fvisibility=hidden -fno-common \
               -fstack-clash-protection -flto -fno-strict-aliasing \
               -funroll-loops -finline-functions -fno-plt

# Linker Flags for Production
LDFLAGS_PROD = -flto -Wl,-z,relro -Wl,-z,now -Wl,-z,defs -Wl,-z,noexecstack

# Default Target: Builds the 'lscav' executable in the base directory
all: lscav

# Compile the source file into the executable (using base flags)
lscav: lscav.c
	$(CC) $(CFLAGS_BASE) lscav.c -o lscav_base

# Link Object Files into the Debug Executable
debug: lscav.c
	$(CC) $(CFLAGS_DEBUG) lscav.c -o lscav_debug

# Link Object Files into the Production Executable
release: lscav.c
	$(CC) $(CFLAGS_PROD) lscav.c $(LDFLAGS_PROD) -o lscav_release

# Clean Build Artifacts
clean:
	rm -f lscav lscav_base lscav_debug lscav_release

# Phony Targets (Avoid conflicts with filenames)
.PHONY: all clean debug release
