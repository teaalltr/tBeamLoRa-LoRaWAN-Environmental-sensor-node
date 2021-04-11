/*
 *
 * Sleep module
 *
 * PURPOSE: Holds functions for to for deep-sleeping
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

#include <Arduino.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <lmic/lmic.h>

#include "../../../config.h"
#include "include/pwr/AXP192.h"

static const char *TAG = "Sleep";

namespace deepsleep
{

#define USEC_TO_SEC 1000000

void interrupt(const uint8_t gpio, const uint8_t mode)
{
        (void) esp_sleep_enable_ext0_wakeup((gpio_num_t)gpio, mode);
}

void seconds(const uint32_t seconds)
{
        (void) esp_sleep_enable_timer_wakeup(seconds * USEC_TO_SEC);
        esp_deep_sleep_start();
}

void do_deepsleep(const uint64_t ms, const bool gps)
{

        //
        // show message
        //
        ESP_LOGI(TAG, "Entering deep sleep for %llu seconds", ms / 1000);

        //
        // gracefully shutdown the LoRa module
        //
        LMIC_shutdown();

        //
        // shutdown/power off I2C hardware
        //
        if (int res = Wire.endTransmission(true) != I2C_ERROR_OK)
        {
                ESP_LOGE(TAG, "!!! WIRE endTransmission: %u", res);
        }
        // Wire.~TwoWire();
        pinMode(SDA, INPUT); // needed because Wire.end() enables pullups, power Saving
        pinMode(SCL, INPUT);

        //
        // cut off power from modules
        //
        if (!gps)
        {
                axp192::turn_off_GPS();
        }
        axp192::turn_off_LoRa();
        // AXP192::turn_off_PowerPinDCDC1();

        //
        // wait a bit for stuff to complete
        //
        delay(500);

        //
        // check if the modules are off
        //
        axp192::print_onoff();

        //
        // sets processor registers to sleep periphs unneeded for deep sleep
        //
        (void) esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

        //
        // only GPIOs which are have RTC functionality can be used
        // in this bit map: 0, 2, 4, 12-15, 25-27, 32-39.
        //
        uint64_t gpioMask = (1ULL << static_cast<unsigned int>(CONFIG_BUTTON_PIN));

        //
        // set BUTTON_PIN to pullup
        //
        (void) gpio_pullup_en(static_cast<gpio_num_t>(CONFIG_BUTTON_PIN));

        //
        // enable IRQ for our middle button (can wakeup the CPU)
        //
        (void) esp_sleep_enable_ext1_wakeup(gpioMask, ESP_EXT1_WAKEUP_ALL_LOW);

        //
        // sets the alarm to ms milliseconds (call expects microseconds)
        //
        (void) esp_sleep_enable_timer_wakeup(ms * 1000ULL);

        //
        // put MCU to sleep
        //
        esp_deep_sleep_start();
}

} // namespace deepsleep