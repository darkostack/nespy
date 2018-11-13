#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/int-master.h"
#include "ns/lib/py/obj-process.h"
#include <stdio.h>

// Example usage to Process & Thread objects
//
//      process = ns.Process()           # only create once!
//      process.run()                    # run the internal network stack process
//      process.autostart()              # run autostart processes
//      event = process.alloc_event()    # allocate new event in process
//      print(process)                   # to print the process list and number of events waiting
//
//      # callback function example
//      test = ns.Thread(callback=cb)
//      test.start()                     # start `test` process thread
//      test.is_running()                # use to check `test` thread is running or not
//      test.post(event, data)           # post event with data to `test` process thread
//      test.delete()                    # delete `test` process thread
//      print(test)                      # print this thread information

const mp_obj_type_t ns_process_type;
const mp_obj_type_t ns_thread_type;

static ns_thread_obj_all_t thread_obj_all;
static bool is_process_obj_created = false;
static ns_thread_id_t thread_get_id(void);

PROCESS(thread0, "thread0");
PROCESS(thread1, "thread1");
PROCESS(thread2, "thread2");
PROCESS(thread3, "thread3");
PROCESS(thread4, "thread4");
PROCESS(thread5, "thread5");
PROCESS(thread6, "thread6");
PROCESS(thread7, "thread7");
PROCESS(thread8, "thread8");
PROCESS(thread9, "thread9");

struct process *ns_process[] = {
    &thread0,
    &thread1,
    &thread2,
    &thread3,
    &thread4,
    &thread5,
    &thread6,
    &thread7,
    &thread8,
    &thread9,
};

NS_PROCESS_THREAD(0);
NS_PROCESS_THREAD(1);
NS_PROCESS_THREAD(2);
NS_PROCESS_THREAD(3);
NS_PROCESS_THREAD(4);
NS_PROCESS_THREAD(5);
NS_PROCESS_THREAD(6);
NS_PROCESS_THREAD(7);
NS_PROCESS_THREAD(8);
NS_PROCESS_THREAD(9);

// process = ns.Process() constructor
STATIC mp_obj_t ns_process_make_new(const mp_obj_type_t *type,
                                    size_t n_args,
                                    size_t n_kw,
                                    const mp_obj_t *all_args)
{
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // make sure we only create process object once
    if (is_process_obj_created) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: can't create more than one process object"));
    } else {
        // initialize thread status
        for (int i = 0; i < THREAD_OBJ_ALL_NUM; i++) {
            thread_obj_all.thread[i].cb = mp_const_none;
            thread_obj_all.thread[i].id = 0;
            thread_obj_all.thread[i].data = mp_const_none;
            thread_obj_all.thread[i].is_used = false;
        }
        thread_obj_all.remain = THREAD_OBJ_ALL_NUM;
    }

    // create process object
    ns_process_base_obj_t *process = m_new_obj(ns_process_base_obj_t);
    process->base.type = &ns_process_type;
    is_process_obj_created = true;

    return MP_OBJ_FROM_PTR(process);
}

// process.run()
STATIC mp_obj_t ns_process_run(mp_obj_t self_in)
{
    return mp_obj_new_int(process_run());
}

// process.autostart()
STATIC mp_obj_t ns_process_autostart(mp_obj_t self_in)
{
    autostart_start(autostart_processes);
    return mp_const_none;
}

// process.alloc_event()
STATIC mp_obj_t ns_process_alloc_event(mp_obj_t self_in)
{
    return mp_obj_new_int_from_uint(process_alloc_event());
}

// print(process)
STATIC void ns_process_print(const mp_print_t *print,
                             mp_obj_t self_in,
                             mp_print_kind_t kind)
{
    struct process *p;
    mp_printf(print, "ns: process list ---\n");
    for (p = process_list; p != NULL; p = p->next) {
        mp_printf(print, "ns: %s\n", p->name);
    }
    mp_printf(print, "ns: ----------------\n");
    mp_printf(print, "ns: process events waiting (%d)", process_nevents());
}

