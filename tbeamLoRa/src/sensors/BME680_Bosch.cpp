/*
 *
 * BME680 module
 *
 * PURPOSE: Handles the BME680 sensor, implementing the BME680 common
 *         interface defined in BME680.h using the official Bosch BSEC lib
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

#if USE_BME680_BOSCH_LIB

#include "Adafruit_BME680.h"
#include <MovingAverage.h>

#include "bsec.h"

static const char *TAG = "BME680_Bosch";

namespace bme680
{

void check_status();

/// our sensor
Bsec iaqSensor;

/// moving average for the temperature
MovingAverage<float> avgTemp(CONFIG_BME680_TEMP_MV_AVG);

bool valid_measurement;

#define ERR_CODE MAXFLOAT

// list of data we want from the sensor
bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
};

bool setup()
{
        // set up I2C comm

        iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);

        // print BSEC library version
        ESP_LOGD(TAG, "BSEC library version %u.%u.%u.%u", iaqSensor.version.major, iaqSensor.version.minor,
                 iaqSensor.version.major_bugfix, iaqSensor.version.minor_bugfix);

        check_status();

        // set sensor update rate to Ultra Low Power (ULP) mode
        // and pass the list of wanted data
        iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_ULP);

        // check if the sensor got set properly

        check_status();

        return true;
}

void check_status()
{

        if (iaqSensor.status != BSEC_OK)
        {

                if (iaqSensor.status < BSEC_OK)
                {

                        ESP_LOGE(TAG, "BSEC error code: %d", iaqSensor.status);
                }
                else
                {

                        ESP_LOGW(TAG, "BSEC warning code: %d", iaqSensor.status);
                }
        }

        if (iaqSensor.bme680Status != BME680_OK)
        {

                if (iaqSensor.bme680Status < BME680_OK)
                {

                        ESP_LOGE(TAG, "BME680 error code: %d", iaqSensor.bme680Status);
                }
                else
                {

                        ESP_LOGW(TAG, "BME680 warning code: %d", iaqSensor.bme680Status);
                }
        }

        iaqSensor.status = BSEC_OK;
}

void init_measurement()
{

        valid_measurement = false;

        if (iaqSensor.run())
        {

                valid_measurement = true;
                return;
        }
}

float get_temperature()
{

        if (valid_measurement)
        {

                return iaqSensor.temperature;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_humidity()
{

        if (valid_measurement)
        {

                return iaqSensor.humidity;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_gas()
{

        if (valid_measurement)
        {

                return iaqSensor.gasResistance;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_pressure()
{

        if (valid_measurement)
        {
                return iaqSensor.pressure;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_iaq()
{

        if (valid_measurement)
        {

                return iaqSensor.iaq;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_avg_temp()
{

        if (valid_measurement)
        {

                return avgTemp.CalculateMovingAverage(iaqSensor.temperature);
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

float get_altitude()
{

        if (valid_measurement)
        {

                float atmospheric = iaqSensor.pressure / 100.0;
                return 44330.0 * (1.0 - pow(atmospheric / iaqSensor.pressure, 0.1903)) / 100;
        }
        else
        {

                check_status();
                return ERR_CODE;
        }
}

} // namespace bme680

#endif