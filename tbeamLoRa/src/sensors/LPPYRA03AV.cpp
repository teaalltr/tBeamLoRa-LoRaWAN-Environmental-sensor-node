/*
 *
 * LPPYRA03AV module
 *
 * PURPOSE: Holds LPPYRA03AV module handling functions
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
#include <filters.h>

#include "include/sensors/LPPYRA03AV.h"

#include "../../../config.h"

static const char *TAG = "LPPYRA03AV";

const float CUTOFF_FREQ = 0.01;     // Cutoff frequency in Hz
const float SAMPLING_TIME = 0.020;  // Sampling time in seconds (20 ms)

// Low-pass filter
Filter lowpassFilter(CUTOFF_FREQ, SAMPLING_TIME, IIR::ORDER::OD1);

namespace lppyra03av
{

float get_irradiance()
{

        int start = millis();
        float filteredval = 0.0;

        ESP_LOGI(TAG, "Init pyra measurement - 5 s");

        while (millis() - start < 5000)
        {

                //
                // read raw value
                //
                auto sensorValue = static_cast<float>(analogRead(CONFIG_LPPYRA03AV_PIN));

                //
                // lowpass filter
                //
                filteredval = lowpassFilter.filterIn(sensorValue);

                // ESP_LOGD(TAG, "%u, %f, %f", sensorValue, outvoltage, irradiance);
                delay(20);
        }

        //
        // convert to actual voltage
        //
        float outvoltage = filteredval * (3.3 / 4095.0);

        //
        // actual irradiance in W/m^2
        // see http://www.deltaohm.com/ver2012/download/LP_pyra03_M_uk.pdf section 5.3, formula 2
        //
        float irradiance = 400.0 * outvoltage;

        return irradiance;
}

} // namespace lppyra03av