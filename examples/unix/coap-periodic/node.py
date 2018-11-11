from lib import init
from lib import process
from lib import platform
from lib import hello

def callback():
    # initialize coap resources
    hello.resource.server_activate("res/hello")

def main():
    init.ns.node_id(2)
    init.ns.netstack()
    init.ns.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    # enable uart command line interface
    init.ns.cli()

    # enable coap application
    init.ns.coap()

    # use this to get notification when node is join the network
    init.ns.network_up(callback)

    # autostart internal nespy processes
    process.ns.autostart()

    while True:
        process.ns.run()
        platform.ns.process_update()

if __name__ == "__main__":
    main()
