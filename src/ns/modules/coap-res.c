#if APP_CONF_WITH_COAP
#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/int-master.h"
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/net/app-layer/coap/coap-blocking-api.h"
#include "ns/modules/coap-res.h"
#include "ns/modules/nstd.h"
#include <stdio.h>

// Example usage to Coap Resource objects
//
//      # create periodic coap resource
//      res_sensor = ns.CoapResource(attr="title=\"Sensor periodic\";obs",
//                                   get=sensor_get,
//                                   post=None,
//                                   put=None,
//                                   delete=None,
//                                   period=1000,
//                                   periodic=sensor_periodic)
//
//      # create non-periodic coap resource
//      res_hello = ns.CoapResource(attr="title=\"Hello:?len=0..\";rt=\"Test\"",
//                                  get=hello_get,
//                                  post=None,
//                                  put=None,
//                                  delete=None,
//                                  period=0,
//                                  periodic=None);

const mp_obj_type_t ns_coap_resource_type;
static ns_coap_res_obj_all_t coap_res_obj_all;
static process_event_t client_get_event;
static process_event_t client_post_event;
static process_event_t client_put_event;
static process_event_t client_delete_event;
static char client_end_point_buf[64];

void ns_coap_resource_init0(void)
{
    coap_res_obj_all.remain = COAP_RES_OBJ_ALL_NUM;
    for (int i = 0; i < COAP_RES_OBJ_ALL_NUM; i++) {
        coap_res_obj_all.res[i].is_initialized = false;
    }
    // initialize client process event
    client_get_event = process_alloc_event();
    client_post_event = process_alloc_event();
    client_put_event = process_alloc_event();
    client_delete_event = process_alloc_event();
}

static ns_coap_res_id_t coap_res_get_id(void);

// predefined resource handler
COAP_RES_HANDLER(get_handler0);
COAP_RES_HANDLER(get_handler1);

COAP_RES_HANDLER(post_handler0);
COAP_RES_HANDLER(post_handler1);

COAP_RES_HANDLER(put_handler0);
COAP_RES_HANDLER(put_handler1);

COAP_RES_HANDLER(delete_handler0);
COAP_RES_HANDLER(delete_handler1);

// predefined periodic handler
COAP_RES_PERIODIC_HANDLER(periodic0);
COAP_RES_PERIODIC_HANDLER(periodic1);

// predefined coap client process
PROCESS(ns_coap_client_process, "coap client process");

// predefined client message handler
static void client_msg_handler0(coap_message_t *response);
static void client_msg_handler1(coap_message_t *response);

static ns_coap_resource_handler_t res_handler[] = {
    { get_handler0, post_handler0, put_handler0, delete_handler0, &res_periodic0, client_msg_handler0 },
    { get_handler1, post_handler1, put_handler1, delete_handler1, &res_periodic1, client_msg_handler1 },
};

