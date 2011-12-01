/*
 *
 * Copyright (C) 2011 Deven Fan <deven.fan@gmail.com>
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

/* The most important header */
#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

/***************************************************************************/

struct w1hal_module_t {

    /* struct hw_module_t must come first, and named common */
    struct hw_module_t common;

    /* Place attributes here. */

    /* Place methods here. */

};

struct w1hal_control_device_t {

    /* struct hw_device_t must come first, and named common */
    struct hw_device_t common;

    /* attributes */
    int fd;

    /* Operation APIs, realy useful for upper layer */
    struct w1hal_control_operations operations;
};

struct w1hal_control_operations {

    int (*set_on)(struct w1hal_control_device_t *dev, int32_t led);

    int (*set_off)(struct w1hal_control_device_t *dev, int32_t led);

};


/***************************************************************************/

struct w1hal_control_context_t {

	struct w1hal_control_device_t device;

};

/* Module ID must be claimed. */
#define W1HAL_HARDWARE_MODULE_ID "w1hal"

