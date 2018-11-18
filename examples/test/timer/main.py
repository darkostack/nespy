import nespy

instance = nespy.Instance()
platform = nespy.Platform(inst=instance, id=1)

def timer1_cb():
    print("timer1 expired\r")

def timer2_cb():
    print("timer2 expired\r")

def timer3_cb():
    print("timer3 expired\r");

timer1 = nespy.Timer(inst=instance, cb=timer1_cb)
timer2 = nespy.Timer(inst=instance, cb=timer2_cb)
timer3 = nespy.Timer(inst=instance, cb=timer3_cb)

def main():
    platform.system_init()
    timer1.start(1000)
    timer2.start(500)
    timer3.start(700)
    while True:
        platform.process_drivers()

if __name__ == "__main__":
    main()
