from pd4web import Pd4Web


if __name__ == "__main__":
    Pd4WebInstance = Pd4Web()
    # add current directory to the path of the system
    Pd4WebInstance.PD_EXTERNAL = True
    Pd4WebInstance.argParse()
