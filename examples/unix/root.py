import ns
from resource import hello

# initialize ns objects
init     = ns.Init()
process  = ns.Process()
platform = ns.Platform()

def callback():
    hello.resource.client_ep("fd00::200:0:0:2")
    hello.resource.client_get("res/hello", hello.client_msg_callback)

def main():
    init.node_id(1)
    init.netstack()
    init.platform()
    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")
    # enable uart command line interface
    init.cli()
    # enable coap application
    init.coap()
    # set this node as a root with "fd00::" prefix
    init.set_root("fd00::");
    # use this to get notification when network is up
    init.network_up(callback)
    # autostart internal nespy processes
    process.autostart()
    while True:
        process.run()
        platform.process_update()

if __name__ == "__main__":
    main()
