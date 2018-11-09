import ns
from resource import hello

# initialize ns objects
init     = ns.Init()
process  = ns.Process()
platform = ns.Platform()

def callback():
    # initialize coap resources
    hello.resource.server_activate("res/hello")

def main():
    init.node_id(2)
    init.netstack()
    init.platform()
    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")
    # enable uart command line interface
    init.cli()
    # enable coap application
    init.coap()
    # use this to get notification when node is join the network
    init.network_up(callback)
    # autostart internal nespy processes
    process.autostart()
    while True:
        process.run()
        platform.process_update()

if __name__ == "__main__":
    main()
