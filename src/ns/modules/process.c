#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/int-master.h"
#include "ns/modules/process.h"

// Example usage to Process & Thread objects
//
//      process = ns.Process()           # only create once!
//      process.run()                    # run the internal network stack process
//      process.autostart()              # run autostart processes
//      event = process.alloc_event()    # allocate new event in process
//      print(process)                   # to print the process list and number of events waiting
//
//      # callback function example
//      def cb(ev, data):
//          # do something with the event and data
//
//      test = ns.Thread(callback=cb)    # create `test` thread with it's callback
//      test.start()                     # start `test` process thread
//      test.is_running()                # use to check `test` thread is running or not
//      test.post(event, data)           # post event with data to `test` process thread
//      test.delete()                    # delete `test` process thread
//      print(test)                      # print this thread information

const mp_obj_type_t ns_process_type;
const mp_obj_type_t ns_thread_type;

static ns_thread_container_t thread_container;
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
        for (int i = 0; i < NS_THREAD_DEPTH; i++) {
            thread_container.evid[i] = mp_obj_new_int_from_uint(PROCESS_EVENT_NONE);
        }
        thread_container.nthread = 0;
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

// test = ns.Thread(test_callback) # thread creation
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

    // check arguments (only take 1 arg (callback) (n_args=0, n_kw=1)
    if (n_args != 0 || n_kw != 1) {
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

    // get valid thread id and assigned PROCESS_EVENT_INT to this thread event
    ns_thread_id_t thread_id = thread_get_id();
    if (thread_id < 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: thread overflow! max(%d)",
                  NS_THREAD_DEPTH));
    }

    // create thread object
    ns_thread_obj_t *thread = m_new_obj(ns_thread_obj_t);
    thread->base.type = &ns_thread_type;
    thread->cb = args[ARG_callback].u_obj;
    thread->id = thread_id;
    thread->ev = thread_container.evid[thread->id]; // asigned default event after init (PROCESS_EVENT_INIT)

    return MP_OBJ_FROM_PTR(thread);
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
    self->ev = event_in;
    self->data = data_in;
    // assigned this thread obj to the container and post
    thread_container.obj[self->id] = *self;
    process_post(ns_process[self->id], PROCESS_EVENT_POLL, NULL);
    return mp_const_none;
}

STATIC mp_obj_t ns_thread_delete(mp_obj_t self_in)
{
    ns_thread_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((process_event_t)mp_obj_get_int(thread_container.evid[self->id]) ==
        PROCESS_EVENT_INIT && process_is_running(ns_process[self->id])) {
        self->ev = mp_obj_new_int_from_uint(PROCESS_EVENT_NONE);
        self->data = mp_obj_new_int_from_uint(0);
        process_post(ns_process[self->id], PROCESS_EVENT_EXIT, NULL);
    } else {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: thread id (%d) is not running or can't get event init!",
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
    mp_printf(print, "ns: thread event assigned (0x%x)\n",
              (process_event_t)mp_obj_get_int(self->ev));
    mp_printf(print, "ns: thread is running (%s)\n",
              process_is_running(ns_process[self->id]) ? "1" : "0");
    mp_printf(print, "ns: thread queue num (%d)", (int)thread_container.nthread);
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
    mp_obj_t *ev;
    ns_thread_id_t id;

    int_master_status_t int_status = int_master_read_and_disable();

    // find an unused thread id based on it's event
    ev = &thread_container.evid[0];
    id = 0;
    while (*ev != mp_obj_new_int_from_uint(PROCESS_EVENT_NONE) &&
           ev <= &thread_container.evid[NS_THREAD_DEPTH - 1]) {
        ev++;
        id++;
    }

    if (ev > &thread_container.evid[NS_THREAD_DEPTH - 1]) {
        return -1;
    }

    // this mean this thread id is occupied and initialized
    *ev = mp_obj_new_int_from_uint(PROCESS_EVENT_INIT);

    thread_container.nthread++;

    int_master_status_set(int_status);
    int_master_enable();

    return id;
}
