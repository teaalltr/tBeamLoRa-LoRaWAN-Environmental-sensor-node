/*
 *
 * GPS module
 *
 * PURPOSE: Holds the GPS module handling functions
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

#include <TinyGPS++.h>

#include "include/sensors/GPS.h"

#include "../../../config.h"

static const char *TAG = "GPS";

namespace gps
{

/// private TinyGPSPlus instance
TinyGPSPlus gps;

/// serial comm port the GPS will use
HardwareSerial serial_gps(CONFIG_GPS_SERIAL_NUM);

bool isValidData()
{

        return gps.location.isValid() && gps.altitude.value() > 0;
}

float get_latitude()
{

        return gps.location.lat();
}

float get_longitude()
{

        return gps.location.lng();
}

float get_altitude()
{

        return gps.altitude.meters();
}

void setup()
{

        serial_gps.begin(CONFIG_GPS_BAUDRATE, SERIAL_8N1, CONFIG_GPS_RX_PIN, CONFIG_GPS_TX_PIN);
        ESP_LOGI(TAG, "GPS setup complete");
}

void loop()
{

        while (serial_gps.available() != 0)
        {

                (void) gps.encode(serial_gps.read());
        }
}

} // namespace gps