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
#include "services/6lbr/mac-wrapper.h"
#include "net/mac/csma/csma.h"
#include "net/mac/nullmac/nullmac.h"
#include "services/6lbr/nvm-config.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "6LBR"
#define LOG_LEVEL LOG_LEVEL_6LBR

static const struct mac_driver *current_mac_driver = &csma_driver;

void mac_wrapper_init(void)
{
    if(nvm_data.mac_layer == CETIC_6LBR_MAC_LAYER_NONE) {
        LOG_INFO("Using 'nullmac' mac driver\r\n");
        current_mac_driver = &nullmac_driver;
    } else if(nvm_data.mac_layer == CETIC_6LBR_MAC_LAYER_CSMA) {
        LOG_INFO("Using 'csma' mac driver\r\n");
        current_mac_driver = &csma_driver;
    } else if(nvm_data.mac_layer == CETIC_6LBR_MAC_LAYER_NULLMAC) {
        LOG_INFO("Using 'nullmac' mac driver\r\n");
        current_mac_driver = &nullmac_driver;
    } else {
        LOG_ERROR("Unknown mac driver, using 'csma' instead\r\n");
        current_mac_driver = &csma_driver;
        nvm_data.mac_layer = CETIC_6LBR_MAC_LAYER_CSMA;
    }
    current_mac_driver->init();
}

char const * mac_wrapper_name(void)
{
  return current_mac_driver->name;
}

static void send(mac_callback_t sent, void *ptr)
{
  current_mac_driver->send(sent, ptr);
}

static void input(void)
{
  current_mac_driver->input();
}

static int on(void)
{
  return current_mac_driver->on();
}

static int off(void)
{
  return current_mac_driver->off();
}

static void init(void)
{
  //init() is postponed to 6LBR initialisation
}

const struct mac_driver mac_wrapper_driver = {
  "mac_wrapper",
  init,
  send,
  input,
  on,
  off,
};
