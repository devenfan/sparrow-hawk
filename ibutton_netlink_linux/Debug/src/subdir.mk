################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ibutton_netlink_linux.c \
../src/sh_queue.c \
../src/sh_thread.c \
../src/w1_netlink_connector_userapp.c 

OBJS += \
./src/ibutton_netlink_linux.o \
./src/sh_queue.o \
./src/sh_thread.o \
./src/w1_netlink_connector_userapp.o 

C_DEPS += \
./src/ibutton_netlink_linux.d \
./src/sh_queue.d \
./src/sh_thread.d \
./src/w1_netlink_connector_userapp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


