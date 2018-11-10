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
    message = res.get_payload()
    print(message, "\r")
    return;

# hello thread functions
def thread_begin():
    resource.client_ep("fd00::200:0:0:2")
    timer.start()
    return;

def thread_process(ev, data):
    if ev == event_begin:
        thread_begin()
    elif ev == timer.event():
        if (timer.expired()):
            resource.client_get("res/hello", client_msg_callback)
            timer.reset()
    return;

msg_counter = 0

# create coap resource object
resource = ns.CoapResource(attr="title=\"Hello World!\"", get=get)

# create event object for hello thread boot up sequence
event_begin = process.ns.alloc_event()

# create event timer object with 500ms period
timer = ns.Etimer(period=500)

# create hello thread object
thread = ns.Thread(callback=thread_process)
