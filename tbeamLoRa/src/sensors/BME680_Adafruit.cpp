/*
 *
 * BME680 module
 *
 * PURPOSE: Handles the BME680 sensor, implementing the BME680 common
 *         interface defined in BME680.h using the simpler Adafruit lib
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

#include "../../../config.h"

#if !USE_BME680_BOSCH_LIB

#include <Adafruit_BME680.h>
#include <MovingAverage.h>

#include "../../include/sensors/BME680.h"

static const char *TAG = "BME680_Adafruit";

namespace bme680
{

/// our sensor
Adafruit_BME680 bme; // NOLINT, I2C

/// moving average for the temperature
MovingAverage<float> avgTemp(CONFIG_BME680_TEMP_MV_AVG);

bool setup()
{
        if (!bme.begin())
        {
                ESP_LOGE(TAG, "Could not find a valid BME680 sensor");
                return false;
        }

        //
        // set up oversampling and passband filter
        //

        if (bme.setTemperatureOversampling(BME680_OS_8X)            // oversampling
                || bme.setHumidityOversampling(BME680_OS_2X)
                || bme.setPressureOversampling(BME680_OS_4X)
                || bme.setIIRFilterSize(BME680_FILTER_SIZE_3)
                || bme.setGasHeater(320, 150))   // configure gas sensor heater, 320 C for 150 ms
        {
                ESP_LOGE(TAG, "!!! CANNOT SET BME680 CONFIG !!!");
        }

        return true;
}

bool is_valid_data()
{
        return get_gas() / 1000.0 > 0.0;
}

void init_measurement()
{
        if (!bme.performReading())
        {
                ESP_LOGE(TAG, "!!! CANNOT INIT BME680 MEASUREMENT !!!");
        }
}

float get_temperature()
{
        init_measurement();
        return bme.temperature;
}

float get_humidity()
{
        init_measurement();
        return bme.humidity;
}

float get_gas()
{
        init_measurement();
        return static_cast<float>(bme.gas_resistance);
}

float get_pressure()
{
        init_measurement();
        return static_cast<float>(bme.pressure);
}

float get_avg_temp()
{
        init_measurement();
        return avgTemp.CalculateMovingAverage(bme.temperature);
}

float get_altitude()
{
        init_measurement();
        return bme.readAltitude(CONFIG_SEALEVELPRESSURE_HPA);
}

float get_iaq()
{

#if false

        if (has_bme680) {

            uint16_t caqi2 = 0, caqi10 = 0;
            uint16_t pm2 = (uint16_t) getAvgPM2(); // change
            uint16_t pm10 = (uint16_t) getAvgPM10(); // change

            // Check PM2.5
            if (pm2 <= 30)
                caqi2 = map(pm2, 0, 30, 0, 50);
            else if (pm2 <= 55)
                caqi2 = map(pm2, 30, 55, 50, 75);
            else if (pm2 <= 110)
                caqi2 = map(pm2, 55, 110, 75, 100);
            else
                caqi2 = pm2 / 1.1;

            // Check PM10
            if (pm10 <= 50)
                caqi10 = pm10;
            else if (pm10 <= 90)
                caqi10 = map(pm10, 50, 90, 50, 75);
            else if (pm10 <= 180)
                caqi10 = map(pm10, 90, 180, 75, 100);
            else
                caqi10 = pm10 / 1.8;

            if (caqi2 > caqi10)
                return caqi2;

            return caqi10;

        }
        else
#endif

        //
        // not available for this library
        // (it may be calculated with data from the SPS30, maybe)
        //
        return MAXFLOAT;
}

} // namespace bme680

#endif // !USE_BME680_BOSCH_LIB