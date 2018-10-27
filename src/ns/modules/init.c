#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/modules/init.h"

// Example usage to Init objects
//
//      init = ns.Init()
//
//      init.platform(1)            # init platform stage one
//      init.clock()
//      init.rtimer()
//      init.process()
//      init.etimer()
//      init.ctimer()
//      init.watchdog()
//      init.energerst()
//      init.stack_check()
//      init.platform(2)            # init platform stage two
//      init.netstack()
//      init.node_id()
//      init.ipv6_addr()
//      init.platform(3)            # init platform stage three
//      init.rpl_border_router()
//      init.orchestra()
//      init.shell()
//      init.coap_engine()
//      init.simple_energest()
//      init.tsch_cs_adaptions()
