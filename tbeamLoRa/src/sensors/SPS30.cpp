/*
 *
 * SPS30 module
 *
 * PURPOSE: Holds SPS30 module handling functions
 *
 * -----------------------------------------------------------------------
 *
 * This file is part of tbeamLoRa
 * Copyright (C) 2020-2021  Marco Savelli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "include/sensors/SPS30.h"

#include "../config.h"

static const char *TAG = "SPS30";

/// the sensor to read from
SPS30 sps30;

namespace SPS3O
{

#define DESCR_BUF_SIZE   80
#define DEVINFO_BUF_SIZE 32

void errtos(const char *mess, uint8_t r)
{

        /// will hold the error message
        char buf[DESCR_BUF_SIZE];

        //
        // show error message
        //
        ESP_LOGE(TAG, "%s", mess);

        //
        // get description
        //
        sps30.GetErrDescription(r, buf, DESCR_BUF_SIZE);

        //
        // show description
        //
        ESP_LOGE(TAG, "%s", buf);
}

bool setup()
{

        // begin communication channel
        // Wire.begin();

        if (!sps30.begin(&Wire))
        {

                ESP_LOGE(TAG, "Could not set I2C communication channel");
        }

        //
        // check for SPS30 connection
        //
        if (!sps30.probe())
        {

                ESP_LOGE(TAG, "Can't probe/connect to SPS30");
                return false;
        }
        else
        {

                ESP_LOGI(TAG, "SPS30 sensor detected");
        }

        //
        // reset SPS30 connection
        //
        if (!sps30.reset())
                ESP_LOGW(TAG, "Could not reset");

        //
        // read device info
        //
        getDeviceInfo();

        //
        // start measurement
        //
        if (sps30.start())
        {
                ESP_LOGI(TAG, "Async measurement started");

#if CONFIG_SPS30_CLEAN_NOW

                //
                // issue a clean-now
                //
                if (sps30.clean())
                {

                        ESP_LOGI(TAG, "Fan-cleaning manually started");
                }
                else
                {

                        ESP_LOGW(TAG, "Could NOT manually start fan-cleaning");
                }

#endif
        }
        else
        {

                ESP_LOGE(TAG, "Could NOT start measurement");
        }

        if (sps30.I2C_expect() == 4)
        {

                ESP_LOGE(TAG, "Due to I2C buffersize only the SPS30 MASS concentration is available");
        }

        return true;
}

void getDeviceInfo()
{

        /// will hold the info
        char buf[DEVINFO_BUF_SIZE];

        //
        // try and read the serial number
        //
        uint8_t ret = sps30.GetSerialNumber(buf, DEVINFO_BUF_SIZE);

        if (ret == ERR_OK)
        {

                ESP_LOGI(TAG, "--> Serial number: ");

                if (strlen(buf) > 0)
                {

                        ESP_LOGI(TAG, "    --> %s", buf);
                }
                else
                {

                        ESP_LOGW(TAG, "    --> Not available");
                }
        }
        else
        {

                errtos("Could not get serial number. ", ret);
        }

        //
        // clean up buf
        //
        (void) memset(buf, 0, sizeof(buf));

        //
        // try and get the product name
        //

        ret = sps30.GetProductName(buf, DEVINFO_BUF_SIZE);
        if (ret == ERR_OK)
        {

                ESP_LOGI(TAG, "--> Product name: ");

                if (strlen(buf) > 0)
                {
                        ESP_LOGI(TAG, "    --> %s", buf);
                }
                else
                {
                        ESP_LOGW(TAG, "    --> Not available");
                }
        }
        else
        {

                errtos("Could not get product name", ret);
        }

        //
        // try and get version info
        //
        SPS30_version v{0};
        ret = sps30.GetVersion(&v);

        if (ret != ERR_OK)
        {
                ESP_LOGW(TAG, "Can not read version info");
                return;
        }

        //
        // show version
        //
        ESP_LOGD(TAG, "--> Firmware version: %u.%u", v.major, v.minor);
        ESP_LOGD(TAG, "--> Library  version: %u.%u", v.DRV_major, v.DRV_minor);
}

void setAutoClean()
{

        uint32_t interval;
        uint8_t ret;

        // try and get the interval
        ret = sps30.GetAutoCleanInt(&interval);

        if (ret == ERR_OK)
        {

                ESP_LOGI(TAG, "Current Auto Clean interval: %u seconds", interval);
        }
        else
        {

                errtos("Could not get clean interval", ret);
        }

#if CONFIG_SPS30_CLEAN_NOW

        ESP_LOGI(TAG, "No Auto Clean interval change requested");

#endif
}

} // namespace SPS3O