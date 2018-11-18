import nespy

platform = nespy.Platform(id=1)
instance = nespy.Instance()

def timer_cb():
    print("timer expired\r")
    test_task.post()

def test_task():
    print("test task executed\r")

timer = nespy.Timer(inst=instance, cb=timer_cb)
test_task = nespy.Tasklet(inst=instance, task=test_task)

def main():
    platform.system_init()
    timer.start(1000)
    while True:
        platform.tasklet_run(instance)
        platform.process_drivers(instance)

if __name__ == "__main__":
    main()