STATIC mp_obj_t ns_coap_resource_make_new(const mp_obj_type_t *type,
                                          size_t n_args,
                                          size_t n_kw,
                                          const mp_obj_t *all_args)
{
    if (n_args == 0 && n_kw == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                        "ns: invalid arguments"));
    }

    ns_coap_res_id_t res_id = coap_res_get_id();

    if (res_id < 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap periodic resource overflow! max(%d)",
                    COAP_RES_OBJ_ALL_NUM));
    }

    enum {ARG_attr, ARG_get, ARG_post, ARG_put, ARG_delete, ARG_period, ARG_periodic};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_attr,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_get,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_post,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_put,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_delete,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_period,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_periodic, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj=mp_const_none} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // check the arguments
    if (args[ARG_attr].u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                        "ns: can't find coap resource attr arguments"));
    } else {
        // make sure at least one method is specified
        if (args[ARG_get].u_obj == mp_const_none &&
            args[ARG_post].u_obj == mp_const_none &&
            args[ARG_put].u_obj == mp_const_none &&
            args[ARG_delete].u_obj == mp_const_none) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                            "ns: can't find coap resource method arguments"));
        }
    }

    // create coap resource object
    ns_coap_res_obj_t *res_obj = m_new_obj(ns_coap_res_obj_t);
    res_obj->base.type = &ns_coap_resource_type;
    res_obj->id = res_id;
    res_obj->res.next = NULL;
    res_obj->res.url = NULL;
    res_obj->res.flags = IS_OBSERVABLE | IS_PERIODIC;
    res_obj->res.attributes = mp_obj_str_get_str(args[ARG_attr].u_obj);
    // check get method
    if (args[ARG_get].u_obj != mp_const_none) {
        res_obj->res.get_handler = res_handler[res_obj->id].get;
        res_obj->get_obj = args[ARG_get].u_obj;
    } else {
        res_obj->res.get_handler = NULL;
        res_obj->get_obj = mp_const_none;
    }
    // check post method
    if (args[ARG_post].u_obj != mp_const_none) {
        res_obj->res.post_handler = res_handler[res_obj->id].post;
        res_obj->post_obj = args[ARG_post].u_obj;
    } else {
        res_obj->res.post_handler = NULL;
        res_obj->post_obj = mp_const_none;
    }
    // check put method
    if (args[ARG_put].u_obj != mp_const_none) {
        res_obj->res.put_handler = res_handler[res_obj->id].put;
        res_obj->put_obj = args[ARG_put].u_obj;
    } else {
        res_obj->res.put_handler = NULL;
        res_obj->put_obj = mp_const_none;
    }
    // check delete method
    if (args[ARG_delete].u_obj != mp_const_none) {
        res_obj->res.delete_handler = res_handler[res_obj->id].del;
        res_obj->delete_obj = args[ARG_delete].u_obj;
    } else {
        res_obj->res.delete_handler = NULL;
        res_obj->delete_obj = mp_const_none;
    }
    // check periodic method
    if (args[ARG_period].u_int != 0 && args[ARG_periodic].u_obj != mp_const_none) {
        // set coap resource periodic
        res_handler[res_obj->id].periodic->period = (uint32_t)args[ARG_period].u_int;
        res_obj->res.periodic = res_handler[res_obj->id].periodic;
        res_obj->periodic_obj = args[ARG_periodic].u_obj;
    } else {
        res_obj->res.periodic = NULL;
        res_obj->periodic_obj = mp_const_none;
    }

    // set this to none unless this node is set as client
    res_obj->client_msg_callback_obj = mp_const_none;

    // put this object info container
    coap_res_obj_all.res[res_obj->id] = *res_obj;

    return MP_OBJ_FROM_PTR(res_obj);
}

STATIC void ns_coap_resource_print(const mp_print_t *print,
                                   mp_obj_t self_in,
                                   mp_print_kind_t kind)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ns: --- coap resource (%d) --- \n", self->id);
    mp_printf(print, "ns: is periodic  : %s\n", self->res.periodic != NULL ? "1" : "0");
    mp_printf(print, "ns: attributes   : %s\n", self->res.attributes);
    mp_printf(print, "ns: flags        : 0x%x\n", self->res.flags);
    mp_printf(print, "ns: has get      : %s\n", self->res.get_handler != NULL ? "1" : "0");
    mp_printf(print, "ns: has post     : %s\n", self->res.post_handler != NULL ? "1" : "0");
    mp_printf(print, "ns: has put      : %s\n", self->res.put_handler != NULL ? "1" : "0");
    mp_printf(print, "ns: has delete   : %s\n", self->res.delete_handler != NULL ? "1" : "1");
    mp_printf(print, "ns: is activated : %s\n", self->is_activated ? "1" : "0");
    mp_printf(print, "ns: uri path     : %s\n", self->is_activated ? self->uri_path : "NULL");
}

