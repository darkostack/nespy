import nespy

platform = nespy.Platform(id=1)
instance = nespy.Instance()

def callback1():
    print("timer1 expired\r")

def callback2():
    print("timer2 expired\r")

def callback3():
    print("timer3 expired\r");

timer1 = nespy.Timer(inst=instance, cb=callback1)
timer2 = nespy.Timer(inst=instance, cb=callback2)
timer3 = nespy.Timer(inst=instance, cb=callback3)

def main():
    platform.system_init()
    timer1.start(1000)
    timer2.start(500)
    timer3.start(1000)
    while True:
        platform.process_drivers(instance)

if __name__ == "__main__":
    main()
