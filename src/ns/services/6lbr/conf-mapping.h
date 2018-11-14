/*
 * Copyright (c) 2013, CETIC.
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

#ifndef CONF_MAPPING_H_
#define CONF_MAPPING_H_

#include "nvm-config.h"

//Shadow configuration

extern uint8_t cetic_6lbr_mode;
extern uint16_t cetic_6lbr_global_flags;
extern uint16_t cetic_6lbr_rpl_config;
extern uint8_t cetic_6lbr_eth_ip64_flags;

//RA Configuration

#undef UIP_CONF_ROUTER_LIFETIME
#define UIP_CONF_ROUTER_LIFETIME (nvm_data.ra_router_lifetime)

#undef UIP_CONF_ND6_MAX_RA_INTERVAL
#define UIP_CONF_ND6_MAX_RA_INTERVAL (nvm_data.ra_max_interval)

#undef UIP_CONF_ND6_MIN_RA_INTERVAL
#define UIP_CONF_ND6_MIN_RA_INTERVAL  (nvm_data.ra_min_interval)

#undef UIP_CONF_ND6_MIN_DELAY_BETWEEN_RAS
#define UIP_CONF_ND6_MIN_DELAY_BETWEEN_RAS  (nvm_data.ra_min_delay)

//RPL Configuration

#if CETIC_6LBR_WITH_RPL

#undef RPL_CONF_DEFAULT_INSTANCE
#define RPL_CONF_DEFAULT_INSTANCE    (nvm_data.rpl_instance_id)

#undef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_CONF_DIO_INTERVAL_DOUBLINGS    (nvm_data.rpl_dio_intdoubl)

#undef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_CONF_DIO_INTERVAL_MIN    (nvm_data.rpl_dio_intmin)

#undef RPL_CONF_DIO_REDUNDANCY
#define RPL_CONF_DIO_REDUNDANCY    (nvm_data.rpl_dio_redundancy)

#undef RPL_CONF_DEFAULT_LIFETIME
#define RPL_CONF_DEFAULT_LIFETIME    (nvm_data.rpl_default_lifetime)

#undef RPL_CONF_MIN_HOPRANKINC
#define RPL_CONF_MIN_HOPRANKINC    (nvm_data.rpl_min_hoprankinc)

#undef RPL_CONF_MAX_RANKINC
#define RPL_CONF_MAX_RANKINC    (nvm_data.rpl_max_rankinc)

#undef RPL_CONF_DEFAULT_LIFETIME_UNIT
#define RPL_CONF_DEFAULT_LIFETIME_UNIT    (nvm_data.rpl_lifetime_unit)

#undef RPL_CONF_PREFERENCE
#define RPL_CONF_PREFERENCE         (nvm_data.rpl_preference)

#undef RPL_CONF_WITH_DAO_ACK_TEST
#define RPL_CONF_WITH_DAO_ACK_TEST       ((cetic_6lbr_rpl_config & CETIC_6LBR_RPL_DAO_ACK) != 0)

#undef RPL_CONF_RPL_REPAIR_ON_DAO_NACK
#define RPL_CONF_RPL_REPAIR_ON_DAO_NACK    ((cetic_6lbr_rpl_config & CETIC_6LBR_RPL_DAO_ACK_REPAIR) != 0)

#undef RPL_CONF_DIO_REFRESH_DAO_ROUTES
#define RPL_CONF_DIO_REFRESH_DAO_ROUTES     ((cetic_6lbr_rpl_config & CETIC_6LBR_RPL_DAO_DISABLE_REFRESH) == 0)

#undef RPL_CONF_DAO_PATH_SEQUENCE_TEST
#define RPL_CONF_DAO_PATH_SEQUENCE_TEST       ((cetic_6lbr_rpl_config & CETIC_6LBR_RPL_CHECK_PATH_SEQUENCE) != 0)

#if CETIC_6LBR_RPL_RUNTIME_MOP
extern int cetic_6lbr_config_rpl_mop(void);
#undef RPL_CONF_MOP
#define RPL_CONF_MOP (cetic_6lbr_config_rpl_mop())
#endif

#endif /* CETIC_6LBR_WITH_RPL */

#if CONTIKI_TARGET_NATIVE
extern int sixlbr_config_select_timeout;
#undef SELECT_CONF_TIMEOUT
#define SELECT_CONF_TIMEOUT sixlbr_config_select_timeout
#endif /* CONTIKI_TARGET_NATIVE */

#endif // CONF_MAPPING_H_
