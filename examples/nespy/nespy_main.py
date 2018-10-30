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
    
    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        unix.process_update()

if __name__ == "__main__":
    main()
