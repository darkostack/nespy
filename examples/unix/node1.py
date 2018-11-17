import nespy

platform = nespy.Platform(id=1)
instance = nespy.Instance()

def main():
    platform.system_init()

    while True:
        platform.process_drivers(instance)

if __name__ == "__main__":
    main()
