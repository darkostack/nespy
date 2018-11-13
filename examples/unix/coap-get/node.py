from lib import ns
from lib import hello

def callback():
    # network is ready, initialize coap resource servers
    hello.resource.server_activate("res/hello")

def main():
    ns.init.node_id(2)
    ns.init.protocol()
    ns.init.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    # enable uart command line interface
    ns.init.cli()

    # enable coap application
    ns.init.coap()

    # start the network and get notification when network is ready
    ns.init.network(callback)

    # autostart internal nespy processes
    ns.process.autostart()

    while True:
        ns.process.run()
        ns.platform.process_update()

if __name__ == "__main__":
    main()
