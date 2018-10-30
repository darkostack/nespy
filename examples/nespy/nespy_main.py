import ns

def main():
    init = ns.Init()
    process = ns.Process()
    unix = ns.Unix()

    init.clock()
    init.rtimer()
    init.process()
    init.etimer()
    init.ctimer()
    init.netstack()
    init.node_id()
    init.ipv6_addr()
    
    print(init)

    process.autostart()

    while True:
        process.run()
        unix.process_update()

if __name__ == "__main__":
    main()
