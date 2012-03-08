
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>


#include "sh_types.h"
#include "sh_error.h"
#include "sh_util.h"
#include "sh_thread.h"

#include "w1_userspace.h"
#include "w1_userservice.h"


#include "w1_netlink_userservice.h"

/* ====================================================================== */
/* ============================ log ralated ============================= */
/* ====================================================================== */

//#define ANDROID_NDK

#define LOG_TAG   "ibutton_netlink_app"
#include "sh_log.h"

#define Debug(format, args...)    android_debug(format, ##args)

extern void print_master(void);
extern void print_all_slaves(void);

extern BOOL Test_ResetMaster();
extern BOOL Test_ListMasters();
extern BOOL Test_SearchSlaves();

extern BOOL Test_1904ReadRTC();
extern BOOL Test_1904WriteRTC();
extern BOOL Test_1920Temperature();


extern BOOL ibutton_test_setup(struct w1_user_service * w1UserService);
extern void ibutton_test_teardown();

int main(void)
{
    if(!ibutton_test_setup(&w1_netlink_userservice))
    {
        Debug("ibutton_test_setup failed...\n");
        goto GameOver;
    }

    Debug("======================================================\n");

    if(Test_ResetMaster())
    {
        Debug("Test_ResetMaster OK!!!\n");
    }
    else
    {
        Debug("Test_ResetMaster failed...\n");
    }

    Debug("======================================================\n");

    if(Test_ListMasters())
    {
        Debug("Test_ListMasters OK!!!\n");
    }
    else
    {
        Debug("Test_ListMasters failed...\n");
    }

    Debug("======================================================\n");

    if(Test_SearchSlaves())
    {
        Debug("Test_SearchSlaves OK!!!\n");
    }
    else
    {
        Debug("Test_SearchSlaves failed...\n");
    }

GameOver:

    Debug("======================================================\n");
	Debug("Main App Game Over...\n");
	return 0;
}
