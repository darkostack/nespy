from lib import ns
from lib import hello

def callback():
    # network is ready, initialize coap resources
    hello.thread.post(hello.event_begin, None)
    return;

def main():
    ns.init.node_id(1)
    ns.init.protocol()
    ns.init.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    # enable uart command line interface
    ns.init.cli()

    # enable coap application
    ns.init.coap()

    # set this node as a root with "fd00::" prefix
    ns.init.root("fd00::");

    # start the network and get notification when network is ready
    ns.init.network(callback)

    # start coap resources thread
    hello.thread.start()

    # autostart internal nespy processes
    ns.process.autostart()

    while True:
        ns.process.run()
        ns.platform.process_update()

if __name__ == "__main__":
    main()