// test = ns.Thread(callback=cb) # thread creation
STATIC mp_obj_t ns_thread_make_new(const mp_obj_type_t *type,
                                   size_t n_args,
                                   size_t n_kw,
                                   const mp_obj_t *all_args)
{
    // make sure the process obj was created
    if (is_process_obj_created == false) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: please create process object first!"));
    }

    if (n_args == 0 && n_kw == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: invalid argument!"));
    }

    enum { ARG_callback };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // make sure at least we get thread callback argument
    if (args[ARG_callback].u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: thread callback can't empty!"));
    }

    // get valid thread id and assigned PROCESS_EVENT_INT to this thread event
    ns_thread_id_t thread_id = thread_get_id();

    if (thread_id < 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: thread container overflow! max(%d)",
                  THREAD_OBJ_ALL_NUM));
    }

    // create thread object
    ns_thread_obj_t *t = m_new_obj(ns_thread_obj_t);
    t->base.type = &ns_thread_type;
    t->cb = args[ARG_callback].u_obj;
    t->id = thread_id;
    t->data = mp_const_none;
    t->is_used = true;

    // store this object to thread obj all container
    thread_obj_all.thread[t->id] = *t;

    return MP_OBJ_FROM_PTR(t);
}

// test.start() # start this thread
STATIC mp_obj_t ns_thread_start(mp_obj_t self_in)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    process_start(ns_process[self->id], NULL);
    return mp_const_none;
}

// test.is_running() # check thread running status, return bool (true/false)
STATIC mp_obj_t ns_thread_is_running(mp_obj_t self_in)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool ret = process_is_running(ns_process[self->id]) ? true : false;
    return mp_obj_new_bool(ret);
}

// test.post(test_event, test_data)
STATIC mp_obj_t ns_thread_post(mp_obj_t self_in,
                               mp_obj_t event_in,
                               mp_obj_t data_in)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self = &thread_obj_all.thread[self->id];
    self->data = data_in;
    process_event_t event = (process_event_t)mp_obj_get_int(event_in);
    process_post(ns_process[self->id], event, NULL);
    return mp_const_none;
}

STATIC mp_obj_t ns_thread_delete(mp_obj_t self_in)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->is_used) {
        process_post(ns_process[self->id], PROCESS_EVENT_EXIT, NULL);
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: thread id (%d) is not used",
                  (int)self->id));
    }
    return mp_const_none;
}

STATIC void ns_thread_print(const mp_print_t *print,
                            mp_obj_t self_in,
                            mp_print_kind_t kind)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ns: thread id (%d)\n", (int)self->id);
    mp_printf(print, "ns: thread is running (%s)\n",
              process_is_running(ns_process[self->id]) ? "1" : "0");
    mp_printf(print, "ns: thread resource remain (%d)", (int)thread_obj_all.remain);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_process_run_obj, ns_process_run);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_process_autostart_obj, ns_process_autostart);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_process_alloc_event_obj, ns_process_alloc_event);

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_thread_start_obj, ns_thread_start);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_thread_is_running_obj, ns_thread_is_running);
STATIC MP_DEFINE_CONST_FUN_OBJ_3(ns_thread_post_obj, ns_thread_post);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_thread_delete_obj, ns_thread_delete);

STATIC const mp_rom_map_elem_t ns_process_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&ns_process_run_obj) },
    { MP_ROM_QSTR(MP_QSTR_autostart), MP_ROM_PTR(&ns_process_autostart_obj) },
    { MP_ROM_QSTR(MP_QSTR_alloc_event), MP_ROM_PTR(&ns_process_alloc_event_obj) },
};

STATIC const mp_rom_map_elem_t ns_thread_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&ns_thread_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_is_running), MP_ROM_PTR(&ns_thread_is_running_obj) },
    { MP_ROM_QSTR(MP_QSTR_post), MP_ROM_PTR(&ns_thread_post_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&ns_thread_delete_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_process_locals_dict, ns_process_locals_dict_table);
STATIC MP_DEFINE_CONST_DICT(ns_thread_locals_dict, ns_thread_locals_dict_table);

const mp_obj_type_t ns_process_type = {
    { &mp_type_type },
    .name = MP_QSTR_Process,
    .print = ns_process_print,
    .make_new = ns_process_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_process_locals_dict,
};

const mp_obj_type_t ns_thread_type = {
    { &mp_type_type },
    .name = MP_QSTR_Thread,
    .print = ns_thread_print,
    .make_new = ns_thread_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_thread_locals_dict,
};

static ns_thread_id_t thread_get_id(void)
{
    int i;
    bool is_get_id = false;
    ns_thread_id_t id = 0;

    int_master_status_t int_status = int_master_read_and_disable();

    for (i = 0; i < THREAD_OBJ_ALL_NUM; i++) {
        if (thread_obj_all.thread[i].is_used == false) {
            is_get_id = true;
            id = i;
            break;
        }
    }

    if (!is_get_id) {
        return -1;
    }

    thread_obj_all.remain--;

    int_master_status_set(int_status);
    int_master_enable();

    return id;
}
