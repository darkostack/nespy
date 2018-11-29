import nespy

instance = nespy.Instance()
platform = nespy.Platform(inst=instance, id=1)

def timer_cb():
    print("timer expired -> post tasks!\r")
    test1_task.post()
    test2_task.post()
    test3_task.post()
    print("done\r")

def test1_cb():
    print("test1 task executed\r")

def test2_cb():
    print("test2 task executed\r")

def test3_cb():
    print("test3 task executed\r")

timer = nespy.TimerMilli(inst=instance, cb=timer_cb)

test1_task = nespy.Tasklet(inst=instance, cb=test1_cb)
test2_task = nespy.Tasklet(inst=instance, cb=test2_cb)
test3_task = nespy.Tasklet(inst=instance, cb=test3_cb)

def main():
    platform.system_init()
    timer.start(1000)
    while True:
        platform.tasklet_run()
        platform.process_drivers()

if __name__ == "__main__":
    main()
