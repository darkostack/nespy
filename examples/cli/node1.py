import ns

# initialize ns objects
init     = ns.Init()
process  = ns.Process()
platform = ns.Platform()

def main():
    init.node_id(1)
    init.netstack()
    init.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    init.cli() # enable netstack command line interface (optional)

    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        platform.process_update()

if __name__ == "__main__":
    main()
