import ns

# initialize basic objects
init    = ns.Init()
process = ns.Process()
unix    = ns.Unix()

def main():
    init.node_id(2)
    init.netstack()
    init.platform()

    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    init.cli() # enable netstack command line interface (optional)

    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        unix.process_update()

if __name__ == "__main__":
    main()
