/*
 *
 * AXP192 Module
 *
 * PURPOSE: Handles functions related to the AXP192 Power Management Unit,
 *          selectively powering the modules on the board
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

namespace axp192
{

///
/// \brief           Initializes the AXP192 PMU module
///
/// \param[in]       gps     whether to turn on GPS
///
/// \return          void
///
void setup(bool gps);

///
/// \brief           Cut power off the GPS module via the
///                  AXP192 Power Management Unit
///
/// \return          void
///
void turn_off_GPS();

///
/// \brief           Cut power off the LoRa module via the
///                  AXP192 Power Management Unit
///
/// \return          void
///
void turn_off_LoRa();

///
/// \brief           Cut power off the device connected
///                  to the DCDC1 power pin via the
///                  AXP192 Power Management Unit
///
/// \note            The config variable \a DCDC1_DEVICE
///                  holds the name of the device to
///                  print in the log entry
///
/// \return          void
///
void turn_off_PowerPinDCDC1();

///
/// \brief           Turns on the blue led on board
///
/// \return          void
///
void turn_on_blueled();

///
/// \brief           Turns off the blue led on board
///
/// \return          void
///
void turn_off_blueled();

///
/// \brief           Prints the power status of devices
///
/// \return          void
///
void print_onoff();

} // namespace axp192