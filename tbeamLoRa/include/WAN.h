/*
 *
 * WAN module
 *
 * PURPOSE: Handles LoRaWAN message sending and receiving
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

#include <Arduino.h>
#include <lmic/lmic.h>
#include <stdint.h>

namespace wan
{

/// \enum extra events not defined in LMiC library headers
typedef enum
{
        EV_QUEUED = 100,
        EV_PENDING = 101,
        EV_ACK = 102,
        EV_RESPONSE = 103,
} extra_ev;

/// \var message counter, stored in RTC memory to survive deep sleep
static RTC_DATA_ATTR uint32_t count = 0;

///
/// \brief           Registers callback for running when a message
///                  is received
///
/// \param[in]       callback     function pointer to the callback
///                              to run; param message is passed on
///
/// \return          void
///
void regist(void (*callback)(uint8_t message));

///
/// \brief           Sets the spreading factor for LoRa PHY
///
/// \param           sf     the spreading factor to set
///
/// \return          void
///
void set_spreading_factor(unsigned char sf);

///
/// \brief           Sets the ADR
///
/// \param           enabled     whether to enable ADR
///
/// \return          void
///
void adr(const bool enabled);

///
/// \brief           Initializes the message sent counter to zero,
///                  thereby clearing the previous stored value
///                  in the EEPROM
///
/// \return          void
///
void init_count();

///
/// \brief           Initializes the message sent counter to zero,
///                  thereby clearing the previous stored value
///                  in the EEPROM
///
/// \param[in]       callback    function pointer to the callback
///                            to run; param message is passed on
///
/// \return          void
///
bool setup();

///
/// \brief           Tries to join the network using the OTAA credentials
///                  previously set in setup()
///
/// \return          void
///
void join();

///
/// \brief           Main loop of LoRaWAN handler module,
///                  run asynchronously and in another OS task
///
/// \eturn           void
///
void loop();

///
/// \brief           Gets current value of sent message count
///                  from the EEPROM
///
/// \return          the value of the counter
///
uint32_t get_count();

///
/// \brief           Dumps the current message counter value in the EEPROM
///
/// \return          the current count
///
int dump_count();

///
/// \brief           Deletes LoRaWAN module's prefs stored in the EEPROM
///
/// \return          void
///
void erase_prefs();

///
/// \brief           Sends data to port port, optionally asking for ACK
///
/// \param[in]       data        the data buffer to send
/// \param[in]       data_size   size of data
/// \param[in]       port        the LoRaWAN port to use
/// \param[in]       confirmed   whether to ask for ACK
///
/// \return          void
///
void send(uint8_t *data, const uint8_t data_size, const uint8_t port, const bool confirmed);

} // namespace wan