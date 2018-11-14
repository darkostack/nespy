/*
 * Copyright (c) 2018, CETIC.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *         Simple MAC address translation
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */

#include "services/6lbr/mactrans-registry.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "6LBR"
#define LOG_LEVEL LOG_LEVEL_6LBR

//! Location of TRANSLATE (TR) bit in Ethernet address
#define TRANSLATE_BIT_MASK (1<<2)
//! Location of LOCAL (GL) bit in Ethernet address
#define LOCAL_BIT_MASK     (1<<1)
//! Location of MULTICAST (MU) bit in Ethernet address
#define MULTICAST_BIT_MASK (1<<0)

#define PREFIX_BUFFER_SIZE 32

uint8_t prefixCounter = 0;
uint8_t prefixBuffer[PREFIX_BUFFER_SIZE][3];

static uint8_t eth_to_lowpan(uip_lladdr_t * lowpan, const uip_eth_addr * ethernet)
{
    uint8_t index;

    //Check if translate bit is set, hence we have to look up the prefix
    if((ethernet->addr[0] &
       (TRANSLATE_BIT_MASK | MULTICAST_BIT_MASK | LOCAL_BIT_MASK)) ==
       (TRANSLATE_BIT_MASK | LOCAL_BIT_MASK)) {
        //Get top bits
        index = ethernet->addr[0] >> 3;

        //Copy over prefix
        lowpan->addr[0] = prefixBuffer[index][0];
        lowpan->addr[1] = prefixBuffer[index][1];
        lowpan->addr[2] = prefixBuffer[index][2];
        lowpan->addr[3] = ethernet->addr[1];
        lowpan->addr[4] = ethernet->addr[2];

        //Check this is plausible...
        if(index >= prefixCounter) {
            return 0;
        }
    } else {
        lowpan->addr[0] = ethernet->addr[0];
        lowpan->addr[1] = ethernet->addr[1];
        lowpan->addr[2] = ethernet->addr[2];
        lowpan->addr[3] = CETIC_6LBR_ETH_EXT_A;
        lowpan->addr[4] = CETIC_6LBR_ETH_EXT_B;
    }

    lowpan->addr[5] = ethernet->addr[3];
    lowpan->addr[6] = ethernet->addr[4];
    lowpan->addr[7] = ethernet->addr[5];
    return 1;
}

static uint8_t lowpan_to_eth(uip_eth_addr * ethernet, const uip_lladdr_t * lowpan)
{
    uint8_t index = 0;
    uint8_t i;

    //Check if we need to do anything:
    if((lowpan->addr[3] == CETIC_6LBR_ETH_EXT_A) && (lowpan->addr[4] == CETIC_6LBR_ETH_EXT_B)) {
        /** Nope: just copy over 6 bytes **/
        LOG_INFO("Low2Eth direct: ");
        LOG_INFO_LLADDR(lowpan);
        LOG_INFO_("\r\n");

        if((lowpan->addr[0] & (TRANSLATE_BIT_MASK | MULTICAST_BIT_MASK | LOCAL_BIT_MASK)) ==
           (TRANSLATE_BIT_MASK | LOCAL_BIT_MASK)) {
            LOG_INFO("Low2Eth direct : ADDRESS PREFIX CONFLICT\r\n");
        }

        ethernet->addr[0] = lowpan->addr[0];
        ethernet->addr[1] = lowpan->addr[1];
        ethernet->addr[2] = lowpan->addr[2];
        ethernet->addr[3] = lowpan->addr[5];
        ethernet->addr[4] = lowpan->addr[6];
        ethernet->addr[5] = lowpan->addr[7];


    } else {
        /** Yes: need to store prefix **/
        LOG_INFO("Low2Eth translate: ");
        LOG_INFO_LLADDR(lowpan);
        LOG_INFO_("\r\n");

        for(i = 0; i < prefixCounter; i++) {
            //Check the current prefix - if it fails, check next one
            if((lowpan->addr[0] == prefixBuffer[i][0]) &&
               (lowpan->addr[1] == prefixBuffer[i][1]) &&
               (lowpan->addr[2] == prefixBuffer[i][2])) {
                break;
            }
        }
        index = i;

        if(index >= PREFIX_BUFFER_SIZE) {
            LOG_ERR("Low2Eth buffer overflow\r\n");
            // Overflow. Fall back to simple translation.
            // TODO: Implement me!
            ethernet->addr[0] = lowpan->addr[0];
            ethernet->addr[1] = lowpan->addr[1];
            ethernet->addr[2] = lowpan->addr[2];
            ethernet->addr[3] = lowpan->addr[5];
            ethernet->addr[4] = lowpan->addr[6];
            ethernet->addr[5] = lowpan->addr[7];
            return 0;
        } else {
            //Are we making a new one?
            if(index == prefixCounter) {
                LOG_INFO("Low2Eth adding prefix\r\n");
                prefixCounter++;
                prefixBuffer[index][0] = lowpan->addr[0];
                prefixBuffer[index][1] = lowpan->addr[1];
                prefixBuffer[index][2] = lowpan->addr[2];
            }
            //Create ethernet MAC address now
            ethernet->addr[0] = TRANSLATE_BIT_MASK | LOCAL_BIT_MASK | (index << 3);
            ethernet->addr[1] = lowpan->addr[3];
            ethernet->addr[2] = lowpan->addr[4];
            ethernet->addr[3] = lowpan->addr[5];
            ethernet->addr[4] = lowpan->addr[6];
            ethernet->addr[5] = lowpan->addr[7];

            LOG_INFO("Low2Eth Lowpan addr: ");
            LOG_INFO_LLADDR(lowpan);
            LOG_INFO_("\r\n");

            LOG_INFO("Low2Eth Ethernet addr: ");
            LOG_INFO_ETHADDR(ethernet);
            LOG_INFO_("\r\n");
        }
    }

    return 1;
}

const struct mactrans_driver mactrans_registry = {
    eth_to_lowpan,
    lowpan_to_eth
};
