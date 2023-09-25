


def myprint(str, color=None):
    if color is None:
        print("    " + str)
        return
    if color == 'red':
        print("\033[91m" + "    " + str + "\033[0m")
    elif color == 'green':
        print("\033[92m" + "    " + str + "\033[0m")
    elif color == 'yellow':
        print("\033[93m" + "    " + str + "\033[0m")
    elif color == 'blue':
        print("\033[94m" + "    " + str + "\033[0m")
    elif color == 'magenta':
        print("\033[95m" + "    " + str + "\033[0m")
    elif color == 'cyan':
        print("\033[96m" + "    " + str + "\033[0m")
    else:
        print(str)

