import nespy

instance = nespy.Instance()
platform = nespy.Platform(inst=instance, id=1)

def timer_ms_cb():
    print("milliseconds timer expired\r")

def timer_us_cb():
    print("microseconds timer expired\r")

timer_ms = nespy.TimerMilli(inst=instance, cb=timer_ms_cb)
timer_us = nespy.TimerMicro(inst=instance, cb=timer_us_cb)

def main():
    platform.system_init()
    # Note: this timer should fired at the same time 1 second in the future
    timer_ms.start(1000)
    timer_us.start(1000000)
    while True:
        platform.process_drivers()

if __name__ == "__main__":
    main()
