/*
 *
 * SEN0170 module
 *
 * PURPOSE: Holds SEN0170 module handling functions
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

#include "../../config.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "LoopDoesntUseConditionVariableInspection"

static const char *TAG = "SEN0170";

const float CUTOFF_FREQ   = 0.01;                    // Cutoff frequency in Hz
const float SAMPLING_TIME = 0.010;                   // Sampling time in seconds (20 ms)

// Low-pass filter
Filter lowpassFilterAnemometer(CUTOFF_FREQ, SAMPLING_TIME, IIR::ORDER::OD1);

namespace sen0170
{

float get_windspeed()
{

        //
        // set pin to input mode (with hi-z)
        //
        pinMode(CONFIG_SEN0170_PIN, INPUT);

        int start = millis();
        float filteredval = 0.0;

        ESP_LOGI(TAG, "Initializing anemometer measurement - 5 seconds...");

        while (millis() - start < 5000)
        {
                //
                // read raw value
                //
                unsigned short rawValue = analogRead(CONFIG_SEN0170_PIN);

                //
                // lowpass filter
                //
                filteredval = lowpassFilterAnemometer.filterIn(static_cast<float_t>(rawValue));
        }

        //
        // convert to actual voltage
        //
        float outvoltage = (filteredval * 5.0F) / 4095.0F;

        //
        // convert to wind speed
        //
        float speed = 6.0 * outvoltage;

        return speed;
}

} // namespace sen0170

#pragma clang diagnostic pop