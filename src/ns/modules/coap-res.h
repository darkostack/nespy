#ifndef NS_MODULES_COAP_RES_H_
#define NS_MODULES_COAP_RES_H_

#define COAP_RES_OBJ_ALL_NUM 2

#define COAP_RES_HANDLER(name) \
    static void name(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)

#define COAP_RES_PERIODIC_HANDLER(name) \
    static void name(void); \
    static coap_periodic_resource_t res_##name = { 0, { 0 }, name }
    

typedef int ns_coap_res_id_t;

typedef struct _ns_coap_res_obj_t {
    mp_obj_base_t base;
    mp_obj_t get_obj;
    mp_obj_t post_obj;
    mp_obj_t put_obj;
    mp_obj_t delete_obj;
    mp_obj_t periodic_obj;
    ns_coap_res_id_t id;
    coap_resource_t res;
    coap_message_t *request;
    coap_message_t *response;
    uint8_t *buffer;
    uint16_t preferred_size;
    int32_t *offset;
    const char *uri_path;
    const char *query;
    bool is_activated;
} ns_coap_res_obj_t;

typedef struct _ns_coap_res_obj_all_t {
    ns_coap_res_obj_t *res[COAP_RES_OBJ_ALL_NUM];
    uint8_t remain;
} ns_coap_res_obj_all_t;

struct ns_coap_resource_handler_s {
    coap_resource_handler_t get;
    coap_resource_handler_t post;
    coap_resource_handler_t put;
    coap_resource_handler_t del;
    coap_periodic_resource_t *periodic;
};

typedef struct ns_coap_resource_handler_s ns_coap_resource_handler_t;

#endif // NS_MODULES_COAP_RES_H_
