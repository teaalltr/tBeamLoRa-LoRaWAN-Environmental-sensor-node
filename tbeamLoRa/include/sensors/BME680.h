/*
 *
 * BME680 module definitions
 *
 * PURPOSE: Holds BME680 module definitions
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

#pragma once

namespace bme680
{

///
/// \brief          Set up the sensor's communication and settings
///
/// \return         true if succeeded, false otherwise
///
bool setup();

///
/// \brief          Set up the sensor for a measurement session
///
/// \return         void
///
void init_measurement();

///
/// \brief          Get the measured relative humidity in percentage
///
/// \return         the relative humidity
///
float get_temperature();

///
/// \brief          Get the measured temperature
///
/// \return         the temperature
///
float get_humidity();

///
/// \brief          Get the measured electric resistance from the
///                 gas sensor
///
/// \return         the gas resistance
///
float get_gas();

///
/// \brief          Get the measured pressure in hPa
///
/// \return         the pressure in hPa
///
float get_pressure();

///
/// \brief          Get the measured IAQ (quality of air index)
///
/// \return         the IAQ
///
float get_iaq();

///
/// \brief          Get the average temperature
///
/// \return         the temperature
///
float get_avg_temp();

///
/// \brief          Get the estimated altitude using the
///                 international barometric formula
///
/// \return         the altitude
///
float get_altitude();

///
/// \brief          Checks if measured data is valid
///
/// \return         true if valid, else false
///
bool is_valid_data();

} // namespace bme680