/*
 *
 * Sleep module
 *
 * PURPOSE: Gives various functions to selectively put the MCU in deep-sleep
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

namespace deepsleep
{

///
/// \brief              Put the MCU to deep-sleep awaiting for a
///                     GPIO external interrupt event
///
/// \param[in]          gpio     GPIO pin to wait on
/// \param[in]          mode     GPIO pin mode (rising, falling)
///
/// \return             void
///
void interrupt(const uint8_t gpio, const uint8_t mode);

///
/// \brief              Put the MCU in deep-sleep for seconds seconds
///
/// \param              seconds     the period to sleep in seconds
///
/// \return             void
///
void seconds(const uint32_t seconds);

/// \brief              Shutdown modules and put the MCU in deep-sleep
///                     for ms millliseconds; GPS can be left on
///
/// \param[in]          ms     time to sleep in milliseconds
/// \param[in]          gps    whether to leave the GPS on
///
/// \return             void
///
void do_deepsleep(const uint64_t ms, const bool gps);

} // namespace deepsleep