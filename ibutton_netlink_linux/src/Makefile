#################################################
#x86 or ARM
BUILD_TYPE = x86

BUILD_NAME = ibutton_netlink_linux.exe

#################################################
ifeq ($(BUILD_TYPE), ARM) 
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
CFLAGS += -pthread

#CXXFLAGS := -O2 -g -Wall -fmessage-length=0
CXXFLAGS := -O0 -g3 -Wall -fmessage-length=0
# CXXFLAGS += -lpthread
CXXFLAGS += -pthread

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
OBJDIR = ../build
DISTDIR = ../dist

# Every subdirectory with source files must be described here
SUBDIRS := \
src \

#################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
$(SRCDIR)/sh_queue.c \
$(SRCDIR)/sh_thread.c \
$(SRCDIR)/w1_netlink_connector_userapp.c 

OBJS += \
$(OBJDIR)/sh_queue.o \
$(OBJDIR)/sh_thread.o \
$(OBJDIR)/w1_netlink_connector_userapp.o 

C_DEPS += \
$(OBJDIR)/sh_queue.d \
$(OBJDIR)/sh_thread.d \
$(OBJDIR)/w1_netlink_connector_userapp.d 

LIBS += \
../lib/x86/libpthread.so
#/usr/lib/libpthread.a 

#################################################

# INCFLAG = $(foreach d, $(SRCDIR), $(join -I, $(d)))
#
# HEADS = $(foreach d, $(SRCDIR), $(wildcard $(d)/*.h))
#
# $(OBJDIR)/%.o : %.c
# 	$(CC)     -c $(SRCDIR)/$<     -o $@     $(INCFLAG)


#################################################

# Each subdirectory must supply rules for building sources it contributes
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo 'Building file: $<'
	@echo 'Invoking GCC C Compiler: $(cc)'
	$(CC) $(CFLAGS) -c -o"$@" "$<" 
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
	@echo TARGET:    	$(CFLAGS)
	@echo SRCS:    		$(C_SRCS)
	@echo OBJS:   		$(OBJS)
	@echo end
