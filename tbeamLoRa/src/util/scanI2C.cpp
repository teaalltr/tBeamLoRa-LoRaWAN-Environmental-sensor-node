/*
 *
 * scanI2C module
 *
 * PURPOSE: Scans I2C addresses in search for known ones
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

#include <Wire.h>

#include "../../../config.h"

static const char *TAG = "scanI2C";

namespace i2c
{

void scan()
{

        int nDevices = 0;

        //
        // scan the whole I2C address space
        //
        for (int addr = 1; addr < 127; addr++)
        {

                //
                // try initializing bus, see if someone replies
                //
                Wire.beginTransmission(addr);
                char err = Wire.endTransmission();

                int address = addr < 16 ? 0 : addr;

                if (err == I2C_ERROR_OK)
                {

                        ESP_LOGI(TAG, "I2C device found at address 0x%x", address);

                        nDevices++;

                        //
                        // check if it is a supported device
                        //
                        switch (addr)
                        {

                        case CONFIG_AXP192_ADDR:
                                ESP_LOGI(TAG, "--> AXP192 PMU found");
                                break;

                        case CONFIG_SPS30_ADDR:
                                ESP_LOGI(TAG, "--> SPS30 sensor found");
                                break;

                        case CONFIG_BME680_ADDR1:
                        case CONFIG_BME680_ADDR2:
                                ESP_LOGI(TAG, "--> BME680 sensor found");
                                break;

                        default: {

                                ESP_LOGW(TAG, "-> UNKNOWN device found");
                                break;
                        }
                        }
                }
                else if (err == I2C_ERROR_BUS)
                {

                        ESP_LOGE(TAG, "Unknown error at address 0x%x", address);
                }
                else
                {

                }
        }

        if (nDevices == 0)
        {

                ESP_LOGW(TAG, "No I2C devices found");
        }
        else
        {

                ESP_LOGI(TAG, "I2C search done");
        }
}

} // namespace i2c