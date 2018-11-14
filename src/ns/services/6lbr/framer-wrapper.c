/*
 * Copyright (c) 2016, CETIC.
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
 * \author
 *         6LBR Team <6lbr@cetic.be>
 */

#include "contiki.h"
#include "services/6lbr/framer-wrapper.h"
#include "net/mac/framer/framer-802154.h"
#include "net/mac/framer/framer-nullmac.h"
#include "services/6lbr/nvm-config.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "6LBR"
#define LOG_LEVEL LOG_LEVEL_6LBR

static const struct framer *current_framer = &framer_802154;

void framer_wrapper_init(void)
{
    if(nvm_data.security_layer == CETIC_6LBR_SECURITY_LAYER_NONE) {
        if(nvm_data.mac_layer == CETIC_6LBR_MAC_LAYER_NULLMAC) {
            LOG_INFO("Using 'nullmac' framer\r\n");
            current_framer = &framer_nullmac;
        } else {
            LOG_INFO("Using 802.15.4 framer\r\n");
            current_framer = &framer_802154;
        }
    } else {
        LOG_WARN("Unknown framer, using '802.15.4' instead\r\n");
        current_framer = &framer_802154;
    }
}

static int length(void)
{
  return current_framer->length();
}

static int create(void)
{
  return current_framer->create();
}

static int parse(void)
{
  return current_framer->parse();
}

const struct framer framer_wrapper = {
  length,
  create,
  parse,
};
