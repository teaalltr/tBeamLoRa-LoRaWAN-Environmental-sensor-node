/*
 *
 * AXP192 Module
 *
 * PURPOSE: Handles functions related to the AXP192 Power Management Unit,
 *          selectively powering the modules on the board
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
#include <axp20x.h>
#include <include/pwr/AXP192.h>

#include "../config.h"

static const char *TAG = "AXP192";

namespace axp192
{

/// our PMU module
AXP20X_Class axp;

void powerevent_IRQ();

void setup(const bool gps)
{

        //===================================================
        //                   init the PMU
        //===================================================

        if (axp.begin(Wire, AXP192_SLAVE_ADDRESS) == AXP_PASS)
        {
                ESP_LOGI(TAG, "AXP192 Begin PASS");
        }
        else
        {
                ESP_LOGE(TAG, "AXP192 Begin FAIL");
        }

        //===================================================
        //              give power to modules
        //===================================================

        // turn on devices
        if (axp.setPowerOutPut(AXP192_LDO2, true) != AXP_PASS) // LORA radio
        {
                ESP_LOGE(TAG, "!!! AXP192 LDO2 setPowerOutPut FAIL !!!");
        }

        if (gps)
        {
                //
                // turn on GPS
                //
                if (axp.setPowerOutPut(AXP192_LDO3, true) != AXP_PASS)
                {
                        ESP_LOGE(TAG, "!!! AXP192 GPS setPowerOutPut FAIL !!!");
                }
        }

        if (axp.setPowerOutPut(AXP192_DCDC2, true) != AXP_PASS)
        {
                ESP_LOGE(TAG, "!!! AXP192 DCDC2 setPowerOutPut FAIL !!!");
        }

        if (axp.setPowerOutPut(AXP192_EXTEN, true) != AXP_PASS)
        {
                ESP_LOGE(TAG, "!!! AXP192 EXTEN setPowerOutPut FAIL !!!");
        }

        if (axp.setPowerOutPut(AXP192_DCDC1, true) != AXP_PASS) // see DCDC1_DEVICE
        {
                ESP_LOGE(TAG, "!!! AXP192 DCDC1 setPowerOutPut FAIL !!!");
        }

        //
        // set voltage to 3.3 V (3300 mV)
        //

        if (axp.setDCDC1Voltage(3300) != AXP_PASS)
        {
                ESP_LOGE(TAG, "!!! AXP192 setDCDC1Voltage FAIL !!!");
        }

        if (axp.setLDO2Voltage(3300) != AXP_PASS)
        {
                ESP_LOGE(TAG, "!!! AXP192 setLDO2Voltage FAIL !!!");
        }

        if (axp.setLDO3Voltage(3300) != AXP_PASS)
        {
                ESP_LOGE(TAG, "!!! AXP192 setLDO3Voltage FAIL !!!");
        }

        //===================================================
        //              set onboard led on/off
        //===================================================

#ifdef CONFIG_LED_BLINK

        axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ); // 1 blink/sec, "low rate"
        ESP_LOGD(TAG, "Blue LED blinking");

#endif

#ifdef CONFIG_LED_OFF

        if (axp.setChgLEDMode(AXP20X_LED_OFF) != AXP_PASS) // LED off
        {
                ESP_LOGE(TAG, "!!! AXP192 setChgLEDMode FAIL !!!");
        }
        else
        {
                ESP_LOGD(TAG, "Blue LED disabled");
        }

#endif

        //===================================================
        //                  check devices
        //===================================================

        axp192::print_onoff();

        bool batteryConnected = axp.isBatteryConnect();
        ESP_LOGI(TAG, "Battery is%s connected", batteryConnected ? "" : " not");
        if (batteryConnected)
        {
                ESP_LOGI(TAG, "    Battery voltage: %.2f V", axp.getBattVoltage() / 1000.0);
                ESP_LOGI(TAG, "    Battery is %s charging", axp.isChargeing() ? "" : " not");
        }

        //===================================================
        //               sets IRQs for PMU
        //===================================================

        //
        // set pin IRQ
        //
        pinMode(CONFIG_PMU_IRQ, INPUT_PULLUP);

        //
        // attach our interrupt handler
        //
        attachInterrupt(CONFIG_PMU_IRQ,
                        [] {axp192::powerevent_IRQ();},
                        FALLING);

        //
        // set adc1 to battery voltage monitor
        //
        (void) axp.adc1Enable(AXP202_BATT_CUR_ADC1, true);

        //
        // disable internal Coulomb counter to save power
        //
        (void) axp.DisableCoulombcounter();

        //
        // sets IRQs for battery removed or connected,
        // useful if we need to check for battery status,
        // atm we just show an info message in the serial port
        //
        (void) axp.enableIRQ(AXP202_VBUS_REMOVED_IRQ |          // USB unplugged
                          AXP202_VBUS_CONNECT_IRQ |      // USB plugged
                          AXP202_BATT_REMOVED_IRQ |      // battery removed
                          AXP202_BATT_CONNECT_IRQ |      // battery connected
                          AXP202_CHARGING_FINISHED_IRQ | // finished charging
                          AXP202_VBUS_OVER_VOL_IRQ |     // overvoltage on USB port
                          AXP202_BATT_LOW_TEMP_IRQ |     // battery temp too low
                          AXP202_BATT_OVER_TEMP_IRQ |    // battery temp too high
                          AXP202_PEK_LONGPRESS_IRQ |     // power button long press
                          AXP202_PEK_SHORTPRESS_IRQ,     // power button short press
                      true);

        // clear off other IRQs
        axp.clearIRQ();
}

void turn_off_GPS()
{
        (void) axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
        ESP_LOGD(TAG, "GPS module turned off");
}

void turn_off_LoRa()
{
        (void) axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
        ESP_LOGD(TAG, "LoRa module turned off");
}

void turn_off_PowerPinDCDC1()
{
        (void) axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
        ESP_LOGD(TAG, "DCDC1 turned off (%s)", CONFIG_DCDC1_DEVICE);
}

void turn_on_blueled()
{
        (void) axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL);
}

void turn_off_blueled()
{
        (void) axp.setChgLEDMode(AXP20X_LED_OFF);
}

void print_onoff()
{
        ESP_LOGD(TAG, "%s (DCDC1): %s", CONFIG_DCDC1_DEVICE, axp.isDCDC1Enable() ? "ON" : "OFF");
        ESP_LOGD(TAG, "DCDC2: %s", axp.isDCDC2Enable() ? "ON" : "OFF");

        ESP_LOGD(TAG, "GPS  (LDO3): %s", axp.isLDO3Enable() ? "ON" : "OFF");
        ESP_LOGD(TAG, "LoRa (LDO2): %s", axp.isLDO2Enable() ? "ON" : "OFF");

        ESP_LOGD(TAG, "ESP32 (DCDC3): %s", axp.isDCDC3Enable() ? "ON" : "OFF");
        ESP_LOGD(TAG, "Exten: %s", axp.isExtenEnable() ? "ON" : "OFF");
}

void powerevent_IRQ()
{

        // read IRQ
        if (axp.readIRQ() == AXP_FAIL)
        {
                ESP_LOGE(TAG, "!!! AXP readIRQ FAILED !!!");
        }
        else
        {
                if (axp.isPEKLongtPressIRQ())
                {
                        ESP_LOGI(TAG, "Power button long press");
                }

                if (axp.isPEKShortPressIRQ())
                {
                        ESP_LOGI(TAG, "Power button short press");
                }

                if (axp.isVbusOverVoltageIRQ())
                {
                        ESP_LOGW(TAG, "WARNING: USB voltage %.2f V too high", axp.getVbusVoltage() / 1000);
                }

                if (axp.isVbusPlugInIRQ())
                {
                        ESP_LOGI(TAG, "USB plugged, %.2fV @ %.0mA", axp.getVbusVoltage() / 1000, axp.getVbusCurrent());
                }

                if (axp.isVbusRemoveIRQ())
                {
                        ESP_LOGI(TAG, "USB unplugged");
                }

                if (axp.isBattPlugInIRQ())
                {
                        ESP_LOGI(TAG, "Battery has been connected");
                }

                if (axp.isBattRemoveIRQ())
                {
                        ESP_LOGI(TAG, "Battery has been removed");
                }

                if (axp.isChargingDoneIRQ())
                {
                        ESP_LOGI(TAG, "Battery charging complete");
                }
                if (axp.isBattTempLowIRQ())
                {
                        ESP_LOGW(TAG, "Battery temperature too high");
                }

                if (axp.isBattTempHighIRQ())
                {
                        ESP_LOGW(TAG, "Battery temperature too low");
                }

                axp.clearIRQ();
        }

}

} // namespace axp192