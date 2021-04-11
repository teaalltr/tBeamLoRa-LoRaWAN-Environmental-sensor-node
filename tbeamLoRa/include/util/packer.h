/*
 *
 * Packer module
 *
 * PURPOSE: Read sensor data and encode them for sending;
 *          holds functions to access the buffer created and get its size
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

namespace packer
{

///
/// \brief           Read sensor values and encodes them
///
/// \param           gps_too    whether to process GPS
/// \param           bme680_too whether to process the BME680
/// \param           sps30_too  whether to process the SPS30
/// \param           pyra_too   whether to process the pyranomter
/// \param           wind_too   whether to process the windspeed sensor
///
/// \return          void
///
void read_n_pack(bool gps_too, bool bme680_too, bool sps30_too, bool pyra_too, bool wind_too);

///
/// \brief           Get the buffer encoded by PACKER_read_n_pack
///
/// \return          the encoded buffer
///
uint8_t *get_buffer();

///
/// \brief           Get the size of the encoded buffer
///
/// \return          the size of the encoded buffer in bytes
///
uint8_t get_buffer_size();

} // namespace packer