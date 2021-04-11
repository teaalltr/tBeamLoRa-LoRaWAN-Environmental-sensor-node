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

#include <Preferences.h>
#include <SPI.h>
#include <hal/hal.h>
#include <lmic.h>

#include "../include/WAN.h"

#include "../config.h"
#include "include/config/credentials.h"

static const char *TAG = "LoRaWAN";

void os_getArtEui(uint8_t *buf)
{

        (void)memcpy(buf, APPEUI, sizeof(APPEUI));
}

void os_getDevEui(uint8_t *buf)
{

        (void)memcpy(buf, DEVEUI, sizeof(DEVEUI));
}

void os_getDevKey(uint8_t *buf)
{

        (void)memcpy(buf, APPKEY, sizeof(APPKEY));
}

namespace wan
{

/// LMIC GPIO configuration
const lmic_pinmap lmic_pins =
{
        .nss = CONFIG_NSS_GPIO,
        .rxtx = LMIC_UNUSED_PIN,
        .rst = CONFIG_RESET_GPIO,

        .dio = {CONFIG_DIO0_GPIO,
                CONFIG_DIO1_GPIO,
                CONFIG_DIO2_GPIO},
};

/// function pointer of LoRaWAN callback
void (*cb)(uint8_t);

void set_spreading_factor(unsigned char sf)
{

        LMIC_setDrTxpow(sf, CONFIG_LORA_TX_POW);
}

void adr(const bool enabled)
{
        //
        // set ADR
        //
        LMIC_setAdrMode(enabled);

        //
        // set manual ACK setting accordingly
        //
        LMIC_setLinkCheckMode(!enabled);
}

void run_callback(uint8_t message)
{
        //
        // set callback
        //
        cb(message);
}

void regist(void (*callback)(uint8_t message))
{
        cb = callback;
}

void init_count()
{

        //
        // if the value for LORA packet counts is unknown,
        // restore from flash
        //
        if (count == 0)
        {

                Preferences p;

                if (p.begin("lora", true))
                {

                        count = p.getUInt("count", 0);

                        p.end();
                }
        }
}

bool setup()
{

        //
        // wipe the message counter
        //
        wan::init_count();

        //
        // SPI interface
        //
        SPI.begin(CONFIG_SCK_GPIO, CONFIG_MISO_GPIO, CONFIG_MOSI_GPIO, CONFIG_NSS_GPIO);

        //
        // LMIC init
        //
        return os_init_ex(static_cast<const void*>(&lmic_pins)) == 1;
}

void join()
{

        //
        // reset the MAC state. Session and pending data transfers will be discarded
        //
        LMIC_reset();

        //
        // manually set available frequencies for LMiC to use;
        // without this, only three base channels from the LoRaWAN specification would be used
        //

        (void)LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
        (void)LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
        (void)LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);

        //
        // disable link check validation
        //
        LMIC_setLinkCheckMode(false);

        //
        // set default rate and transmit power for uplink (note: txpow seems to be ignored by the library)
        //
        wan::set_spreading_factor(CONFIG_LORA_SPREADING_FACTOR);

        //
        // make LMiC initialize the default channels, choose a channel, and
        // schedule the OTAA join
        //
        LMIC_startJoining();

        //
        // get keys from flash
        //

        Preferences p;
        (void)p.begin("lora", true);

        uint32_t netId = p.getUInt("netId", UINT32_MAX);     // default to UINT32_MAX
        uint32_t devAddr = p.getUInt("devAddr", UINT32_MAX); // default to UINT32_MAX

        uint8_t nwkKey[16], artKey[16];

        //
        // fill in keys and check for validity
        //
        bool goodKeys = p.getBytes("nwkKey", nwkKey, sizeof(nwkKey)) == sizeof(nwkKey) &&
                        p.getBytes("artKey", artKey, sizeof(artKey)) == sizeof(artKey);

        //
        // close our prefs
        //
        p.end();

        if (!goodKeys)
        {

                //
                // we have not yet joined a network, start a full join attempt:
                // make LMiC initialize the default channels, choose a channel, and
                // schedule the OTAA join
                //
                ESP_LOGI(TAG, "No session saved, joining from scratch");
                (void)LMIC_startJoining();
        }
        else
        {

                //
                // use previous session keys
                //
                ESP_LOGI(TAG, "Rejoining saved session");
                LMIC_setSession(netId, devAddr, nwkKey, artKey);

                //
                // trigger a false joined
                //
                wan::run_callback(EV_JOINED);
        }
}

uint32_t get_count()
{

        return count;
}

int dump_count()
{

        //
        // set packet counter
        //
        (void)LMIC_setSeqnoUp(count);

        //
        // ensure we write at least once
        //
        static uint32_t lastWriteMsec = UINT32_MAX;
        uint32_t now = millis();

        //
        // dump to rom every 5 mins
        //
        if (now < lastWriteMsec || (now - lastWriteMsec) > 5 * 60 * 1000L)
        {

                lastWriteMsec = now;

                Preferences p;

                if (p.begin("lora", false))
                {

                        (void) p.putUInt("count", count);
                        p.end();
                }
        }

        return count;
}

