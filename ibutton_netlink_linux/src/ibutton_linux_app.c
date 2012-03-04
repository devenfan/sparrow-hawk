
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sh_types.h"
#include "sh_util.h"

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

//#define ANDROID_NDK

#define LOG_TAG   "w1_netlink_userapp"
#include "sh_log.h"

#define Debug(format, args...)    android_debug(format, ##args)


static int g_fdMasterBus = -1;


static BOOL Test_1920Temperature()
{
    BOOL succeed;

    //Match ROM + Convert Temperature
    int dataSendLen1 = 10;
    BYTE dataSend1[10] = { 0x55, 0x10, 0x55, 0x69, 0x82, 0x00, 0x08, 0x00, 0x74,
                        0x44};

    //Match ROM + Read Scratchpad
    int dataSendLen2 = 10;
    BYTE dataSend2[10] = { 0x55, 0x10, 0x55, 0x69, 0x82, 0x00, 0x08, 0x00, 0x74,
                        0xBE};

    int dataRecvLen = 9;
    BYTE dataRecv[64];
    memset(dataRecv, 0, 64);


    //-----------------------------------------------------------

    //Every write will reset the bus...

    if(write(g_fdMasterBus, dataSend1, dataSendLen1) != dataSendLen1)
    {
        Debug("Test_1920Temperature-Step1 Failed!\n");
        return FALSE;
    }

    /*
    read(g_fdMasterBus, dataRecv, dataSendLen1);
    print_bytes(dataRecv, 0, dataSendLen1);
    */
    sleep(1); //Data line is held high for at least 0.75 seconds by bus
              //master to allow conversion to complete. Here use 1s instead.

    if(write(g_fdMasterBus, dataSend2, dataSendLen2) != dataSendLen2)
    {
        Debug("Test_1920Temperature-Step2 Failed!\n");
        return FALSE;
    }

    /*
    read(g_fdMasterBus, dataRecv, dataSendLen2);
    print_bytes(dataRecv, 0, dataSendLen2);
    */



    if(read(g_fdMasterBus, dataRecv, dataRecvLen) != dataRecvLen)
    {
        Debug("Test_1920Temperature-Step3 Failed!\n");
        return FALSE;
    }
    print_bytes(dataRecv, 0, dataRecvLen);

    return succeed;
}



int main(void) {

	puts("!!!Hello World!!!");

	Debug("open \"/sys/bus/w1/devices/w1_master_device/data\" \n");

    g_fdMasterBus = open("/sys/bus/w1/devices/w1_master_device/data", O_RDWR);

    if(-1 == g_fdMasterBus) {
        Debug("Open failed... \n");
        return EXIT_FAILURE;
    }

    Debug("Test_1920Temperature \n");

    Test_1920Temperature();

    close(g_fdMasterBus);

	return EXIT_SUCCESS;
}