// res.server_activate("test/hello") # set this node as a coap server
STATIC mp_obj_t ns_coap_resource_server_activate(mp_obj_t self_in, mp_obj_t uri_path_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self = &coap_res_obj_all.res[self->id];

    self->uri_path = mp_obj_str_get_str(uri_path_in);
    self->is_activated = true;
    coap_activate_resource(&self->res, self->uri_path);
    return mp_const_none;
}

// res.client_ep("fe80::200:0:0:2")
STATIC mp_obj_t ns_coap_resource_client_ep(mp_obj_t self_in, mp_obj_t server_ipaddr_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self = &coap_res_obj_all.res[self->id];

    const char *server_ipaddr = mp_obj_str_get_str(server_ipaddr_in);

    if (uiplib_ipaddrconv(server_ipaddr, &self->end_point_ipaddr) == 0) {
        printf("ns: invalid end-point IPv6 addr: %s\r\n", server_ipaddr);
        return mp_const_none;
    }

    strcat(client_end_point_buf, "coap://[");
    strcat(client_end_point_buf, server_ipaddr);
    strcat(client_end_point_buf, "]");

    coap_endpoint_parse((char *)&client_end_point_buf,
                        ns_strlen((char *)&client_end_point_buf),
                        &self->end_point);

    return mp_const_none;
}

// res.client_get("test/hello", callback)
STATIC mp_obj_t ns_coap_resource_client_get(mp_obj_t self_in,
                                            mp_obj_t uri_path_in,
                                            mp_obj_t callback_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self = &coap_res_obj_all.res[self->id];

    const char *uri_path = mp_obj_str_get_str(uri_path_in);
    self->client_msg_callback_obj = callback_in;

    coap_init_message(self->client_request, COAP_TYPE_CON, COAP_GET, 0);
    coap_set_header_uri_path(self->client_request, uri_path);

    if (!process_is_running(&ns_coap_client_process)) {
        process_start(&ns_coap_client_process, NULL);
    }

    process_post(&ns_coap_client_process, client_get_event, (void *)&self->id);
    return mp_const_none;
}

// res.set_payload_text("Hello World!")
STATIC mp_obj_t ns_coap_resource_set_payload_text(mp_obj_t self_in,
                                                  mp_obj_t text_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *text = mp_obj_str_get_str(text_in);
    self->set_payload = text;
    return MP_OBJ_FROM_PTR(self);
}

// TODO: res.set_payload_json()

// res.get_payload()
STATIC mp_obj_t ns_coap_resource_get_payload(mp_obj_t self_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *payload = (const char *)self->get_payload;
    return mp_obj_new_str(payload, strlen(payload));
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_coap_resource_server_activate_obj, ns_coap_resource_server_activate);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_coap_resource_client_ep_obj, ns_coap_resource_client_ep);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ns_coap_resource_client_get_obj, ns_coap_resource_client_get);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_coap_resource_set_payload_text_obj, ns_coap_resource_set_payload_text);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_coap_resource_get_payload_obj, ns_coap_resource_get_payload);

