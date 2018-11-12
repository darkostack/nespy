import ns
from lib import process

# coap resource functions
def get(res):
    global msg_counter
    msg_counter += 1
    data = "Hello World! " + str(msg_counter)
    payload = res.set_payload_text(data)
    return payload

def client_msg_callback(res):
    get_msg = res.get_payload()
    print(get_msg, "\r")
    return;

def periodic_callback(res):
    global periodic_counter
    periodic_counter += 1
    print("periodic callback:", periodic_counter, "\r")
    if periodic_counter % 4 == 0:
        res.notify_observers()
    return;

def thread_begin():
    resource.client_ep("fd00::200:0:0:2")
    resource.client_observe("res/hello", client_msg_callback)
    return;

def thread_process(ev, data):
    if ev == event_begin:
        thread_begin()
    return;

msg_counter = 0
periodic_counter = 0

# create periodic coap resource object
resource = ns.CoapResource(attr="title=\"Periodic\";obs",
                           get=get,
                           period=500,
                           callback=periodic_callback)

# create event object for hello thread boot up sequence
event_begin = process.ns.alloc_event()

# create hello thread object
thread = ns.Thread(callback=thread_process)
