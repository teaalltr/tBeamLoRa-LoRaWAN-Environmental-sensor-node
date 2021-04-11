/*
 *
 * Credentials file
 *
 * PURPOSE: Holds the OTAA credentials to connect to LoRaWAN.
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

///
/// \note
///
/// The PROGMEM directive tells the toolchain to put this in program
/// memory; this is because we need to keep it after waking up from
/// deep sleep *and* shutdown/reboot
///

/// !!!!!!!!!!!!!!!!!!!!! INSERT HERE YOUR KEYS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/// \var APPEUI (application/service identifier)
/// Must be in little-endian format
static const uint8_t PROGMEM APPEUI[8] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

/// \var DEVEUI (device identifier)
/// Must be in little endian format
static uint8_t DEVEUI[8] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
/// \var APPKEY (application/service encryption key)
/// Must be in big endian format
static const uint8_t PROGMEM APPKEY[16] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                                           0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
