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
    init.node_id(2)
    init.netstack()
    init.ipv6_addr()

    print(init) # print initialized info
    
    print("Nespy command line interface: use Ctrl-D to exit")
    print("type `help` to see list of commands")

    init.platform(1)

    # autostart internal nespy processes
    process.autostart()

    while True:
        process.run()
        unix.process_update()

if __name__ == "__main__":
    main()