void erase_prefs()
{

        Preferences p;

        //
        // if there is a entry named "lora", clean it
        //
        if (p.begin("lora", false))
        {
                (void) p.clear();
                p.end();
        }
}

void send(uint8_t *data, const uint8_t data_size, const uint8_t port, const bool confirmed)
{

        //
        // save current packet count
        //
        (void) wan::dump_count();

        //
        // check if there is not a current TX/RX job running
        //
        if (LMIC.opmode & OP_TXRXPEND)
        {

                //
                // if there is, run appropriate callback e.g to send ACKs
                //
                wan::run_callback(wan::EV_PENDING);
                return;
        }

        //
        // prepare upstream data transmission at the next possible time
        //
        (void) LMIC_setTxData2(port, data, data_size, confirmed);

        wan::run_callback(wan::EV_QUEUED);

        //
        // increment global message counter
        //
        count++;
}

void loop()
{
        os_runloop_once();
}

} // namespace WAN

void onEvent(ev_t event)
{

        switch (event)
        {

        case EV_SCAN_TIMEOUT:
                ESP_LOGI(TAG, "EV_SCAN_TIMEOUT");
                break;

        case EV_BEACON_FOUND:
                ESP_LOGI(TAG, "EV_BEACON_FOUND");
                break;

        case EV_BEACON_MISSED:
                ESP_LOGI(TAG, "EV_BEACON_MISSED");
                break;

        case EV_BEACON_TRACKED:
                ESP_LOGI(TAG, "EV_BEACON_TRACKED");
                break;

        case EV_JOINING:
                ESP_LOGI(TAG, "EV_JOINING");
                break;

        case EV_JOINED: {

#if CONFIG_LORA_ADR

                //
                // disable link check validation
                //
                LMIC_setLinkCheckMode(false);

#endif
                ESP_LOGI(TAG, "EV_JOINED");

                uint32_t netid = 0;
                devaddr_t devaddr = 0;

                uint8_t nwkKey[16], artKey[16];

                LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);

                ESP_LOGD(TAG, "Received new netId, devAddr, nwkKey and artKey from network");

                //
                // save session keys to flash
                //

                Preferences p;

                if (p.begin("lora", false))
                {

                        if (p.putUInt("netId", netid) == 0)
                        {
                                ESP_LOGE(TAG, "!!! CANNOT SAVE netId !!!");
                        }
                        if (p.putUInt("devAddr", devaddr) == 0)
                        {
                                ESP_LOGE(TAG, "!!! CANNOT SAVE devAddr !!!");
                        }
                        if (p.putBytes("nwkKey", nwkKey, sizeof(nwkKey)) == 0)
                        {
                                ESP_LOGE(TAG, "!!! CANNOT SAVE nwkKey !!!");
                        }
                        if (p.putBytes("artKey", artKey, sizeof(artKey)) == 0)
                        {
                                ESP_LOGE(TAG, "!!! CANNOT SAVE artKey !!!");
                        }

                        p.end();
                }

                ESP_LOGD(TAG, "-> saved to EEPROM");

                //
                // restart
                //

                ESP_LOGD(TAG, "Restarting...");

                //
                // give some time to read the screen
                //
                delay(CONFIG_LOGO_DELAY);

                ESP.restart();
        }

        case EV_RFU1:
                ESP_LOGI(TAG, "EV_RFU1");
                break;

        case EV_JOIN_FAILED:
                ESP_LOGI(TAG, "EV_JOIN_FAILED");
                break;

        case EV_REJOIN_FAILED:
                ESP_LOGI(TAG, "EV_REJOIN_FAILED");
                break;

        case EV_TXCOMPLETE: {

                ESP_LOGI(TAG, "EV_TXCOMPLETE");

                if (LMIC.txrxFlags & TXRX_ACK)
                {
                        ESP_LOGI(TAG, "EV_ACK");
                        wan::run_callback(wan::EV_ACK);
                }

                if (LMIC.dataLen > 0)
                {
                        wan::run_callback(wan::EV_RESPONSE);
                }

                break;
        }

        case EV_LOST_TSYNC:
                ESP_LOGI(TAG, "EV_LOST_TSYNC");
                break;

        case EV_RESET:
                ESP_LOGI(TAG, "EV_RESET");
                break;

        case EV_RXCOMPLETE:
                ESP_LOGI(TAG, "EV_RXCOMPLETE");
                break;

        case EV_LINK_DEAD:
                ESP_LOGI(TAG, "EV_LINK_DEAD");
                break;

        case EV_LINK_ALIVE:
                ESP_LOGI(TAG, "EV_LINK_ALIVE");
                break;

        case EV_TXSTART:
                ESP_LOGI(TAG, "EV_TXSTART");
                break;

        case EV_JOIN_TXCOMPLETE:
                ESP_LOGI(TAG, "EV_JOIN_TXCOMPLETE");
                break;

        default:
                ESP_LOGI(TAG, "Unknown event");
                break;
        }

        //
        // send message callback
        //
        wan::run_callback(event);
}