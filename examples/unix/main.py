import ns

# initialize basic objects
init    = ns.Init()
process = ns.Process()
unix    = ns.Unix()

def main():
    init.clock()
    init.rtimer()
    init.process()
    init.etimer()
    init.ctimer()
    init.netstack()
    init.node_id()
    init.ipv6_addr()
    
    print("Nespy v1.0 command line interface: use Ctrl-D to exit")

    init.platform(1)

    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        unix.process_update()

if __name__ == "__main__":
    main()
