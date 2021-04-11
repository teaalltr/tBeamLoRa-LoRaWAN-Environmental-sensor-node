/*
 *
 * SPS30 module definitions
 *
 * PURPOSE: Holds SPS30 module definitions
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

#include <sps30.h>

/// the SPS30 external instance
extern SPS30 sps30;

namespace SPS3O
{

///
/// \brief             Setup routine for the SPS30 sensor
///
/// \return            true if the setup went well, false otherwise
///
bool setup();

///
/// \brief             Print device info
///
/// \return            void
///
void getDeviceInfo();

///
/// \brief             Sets SPS30 autoclean
///
/// \return            void
///
void setAutoClean();

} // namespace SPS3O