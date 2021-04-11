/*
 *
 * Scan I2C module
 *
 * PURPOSE: Scans all the I2C addresses looking for known ones
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

namespace i2c
{

///
/// \brief               Scans the whole I2C address space (0x0 - 0xFF)
///                      looking for known addresses, printing what it
///                      finds or errors otherwise
///
/// \return              void
///
void scan();

} // namespace i2c