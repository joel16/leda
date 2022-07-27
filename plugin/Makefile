TARGET = leda
OBJS = leda.o exports.o stubs/InterruptManagerForKernel.o stubs/ModuleMgrForUser.o stubs/sceLoaderCore.o

PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 660

# Use the kernel's small inbuilt libc
USE_KERNEL_LIBC = 1
# Use only kernel libraries
USE_KERNEL_LIBS = 1

INCDIR = ../libs/include 
CFLAGS = -Os -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR  = ../libs/lib

LDFLAGS = -nostartfiles
LIBS = -lpspkubridge -lpspsystemctrl_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
