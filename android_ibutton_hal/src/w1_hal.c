/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "W1HALStub"

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include "w1_hal.h"

/*****************************************************************************/

int w1hal_device_close(struct hw_device_t* device)
{
	struct w1hal_control_device_t* ctx = (struct w1hal_control_device_t*)device;
	if (ctx) {
		free(ctx);
	}
	return 0;
}

int w1hal_on(struct w1hal_control_device_t *dev, int32_t led)
{
	LOGI("LED Stub: set %d on.", led);

	return 0;
}

int w1hal_off(struct w1hal_control_device_t *dev, int32_t led)
{
	LOGI("LED Stub: set %d off.", led);

	return 0;
}

static int w1hal_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
	struct w1hal_control_device_t *dev;

	dev = (struct w1hal_control_device_t *)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	dev->common.tag =  HARDWARE_DEVICE_TAG;
	dev->common.version = 0;
	dev->common.module = module;
	dev->common.close = w1hal_device_close;

    //
	dev->set_on = w1hal_on;
	dev->set_off = w1hal_off;

	*device = &dev->common;

success:
	return 0;
}


static struct hw_module_methods_t w1hal_module_methods = {
    open: w1hal_device_open
};



/*
 * The moust important part of HAL_MODULE_INFO_SYM
 *
 * tag:     Must be HARDWARE_MODULE_TAG
 * id:      Must be the module ID of HAL Stub
 * methods: struct hw_module_methods_t，the interface inside - "Open" must be implemented
 *
 */
const struct w1hal_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: W1HAL_HARDWARE_MODULE_ID,
        name: "W1 HAL Stub",
        author: "Deven Fan",
        methods: &w1hal_module_methods,
    }
    /* supporting APIs go here */
};

