import nespy

platform = nespy.Platform()
process = nespy.Process()
init = nespy.Init()

def callback():
    # network is ready, print network config
    print(init)
    return

def main():
    init.node_id(1)
    init.protocol()
    init.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    # enable uart command line interface
    init.cli()

    # set this node as a root with "fd00::" prefix
    init.root("fd00::");

    # start the network and get notification when network is ready
    init.network(callback)

    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        platform.process_update()

if __name__ == "__main__":
    main()
