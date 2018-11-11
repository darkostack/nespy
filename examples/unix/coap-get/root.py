from lib import init
from lib import process
from lib import platform
from lib import hello

def callback():
    hello.thread.post(hello.event_begin, None)
    return;

def main():
    init.ns.node_id(1)
    init.ns.netstack()
    init.ns.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    # enable uart command line interface
    init.ns.cli()

    # enable coap application
    init.ns.coap()

    # set this node as a root with "fd00::" prefix
    init.ns.set_root("fd00::");

    # use this to get notification when network is up
    init.ns.network_up(callback)

    # start coap resource thread
    hello.thread.start()

    # autostart internal nespy processes
    process.ns.autostart()

    while True:
        process.ns.run()
        platform.ns.process_update()

if __name__ == "__main__":
    main()
