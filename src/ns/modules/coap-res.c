#if APP_CONF_WITH_COAP
#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/int-master.h"
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/net/app-layer/coap/coap-blocking-api.h"
#include "ns/modules/coap-res.h"
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
//
//       res_hello.activate("test/hello")

const mp_obj_type_t ns_coap_resource_type;
static ns_coap_res_obj_all_t coap_res_obj_all;

void ns_coap_resource_init0(void)
{
    coap_res_obj_all.remain = COAP_RES_OBJ_ALL_NUM;
    for (int i = 0; i < COAP_RES_OBJ_ALL_NUM; i++) {
        coap_res_obj_all.res[i] = NULL;
    }
}

static ns_coap_res_id_t coap_res_get_id(void);

// predefined get handler
COAP_RES_HANDLER(get_handler0);
COAP_RES_HANDLER(get_handler1);

// predefined post handler
COAP_RES_HANDLER(post_handler0);
COAP_RES_HANDLER(post_handler1);

// predefined put handler
COAP_RES_HANDLER(put_handler0);
COAP_RES_HANDLER(put_handler1);

// predefined delete handler
COAP_RES_HANDLER(delete_handler0);
COAP_RES_HANDLER(delete_handler1);

// predefined periodic handler
COAP_RES_PERIODIC_HANDLER(periodic_handler0);
COAP_RES_PERIODIC_HANDLER(periodic_handler1);

static ns_coap_resource_handler_t res_handler[] = {
    { get_handler0, post_handler0, put_handler0, delete_handler0, &res_periodic_handler0 },
    { get_handler1, post_handler1, put_handler1, delete_handler1, &res_periodic_handler1 }
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

    // put this object info container
    coap_res_obj_all.res[res_obj->id] = res_obj;

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

// res.activate("test/hello")
STATIC mp_obj_t ns_coap_resource_activate(mp_obj_t self_in, mp_obj_t uri_path_in)
{
    ns_coap_res_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->uri_path = mp_obj_str_get_str(uri_path_in);
    self->is_activated = true;
    coap_activate_resource(&self->res, self->uri_path);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_coap_resource_activate_obj, ns_coap_resource_activate);

STATIC const mp_rom_map_elem_t ns_coap_resource_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&ns_coap_resource_activate_obj) },
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
    int i;
    ns_coap_res_id_t id = 0;

    int_master_status_t int_status = int_master_read_and_disable();

    for (i= 0; i < COAP_RES_OBJ_ALL_NUM; i++) {
        if (coap_res_obj_all.res[i] == NULL) {
            id = i;
            break;
        }
    }

    if (id == 0 && i == COAP_RES_OBJ_ALL_NUM) {
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
    coap_res_obj_all.res[0]->request = request;
    coap_res_obj_all.res[0]->response = response;
    coap_res_obj_all.res[0]->buffer = buffer;
    coap_res_obj_all.res[0]->preferred_size = preferred_size;
    coap_res_obj_all.res[0]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[0]->get_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[0]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have get method"));
    }
}

static void get_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[1]->request = request;
    coap_res_obj_all.res[1]->response = response;
    coap_res_obj_all.res[1]->buffer = buffer;
    coap_res_obj_all.res[1]->preferred_size = preferred_size;
    coap_res_obj_all.res[1]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[1]->get_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[1]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have get method"));
    }
}

static void post_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[0]->request = request;
    coap_res_obj_all.res[0]->response = response;
    coap_res_obj_all.res[0]->buffer = buffer;
    coap_res_obj_all.res[0]->preferred_size = preferred_size;
    coap_res_obj_all.res[0]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[0]->post_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[0]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have post method"));
    }
}

static void post_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[1]->request = request;
    coap_res_obj_all.res[1]->response = response;
    coap_res_obj_all.res[1]->buffer = buffer;
    coap_res_obj_all.res[1]->preferred_size = preferred_size;
    coap_res_obj_all.res[1]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[1]->post_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[1]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have post method"));
    }
}

static void put_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[0]->request = request;
    coap_res_obj_all.res[0]->response = response;
    coap_res_obj_all.res[0]->buffer = buffer;
    coap_res_obj_all.res[0]->preferred_size = preferred_size;
    coap_res_obj_all.res[0]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[0]->put_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[0]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have put method"));
    }
}

static void put_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[1]->request = request;
    coap_res_obj_all.res[1]->response = response;
    coap_res_obj_all.res[1]->buffer = buffer;
    coap_res_obj_all.res[1]->preferred_size = preferred_size;
    coap_res_obj_all.res[1]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[1]->put_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[1]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have put method"));
    }
}

static void delete_handler0(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[0]->request = request;
    coap_res_obj_all.res[0]->response = response;
    coap_res_obj_all.res[0]->buffer = buffer;
    coap_res_obj_all.res[0]->preferred_size = preferred_size;
    coap_res_obj_all.res[0]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[0]->delete_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[0]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have delete method"));
    }
}

static void delete_handler1(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
                         uint16_t preferred_size, int32_t *offset)
{
    coap_res_obj_all.res[1]->request = request;
    coap_res_obj_all.res[1]->response = response;
    coap_res_obj_all.res[1]->buffer = buffer;
    coap_res_obj_all.res[1]->preferred_size = preferred_size;
    coap_res_obj_all.res[1]->offset = offset;

    mp_obj_t callback = coap_res_obj_all.res[1]->delete_obj;

    if (callback != mp_const_none) {
        mp_call_function_1(callback, MP_OBJ_FROM_PTR(coap_res_obj_all.res[1]));
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                    "ns: coap resource (%d) don't have delete method"));
    }
}

static void periodic_handler0(void)
{
    if (coap_res_obj_all.res[0] != NULL) {
        coap_notify_observers(&coap_res_obj_all.res[0]->res);
    }
}

static void periodic_handler1(void)
{
    if (coap_res_obj_all.res[1] != NULL) {
        coap_notify_observers(&coap_res_obj_all.res[1]->res);
    }
}

// predefined PROCESS_THREAD to handle CoAP message ----------------------------
// TODO:

#endif // #if APP_CONF_WITH_COAP
