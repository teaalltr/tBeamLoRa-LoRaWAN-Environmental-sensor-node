/*
 *
 * Main module
 *
 * PURPOSE: Entry point of the application
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

#include <cinttypes>

#include <esp_task_wdt.h>       // watchdog

#include "include/WAN.h"
#include "include/pwr/AXP192.h"
#include "include/pwr/sleep.h"
#include "include/util/panic.h"
#include "include/util/scanI2C.h"

#include "include/sensors/GPS.h"
#include "include/sensors/SPS30.h"

#include <EEPROM.h>
#include <SPIMemory.h>
#include <SimpleButton.h>

#include "include/sensors/BME680.h"
#include "include/util/packer.h"

#include "include/util/art.h"

#include "../config.h"


static const char *TAG = "Main";

//============================================================
//      Globals we want to preserve after deep sleep
//      (RTC_DATA_ATTR specifier)
//============================================================

/// deep sleep boot count
RTC_DATA_ATTR int bootCount = 0;

/// if we want GPS data too
RTC_DATA_ATTR bool wantGPS = false;


//============================================================
//                      Regular globals
//============================================================

bool first_no_valid     = true;
static uint32_t last    = 0;
static bool first_check = true;

bool packetSent, packetQueued;
bool has_bme680, has_sps30;

#define WDT_TIMEOUT (15*60)                  // watchdog timeout


/// button to reset preferences OR send GPS position
simplebutton::Button *button = new simplebutton::Button(CONFIG_BUTTON_PIN, true);

/// the external flash memory, shared between all modules
SPIFlash *flash;


//============================================================
//                        Prototypes
//============================================================

void hello();
void enqueue_packet();
bool grab_n_send();
void sleep();
void callback(uint8_t message);
void show_flash_info();


//============================================================
//                        Main module
//============================================================

//
// override the default watchdog ISR
// note: can't use ESP_LOGx here (not reentrant)
// extern "C" because it doesn't like mangled names
//
extern "C"
{
        void esp_task_wdt_isr_user_handler(void)
        {
                int len = ets_printf("!!!! WATCHDOG TIMEOUT EXPIRED !!!!\n");
                if (len > 0)
                {
                        esp_restart();
                }
        }
}

void hello()
{

        printLogo();
}

void reset_prefs()
{

        ESP_LOGD(TAG, "Erasing prefs...");
        wan::erase_prefs();

        //
        // give some time to read the screen
        //
        delay(CONFIG_LOGO_DELAY);

        ESP_LOGD(TAG, "Restarting...");
        ESP.restart();
}

void send_GPS()
{

        ESP_LOGI(TAG, "Button clicked, sending GPS position too");

        //
        // turn on blue led for 1 second
        //
        axp192::turn_on_blueled();
        delay(1000);
        axp192::turn_off_blueled();

        //
        // we want GPS to turn on on next reboot
        //
        wantGPS = true;

        //
        // reboot
        //
        ESP.restart();
}

void sleep()
{

        //
        // set the user button to wake the board
        //
        deepsleep::interrupt(CONFIG_BUTTON_PIN, LOW);

        //
        // we sleep for the interval between messages minus the current millis:
        // this way we distribute the messages evenly every SEND_INTERVAL millis
        //
        uint32_t sleep_for =
            (millis() < CONFIG_SEND_INTERVAL) ?
                                                CONFIG_SEND_INTERVAL - millis()
                                              : CONFIG_SEND_INTERVAL;
        //
        // delete the button object from the heap to avoid leak
        //
        delete button;
        
        
        //
        // do the deepsleep
        //
        deepsleep::do_deepsleep(sleep_for, wantGPS);
}

void callback(uint8_t message)
{

        if (message == EV_TXCOMPLETE && packetQueued)
        {

                packetQueued = false;
                packetSent   = true;
        }
}

void setup()
{
        //
        // check if we've woken up from deep sleep
        //
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
        {

                ESP_LOGI(TAG, "====== Yaaawn, woken up from deep sleep ======");
        }
        else
        {

                //
                // show logo
                //
                hello();
        }

        ESP_LOGD(TAG, "Configuring watchdog...");
        if (esp_task_wdt_init(WDT_TIMEOUT, true) == ESP_OK) // enable panic so ESP32 restarts
        {
                if (esp_task_wdt_add(nullptr) == ESP_OK)     // add current thread to WDT watch
                {
                        ESP_LOGD(TAG, "Watchdog configured: %u seconds", WDT_TIMEOUT);
                }
                else
                {
                        ESP_LOGE(TAG, "!!! COULD NOT CONFIGURE WATCHDOG !!!");
                }
        }
        else
        {
                ESP_LOGE(TAG, "!!! COULD NOT INIT WATCHDOG !!!");
        }

        ESP_LOGI(TAG, "Watchdog configured: %u seconds", WDT_TIMEOUT);


        //
        // send GPS on first run && handle overflow
        //
        if (bootCount <= 0)
        {
                wantGPS = false;
                bootCount = 0;
        }

        //
        // increment the boot counter
        //
        bootCount++;

        //
        // show the boot counter
        //
        ESP_LOGD(TAG, "Boot counter: %u boots", bootCount);

        //
        // show the packet counter
        //
        ESP_LOGD(TAG, "Packets counter: %u packets", wan::get_count());

        //
        // init serial port
        //
        Serial.begin(CONFIG_SERIAL_BAUD);

        //===============================
        //         Init SPI Flash
        //===============================

        //
        // init SPI flash on CS pin 5
        //
        // flash = new SPIFlash(5);

        // show SPI flash info
        // show_flash_info();

        //================================
        //      Init I2C and devices
        //================================

        //
        // init I2C
        //
        if (!Wire.begin(CONFIG_I2C_SDA, CONFIG_I2C_SCL))
        {
                ESP_LOGE(TAG, "!!! I2C COULD NOT BE INITED !!!");
        }

        //
        // setup the power management chip
        //
        axp192::setup(wantGPS);

        //
        // find I2C devices
        //
        i2c::scan();

        //
        // setup ERASE button pin to pullup
        //
        pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);

        //
        // setup I2C devices
        //

        //delete flash;

        has_sps30 = SPS3O::setup();
        ESP_LOGD(TAG, "has_SPS30 = %s", has_sps30 ? "TRUE" : "FALSE");

        has_bme680 = bme680::setup();
        ESP_LOGD(TAG, "has_bme680 = %s", has_bme680 ? "TRUE" : "FALSE");

        //
        // init GPS if required
        //
        if (wantGPS)
        {
                gps::setup();
        }

        //
        // setup LoRa module
        //
        if (!wan::setup())
        {
                // flash->begin(200);
                // // we MUST initiate the LoRa module!
                // ESP_LOGE(TAG, ERR_NOLORA);
                //
                // // save error to flash
                // String err = ERR_NOLORA;
                // flash->writeStr(0, err, true);
                //
                // // loop forever
                PANIC("CAN'T INIT THE LORA MODULE");
        }
        else
        {
                //
                // register previous callback
                //
                wan::regist(callback);

                //
                // join the net
                //
                wan::join();

                //
                // set Adaptive Data Rate
                //
                wan::set_spreading_factor(CONFIG_LORA_ADR);
        }

        //
        // handler for button press, 1/2 s
        // on holding for 500ms -> reboot, turn on GPS, get coords
        //
        button->setOnHolding(send_GPS, 500);

        //
        // on click -> reset LoRaWAN keys, reboot (and force re-join)
        //
        button->setOnDoubleClicked(reset_prefs);

        //
        // on click -> reboot, turn on GPS, get coords
        //
        //button->setOnDoubleClicked(send_GPS);

        //
        // if required, block program execution until we get a GPS lock
        //
        if (wantGPS)
        {
                while (!gps::isValidData())
                {
                        //
                        // reset watchdog timer
                        // don't consider this loop for watchdog
                        //
                        if (esp_task_wdt_reset() != ESP_OK)
                        {
                                ESP_LOGE(TAG, "!!! COULD NOT RESET WATCHDOG !!!");
                        }

                        gps::loop();
                }
        }
}

void show_flash_info()
{
        ESP_LOGI(TAG, "Initializing external flash...", flash->getJEDECID());
        delay(500);
        ESP_LOGD(TAG, "FLASH JEDEC ID: %u", flash->getJEDECID());
        ESP_LOGD(TAG, "FLASH capacity: %u B", flash->getCapacity());
        ESP_LOGD(TAG, "FLASH unique ID: %ll", flash->getUniqueID());
        ESP_LOGD(TAG, "FLASH man ID: %u", flash->getManID());
        ESP_LOGD(TAG, "FLASH JEDEC ID: %u", flash->getMaxPage());
}

void loop()
{

        //
        // start async LoRaWAN task
        //
        wan::loop();

        if (packetSent)
        {

                packetSent = false;
                sleep();
        }

        //
        // update button state
        //
        button->update();

        if (last == 0 || (millis() - last) >= CONFIG_SEND_INTERVAL)
        {

                if (grab_n_send())
                {

                        last = millis();
                        first_check = false;
                }
                else
                {

                        if (first_check)
                        {

                                if (wantGPS)
                                {
                                        ESP_LOGI(TAG, "Waiting for GPS lock...");
                                }
                                first_check = false;
                        }

                        if (wantGPS)
                        {

#ifdef GPS_WAIT_FOR_LOCK

                                if (millis() > CONFIG_GPS_WAIT_FOR_LOCK)
                                {

                                        sleep();
                                }
#endif
                        }

                        //
                        // put the CPU in low power mode
                        // for 100 ms (can be interrupted)
                        //
                        delay(100);
                }
        }

        //
        // reset watchdog timer
        //
        if (esp_task_wdt_reset() != ESP_OK)
        {
                ESP_LOGE(TAG, "!!! COULD NOT RESET WATCHDOG !!!");
        }
}

bool grab_n_send()
{

        //
        // reset packetSent
        //
        packetSent = false;

        if (wantGPS)
        {

                if (gps::isValidData())
                {

                        //
                        // encode the payload with GPS
                        //
                        packer::read_n_pack(true, has_bme680, has_sps30,
                                            CONFIG_HAS_LPPYRA03AV, CONFIG_HAS_SEN0170);

                        //
                        // the message is queued
                        //
                        packetQueued = true;

                        //
                        // enqueue for sending
                        //
                        enqueue_packet();

                        //
                        // don't want GPS next time
                        //
                        wantGPS = false;

                        //
                        // don't show other messages
                        //
                        first_no_valid = false;

                        return true;
                }
                else
                {

                        //
                        // print the message only once
                        //
                        if (first_no_valid)
                        {

                                ESP_LOGW(TAG, "GPS data not valid");
                                first_no_valid = false;
                        }

                        return false;
                }
        }
        else
        {

                //
                // encode the payload without GPS
                //
                packer::read_n_pack(false, has_bme680, has_sps30,
                                    CONFIG_HAS_LPPYRA03AV, CONFIG_HAS_SEN0170);

                //
                // enqueue for sending
                //
                enqueue_packet();

                return true;
        }
}

void enqueue_packet()
{
        //
        // the packet is queued
        //
        packetQueued = true;
        ESP_LOGD(TAG, "-> Message enqueued for sending");

        //
        // get buffer and it size
        //
        uint8_t *data = packer::get_buffer();
        uint8_t size  = packer::get_buffer_size();

        //
        // delegate WAN to send
        //
        wan::send(data, size, CONFIG_LORAWAN_PORT, true);
}
