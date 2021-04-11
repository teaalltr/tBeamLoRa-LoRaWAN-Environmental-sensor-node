/*
 *
 * Packer module
 *
 * PURPOSE: Read sensor data and encode them for sending;
 *          holds functions to access the buffer created and get its size
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

#include <EEPROM.h>

#include <CayenneLPP.h>
#include <SPIFlash.h>

#include "include/util/packer.h"
#include "include/util/panic.h"

#include "include/sensors/BME680.h"
#include "include/sensors/GPS.h"
#include "include/sensors/LPPYRA03AV.h"
#include "include/sensors/SEN0170.h"
#include "include/sensors/SPS30.h"

#include "include/pwr/AXP192.h"

#include "../../../config.h"

static const char *TAG = "Main";

namespace packer
{

/// latitude, must be kept after deep sleep
RTC_DATA_ATTR float latitude;

/// longitude, must be kept after deep sleep
RTC_DATA_ATTR float longitude;

/// altitude, must be kept after deep sleep
RTC_DATA_ATTR float altitude;

/// the encoded message payload
CayenneLPP Payload(CONFIG_MAX_PAYLOAD);

bool isValid(float temperature, float pressure, float gas, float humidity);

#define CAYENNE_UNK_TYPE "ERROR: CayenneLPP unknown type for %s"
#define CAYENNE_OVERFLOW "ERROR: CayenneLPP overflow for %s"

uint8_t *get_buffer()
{
        return Payload.getBuffer();
}

uint8_t get_buffer_size()
{
        return Payload.getSize();
}

void checkErr(const char *typestr)
{

        uint8_t err = Payload.getError();

        if (err != LPP_ERROR_OK)
        {

                if (err == LPP_ERROR_OVERFLOW)
                {
                        PANIC("CayenneLPP OVERFLOW");
                }

                if (err == LPP_ERROR_UNKOWN_TYPE)
                {
                        PANIC("CayenneLPP UNK ERROR");
                }

        }
}

void read_n_pack(bool gps_too, bool bme680_too, bool sps30_too, bool pyra_too, bool wind_too)
{
        //
        // clear payload
        //
        Payload.reset();

        //===================================================
        //                      GPS
        //===================================================

        //
        // update GPS coordinates if the button was clicked
        // or it's first boot
        //
        if (gps_too)
        {

                //
                // get a more precise value by waiting another 20 seconds
                //
                for (int i = 0; i < 20; i++)
                {

                        latitude = gps::get_latitude();
                        longitude = gps::get_longitude();
                        altitude = gps::get_altitude();

                        delay(1000);
                }
        }

        latitude = 43.6195;
        longitude = 12.6733;
        altitude = 212.4;

        ESP_LOGI(TAG, "GPS:");
        ESP_LOGI(TAG, "   Lat:  %.6f", latitude);
        ESP_LOGI(TAG, "   Long: %.6f", longitude);
        ESP_LOGI(TAG, "   Alt:  %.2f m", altitude);

        //
        // add GPS to payload & check for CayenneLPP errors
        //
        (void)Payload.addGPS(CONFIG_CHAN_GPS, latitude, longitude, altitude);
        checkErr("GPS");

        //===================================================
        //                      SEN0170
        //===================================================

        if (wind_too)
        {

                float windspeed = sen0170::get_windspeed();

                ESP_LOGI(TAG, "Anemometer:");
                ESP_LOGI(TAG, "    Wind speed: %.2f m/s", windspeed);

                //
                // add wind speed to payload
                //
                (void)Payload.addGenericSensor(CONFIG_CHAN_SEN0170_WIND, windspeed);

                //
                // check for CayenneLPP errors
                //
                checkErr("windspeed");
        }

        //===================================================
        //                    LPPYRA03AV
        //===================================================

        if (pyra_too)
        {

                //
                // get irradiance and show it
                //

                float irradiance = lppyra03av::get_irradiance();

                ESP_LOGI(TAG, "Pyranometer:");
                ESP_LOGI(TAG, "    %.2f W/m^2", irradiance);

                //
                // add irradiance to payload & check for CayenneLPP errors
                (void)Payload.addGenericSensor(CONFIG_CHAN_LPPYRA03AV_IRRAD, irradiance);
                checkErr("irradiance");
        }

        //===================================================
        //                    SPS30
        //===================================================

        if (sps30_too)
        {

                //
                // get values
                //
                struct sps_values val {0.0};

                (void)sps30.GetValues(&val);

                //
                // get mass concentration
                //
                float PM1Ugm3 = val.MassPM1;   // PM1 concentration  [μg/m3]
                float PM2Ugm3 = val.MassPM2;   // PM2 concentration  [μg/m3]
                float PM4Ugm3 = val.MassPM4;   // PM4 concentration  [μg/m3]
                float PM10Ugm3 = val.MassPM10; // PM10 concentration [μg/m3]

                //
                // get particle number
                //
                float PM0Particlem3 = val.NumPM0;   // PM0 concentration  [#/m3]
                float PM1Particlem3 = val.NumPM1;   // PM1 concentration  [#/m3]
                float PM2Particlem3 = val.NumPM2;   // PM2 concentration  [#/m3]
                float PM4Particlem3 = val.NumPM4;   // PM4 concentration  [#/m3]
                float PM10Particlem3 = val.NumPM10; // PM10 concentration [#/m3]

                float PMAverageUm = val.PartSize; // average [μm]

                ESP_LOGI(TAG, "SPS30:");
                ESP_LOGI(TAG, "    PM1     (ug/m^3): %.3f", PM1Ugm3);
                ESP_LOGI(TAG, "    PM2     (ug/m^3): %.3f", PM2Ugm3);
                ESP_LOGI(TAG, "    PM4     (ug/m^3): %.3f", PM4Ugm3);
                ESP_LOGI(TAG, "    PM10    (ug/m^3): %.3f", PM10Ugm3);
                ESP_LOGI(TAG, "    PM0     (#/m^3):  %.3f", PM0Particlem3);
                ESP_LOGI(TAG, "    PM1     (#/m^3):  %.3f", PM1Particlem3);
                ESP_LOGI(TAG, "    PM2     (#/m^3):  %.3f", PM2Particlem3);
                ESP_LOGI(TAG, "    PM4     (#/m^3):  %.3f", PM4Particlem3);
                ESP_LOGI(TAG, "    PM10    (#/m^3):  %.3f", PM10Particlem3);
                ESP_LOGI(TAG, "    Average (#/m^3):  %.3f", PMAverageUm);

                //
                // add data to payload and check for CayenneLPP errors
                //
#ifdef SPS30_MOCK
                float PM1Ugm3 = rand();
                float PM2Ugm3 = rand();
                float PM4Ugm3 = rand();
                float PM10Ugm3 = rand();

                float PM0Particlem3 = rand();
                float PM1Particlem3 = rand();
                float PM2Particlem3 = rand();
                float PM4Particlem3 = rand();
                float PM10Particlem3 = rand();
                float PMAverageUm = rand();
#endif
                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM1Ugm3, PM1Ugm3);
                checkErr("PM1Ugm3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM2Ugm3, PM2Ugm3);
                checkErr("PM2Ugm3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM4Ugm3, PM4Ugm3);
                checkErr("PM4Ugm3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM10Ugm3, PM10Ugm3);
                checkErr("PM10Ugm3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM0Particlem3, PM0Particlem3);
                checkErr("PM0Particlem3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM1Particlem3, PM1Particlem3);
                checkErr("PM1Particlem3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM2Particlem3, PM2Particlem3);
                checkErr("PM2Particlem3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM4Particlem3, PM4Particlem3);
                checkErr("PM4Particlem3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PM10Particlem3, PM10Particlem3);
                checkErr("PM10Particlem3");

                (void)Payload.addGenericSensor(CONFIG_CHAN_SPS30_PMAverageUm, PMAverageUm);
                checkErr("PMAverageUm");
        }

        //===================================================
        //                  BME680
        //===================================================

        float temperature, average_temp, altitudeBARO, humidity, pressure, gas;

        if (bme680_too)
        {

                do
                {

                        //
                        // init sensor
                        //
                        bme680::init_measurement();

                        //
                        // get data
                        //

                        temperature  = bme680::get_temperature();
                        average_temp = bme680::get_avg_temp();
                        pressure     = bme680::get_pressure() / 100.0;      // convert to hPa
                        gas          = bme680::get_gas()      / 1000.0;     // convert to KOhm
                        humidity     = bme680::get_humidity();
                        altitudeBARO = bme680::get_altitude();

                } while (!bme680::is_valid_data());

                //
                // show data
                //
                ESP_LOGI(TAG, "BME680:");
                ESP_LOGI(TAG, "    Temp:  %.2f degC", temperature);
                ESP_LOGI(TAG, "    Avg:   %.2f degC", average_temp);
                ESP_LOGI(TAG, "    Press: %.2f hPa", pressure);
                ESP_LOGI(TAG, "    Gas:   %.2f KOhm", gas);
                ESP_LOGI(TAG, "    Hum:   %.2f %%", humidity);
                ESP_LOGI(TAG, "    Alt:   %.2f m", altitudeBARO);

                //
                // add data to payload and check for CayenneLPP errors
                //

                (void)Payload.addTemperature(CONFIG_CHAN_BME680_TEMP, temperature);
                checkErr("temperature");

                (void)Payload.addTemperature(CONFIG_CHAN_BME680_AVGTEMP, average_temp);
                checkErr("average temperature");

                (void)Payload.addBarometricPressure(CONFIG_CHAN_BME680_PRESS, pressure);
                checkErr("pressure");

                (void)Payload.addGenericSensor(CONFIG_CHAN_BME680_GAS, gas);
                checkErr("gas resistance");

                (void)Payload.addRelativeHumidity(CONFIG_CHAN_BME680_HUM, humidity);
                checkErr("humidity");

                (void)Payload.addAltitude(CONFIG_CHAN_BME680_ALT, altitudeBARO);
                checkErr("altitude (baro)");
        }

        // show payload size
        ESP_LOGD(TAG, "--- Payload size: %u B", Payload.getSize());
}

} // namespace packer