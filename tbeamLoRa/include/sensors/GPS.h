/*
 *
 * GPS module definitions
 *
 * PURPOSE: Holds GPS module definitions
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

namespace gps
{
///
/// \brief          Checks whether GPS data is valid (i.e. we have a lock)
///
/// \return         true if GPS data is valid, false otherwise
///
bool isValidData();

///
/// \brief          Get GPS latitude in decimal degrees
///
/// \return         the latitude
///
float get_latitude();

///
/// \brief          Get GPS longitude in decimal degrees
///
/// \return         the latitude
///
float get_longitude();

///
/// \brief          Get GPS altitude in decimal degrees
///
/// \return         the latitude
///
float get_altitude();

///
/// \brief          Setup GPS communication
///
/// \return         void
///
void setup();

///
/// \brief          Continuously parse GPS data until it gets a lock
///
/// \return         void
///
void loop();

} // namespace gps