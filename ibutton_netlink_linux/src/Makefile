#################################################
# x86 or arm
BUILD_TYPE = arm

BUILD_NAME = ibutton_netlink_linux

#################################################
ifeq ($(BUILD_TYPE), arm)
	#CROSS_COMPILE	:= /home/lihacker/apps/android-ndk-r6/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-
	CROSS_COMPILE	:= /usr/local/arm/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-
endif
#################################################
# Make variables (CC, etc...)

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

RM 		= rm -rf

#################################################
#CFLAGS := -O2 -g -Wall -fmessage-length=0
CFLAGS := -O0 -g3 -Wall -fmessage-length=0
# CFLAGS += -v -mwindows
# CFLAGS += -mwindows
# CFLAGS += -Iinclude
# CFLAGS += -lpthread
ifeq ($(BUILD_TYPE), x86)
	CFLAGS += -pthread
endif
# Only static link can be supported on Android Platform
ifeq ($(BUILD_TYPE), arm)
    CFLAGS += -static
endif

#CXXFLAGS := -O2 -g -Wall -fmessage-length=0
CXXFLAGS := -O0 -g3 -Wall -fmessage-length=0
# CXXFLAGS += -lpthread
ifeq ($(BUILD_TYPE), x86)
	CXXFLAGS += -pthread
endif
ifeq ($(BUILD_TYPE), arm)
    CXXFLAGS += -static
endif

#################################################

C_SRCS :=
S_UPPER_SRCS :=
O_SRCS :=
ASM_SRCS :=
OBJ_SRCS :=
OBJS :=
EXECUTABLES :=
C_DEPS :=

LIBS :=
USER_OBJS :=

SRCDIR = ../src
OBJDIR = ../build/$(BUILD_TYPE)
DISTDIR = ../dist/$(BUILD_TYPE)

# Every subdirectory with source files must be described here
SUBDIRS := \
src \

#################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
$(SRCDIR)/sh_queue.c \
$(SRCDIR)/sh_thread.c \
$(SRCDIR)/w1_netlink_userservice.c \
$(SRCDIR)/w1_netlink_connector_userapp.c

OBJS += \
$(OBJDIR)/sh_queue.o \
$(OBJDIR)/sh_thread.o \
$(OBJDIR)/w1_netlink_userservice.o \
$(OBJDIR)/w1_netlink_connector_userapp.o

C_DEPS += \
$(OBJDIR)/sh_queue.d \
$(OBJDIR)/sh_thread.d \
$(OBJDIR)/w1_netlink_userservice.d \
$(OBJDIR)/w1_netlink_connector_userapp.d

ifeq ($(BUILD_TYPE), x86)
    LIBS += \
    	../lib/x86/libpthread.so
endif

ifeq ($(BUILD_TYPE), arm)
    LIBS += \
    	../lib/arm/libc.a
endif


#################################################

ifeq ($(BUILD_TYPE), XX)
	INCDIR = /home/lihacker/apps/android-ndk-r6/platforms/android-5/arch-arm/usr
	INCFLAG = -I$(INCDIR)/include -I$(INCDIR)/lib
endif

# INCFLAG = $(foreach d, $(SRCDIR), $(join -I, $(d)))
#
# HEADERS = $(foreach d, $(SRCDIR), $(wildcard $(d)/*.h))
#
# $(OBJDIR)/%.o : %.c
# 	$(CC)     -c $(SRCDIR)/$<     -o $@     $(INCFLAG)


#################################################

# Each subdirectory must supply rules for building sources it contributes
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo 'Building file: $<'
	@echo 'Invoking GCC C Compiler: $(cc)'
ifeq ($(BUILD_TYPE), arm)
	$(CC) $(CFLAGS) -c -o"$@" "$<"	$(INCFLAG)
else
	$(CC) $(CFLAGS) -c -o"$@" "$<"
endif
	@echo 'Finished building: $<'
	@echo ' '

# $(CC) $(CFLAGS) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

#################################################

# Add inputs and outputs from these tool invocations to the build variables

TARGET = $(DISTDIR)/$(BUILD_NAME)

# All Target
all: $(TARGET)

# Tool invocations
$(TARGET): $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking GCC C Linker: $(CC)'
	$(CC)  -o "$(TARGET)" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS) $(EXECUTABLES) $(C_DEPS) $(TARGET)
	-@echo ' '

.PHONY: all clean dependents test
.SECONDARY:

test:   $(OBJS)
	@echo CFLAGS:    	$(CFLAGS)
	@echo INCFLAG:    	$(INCFLAG)
	@echo SRCS:    		$(C_SRCS)
	@echo OBJS:   		$(OBJS)
	@echo end
