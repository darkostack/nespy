import nespy

instance = nespy.Instance()
platform = nespy.Platform(inst=instance, id=1)

def milli_cb():
    print("milliseconds timer expired\r")

def micro_cb():
    print("microseconds timer expired\r")

milli = nespy.TimerMilli(inst=instance, cb=milli_cb)
micro = nespy.TimerMicro(inst=instance, cb=micro_cb)

def main():
    platform.system_init()
    # Note: this timer should fired at the same time 1 second in the future
    milli.start(1000)
    micro.start(1000000)
    while True:
        platform.process_drivers()

if __name__ == "__main__":
    main()
