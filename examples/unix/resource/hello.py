import ns

def get(res):
    payload = res.set_payload_text("Hello World!")
    return payload

def client_msg_callback(res):
    message = res.get_payload()
    print(message)

# initialize coap resource objects
resource = ns.CoapResource(attr="title=\"Hello World!\"", get=get)