STATIC const mp_rom_map_elem_t ns_coap_resource_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_server_activate), MP_ROM_PTR(&ns_coap_resource_server_activate_obj) },
    { MP_ROM_QSTR(MP_QSTR_client_ep), MP_ROM_PTR(&ns_coap_resource_client_ep_obj) },
    { MP_ROM_QSTR(MP_QSTR_client_get), MP_ROM_PTR(&ns_coap_resource_client_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_payload_text), MP_ROM_PTR(&ns_coap_resource_set_payload_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_payload), MP_ROM_PTR(&ns_coap_resource_get_payload_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_coap_resource_locals_dict, ns_coap_resource_locals_dict_table);

const mp_obj_type_t ns_coap_resource_type = {
    { &mp_type_type },
    .name = MP_QSTR_CoapResource,
    .print = ns_coap_resource_print,
    .make_new = ns_coap_resource_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_coap_resource_locals_dict,
};

// helper functions ------------------------------------------------------------

static ns_coap_res_id_t coap_res_get_id(void)
{
    bool is_get_id = false;
    ns_coap_res_id_t id = 0;

    int_master_status_t int_status = int_master_read_and_disable();

    for (int i= 0; i < COAP_RES_OBJ_ALL_NUM; i++) {
        if (coap_res_obj_all.res[i].is_initialized == false) {
            is_get_id = true;
            break;
        }
    }

    if (!is_get_id) {
        return -1;
    }

    coap_res_obj_all.remain--;

    int_master_status_set(int_status);
    int_master_enable();

    return id;
}

// predefined private functions for coap handlers ------------------------------

static void get_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[0];
    mp_obj_t payload = mp_const_none;

    if (res->get_obj != mp_const_none) {
        // get the payload
        payload = mp_call_function_1(res->get_obj, MP_OBJ_FROM_PTR(res));
    }

    if (payload != mp_const_none) {
        res = MP_OBJ_TO_PTR(payload);
        // TODO: content format
        memcpy(buffer, res->set_payload, strlen(res->set_payload));
        coap_set_header_content_format(response, TEXT_PLAIN);
        coap_set_payload(response, buffer, strlen(res->set_payload));
    }
}

static void get_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[1];
    mp_obj_t payload = mp_const_none;

    if (res->get_obj != mp_const_none) {
        // get the payload
        payload = mp_call_function_1(res->get_obj, MP_OBJ_FROM_PTR(res));
    }

    if (payload != mp_const_none) {
        res = MP_OBJ_TO_PTR(payload);
        // TODO: content format
        memcpy(buffer, res->set_payload, strlen(res->set_payload));
        coap_set_header_content_format(response, TEXT_PLAIN);
        coap_set_payload(response, buffer, strlen(res->set_payload));
    }
}

static void post_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void post_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                          uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void put_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void put_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void delete_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void delete_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                            uint16_t preferred_size, int32_t *offset)
{
    // TODO:
}

static void periodic0(void)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[0];
    if (res->res.periodic->period != 0 && res->res.periodic != NULL) {
        coap_notify_observers(&res->res);
    }
}

static void periodic1(void)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[1];
    if (res->res.periodic->period != 0 && res->res.periodic != NULL) {
        coap_notify_observers(&res->res);
    }
}

// predefined PROCESS_THREAD to handle CoAP message ----------------------------
PROCESS_THREAD(ns_coap_client_process, ev, data)
{
    static ns_coap_res_id_t data_id;
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_EVENT();
        if (ev == client_get_event && data != NULL) {
            data_id = *(ns_coap_res_id_t *)data;
            if (data_id >= 0 && data_id < COAP_RES_OBJ_ALL_NUM) {
                COAP_BLOCKING_REQUEST(&coap_res_obj_all.res[data_id].end_point,
                                      coap_res_obj_all.res[data_id].client_request,
                                      res_handler[data_id].client_msg);
            }
        }
    }
    PROCESS_END();
}

// predefined client message handler
static void client_msg_handler0(coap_message_t *response)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[0];
    const uint8_t *msg;
    coap_get_payload(response, &msg);
    res->get_payload = msg;
    if (res->client_msg_callback_obj != mp_const_none) {
        mp_call_function_1(res->client_msg_callback_obj, MP_OBJ_FROM_PTR(res));
    }
}

static void client_msg_handler1(coap_message_t *response)
{
    ns_coap_res_obj_t *res = &coap_res_obj_all.res[1];
    const uint8_t *msg;
    coap_get_payload(response, &msg);
    res->get_payload = msg;
    if (res->client_msg_callback_obj != mp_const_none) {
        mp_call_function_1(res->client_msg_callback_obj, MP_OBJ_FROM_PTR(res));
    }
}
#endif // #if APP_CONF_WITH_COAP
