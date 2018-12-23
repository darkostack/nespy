import nespy

instance = nespy.Instance()
platform = nespy.Platform(inst=instance, id=1)

def main():
    platform.system_init()
    while True:
        platform.process_drivers()

if __name__ == "__main__":
    main()
