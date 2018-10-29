#include "py/nlr.h"
#include "py/runtime.h"

// Example usage to Hello objects
//
//      hello = ns.Hello()  # create hello obj
//      hello.add("World")  # add another string next to hello
//      print(hello)        # print hello obj

const mp_obj_type_t ns_hello_type;

typedef struct _ns_hello_obj_t {
    mp_obj_base_t base;
    const char *str;
} ns_hello_obj_t;

STATIC mp_obj_t ns_hello_make_new(const mp_obj_type_t *type,
                                  size_t n_args,
                                  size_t n_kw,
                                  const mp_obj_t *all_args)
{
    // check arguments (min: 0, max 0)
    mp_arg_check_num(n_args, n_kw, 0, 0, true);
    ns_hello_obj_t *hello = m_new_obj(ns_hello_obj_t);
    hello->base.type = &ns_hello_type;
    hello->str = NULL;
    return MP_OBJ_FROM_PTR(hello);
}

STATIC mp_obj_t ns_hello_add(mp_obj_t self_in, mp_obj_t str_in)
{
    ns_hello_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->str = mp_obj_str_get_str(str_in);
    return mp_const_none;
}

STATIC void ns_hello_print(const mp_print_t *print,
                           mp_obj_t self_in,
                           mp_print_kind_t kind)
{
    ns_hello_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "Hello %s!", self->str);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_hello_add_obj, ns_hello_add);

STATIC const mp_rom_map_elem_t ns_hello_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_add), MP_ROM_PTR(&ns_hello_add_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_hello_locals_dict, ns_hello_locals_dict_table);

const mp_obj_type_t ns_hello_type = {
    { &mp_type_type },
    .name = MP_QSTR_Hello,
    .print = ns_hello_print,
    .make_new = ns_hello_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_hello_locals_dict,
};
