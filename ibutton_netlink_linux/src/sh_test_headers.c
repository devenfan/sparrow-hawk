
#include <stdio.h>
#include <stdlib.h>

#include <sys/cdefs.h>
#include <sys/types.h>		//must
#include <sys/socket.h>		//must

//#include <linux/types.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "w1_netlink_userspace.h"
#include "sh_error.h"
#include "sh_thread.h"


#include "w1_netlink_userservice.h"