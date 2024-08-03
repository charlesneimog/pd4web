import os
import platform
import sys
import traceback

import requests


def RedExceptions(exc_type, exc_value, exc_traceback):
    """Just to print exceptions in red"""
    formatted_exception = "".join(traceback.format_exception(exc_type, exc_value, exc_traceback))
    print(f"\033[91m{formatted_exception}\033[0m")


sys.excepthook = RedExceptions


def pd4web_print(text, color=None, bright=False, silence=False, pd4web=False):
    tab = " " * 4
    if pd4web:
        if color == "red":
            print("ERROR: " + text)
        elif color == "yellow":
            print("WARNING: " + text)
        else:
            print(text)
        return
    if silence:
        return
    try:
        if color is None:
            color_code = ""
        else:
            color_code = {
                "red": "\033[91;1m" if bright else "\033[91m",
                "green": "\033[92;1m" if bright else "\033[92m",
                "yellow": "\033[93;1m" if bright else "\033[93m",
                "blue": "\033[94;1m" if bright else "\033[94m",
                "magenta": "\033[95;1m" if bright else "\033[95m",
                "cyan": "\033[96;1m" if bright else "\033[96m",
                "lightgray": "\033[97;1m" if bright else "\033[97m",
                "darkgray": "\033[90;1m" if bright else "\033[90m",
                "lightred": "\033[91;1m" if bright else "\033[91m",
                "lightgreen": "\033[92;1m" if bright else "\033[92m",
                "lightyellow": "\033[93;1m" if bright else "\033[93m",
                "lightblue": "\033[94;1m" if bright else "\033[94m",
                "lightmagenta": "\033[95;1m" if bright else "\033[95m",
                "lightcyan": "\033[96;1m" if bright else "\033[96m",
                "white": "\033[97;1m" if bright else "\033[97m",
                "blackbold": "\033[1m",
                "blackunderline": "\033[4m",
                "dark_grey": "\033[90m",
            }.get(color.lower(), "")
        reset_code = "\033[0m"

        if color == "red":
            print(tab + color_code + "🔴️ ERROR: " + text + reset_code)
        elif color == "yellow":
            print(tab + color_code + "🟡️ WARNING: " + text + reset_code)

        elif color == "green":
            print(tab + color_code + "🟢️ " + text + reset_code)
        elif color == "blue":
            print(tab + color_code + "🔵️ " + text + reset_code)
        elif color == "magenta":
            print(tab + color_code + "🟣️ " + text + reset_code)
        else:
            print(tab + color_code + text + reset_code)
    except:
        print(text)


def getPrintValue(color, bright=False):
    if color is None:
        color_code = ""
    else:
        color_code = {
            "red": "\033[91;1m" if bright else "\033[91m",
            "green": "\033[92;1m" if bright else "\033[92m",
            "yellow": "\033[93;1m" if bright else "\033[93m",
            "blue": "\033[94;1m" if bright else "\033[94m",
            "magenta": "\033[95;1m" if bright else "\033[95m",
            "cyan": "\033[96;1m" if bright else "\033[96m",
            "lightgray": "\033[97;1m" if bright else "\033[97m",
            "darkgray": "\033[90;1m" if bright else "\033[90m",
            "lightred": "\033[91;1m" if bright else "\033[91m",
            "lightgreen": "\033[92;1m" if bright else "\033[92m",
            "lightyellow": "\033[93;1m" if bright else "\033[93m",
            "lightblue": "\033[94;1m" if bright else "\033[94m",
            "lightmagenta": "\033[95;1m" if bright else "\033[95m",
            "lightcyan": "\033[96;1m" if bright else "\033[96m",
            "white": "\033[97;1m" if bright else "\033[97m",
            "blackbold": "\033[1m",
            "blackunderline": "\033[4m",
            "dark_grey": "\033[90m",
            "reset": "\033[0m",
        }.get(color.lower(), "")

    return color_code


def fixPaths(path):
    if platform.system() == "Windows":
        path = path.replace("/", "\\")
        return path
    else:
        path = path.replace("\\", "/")
        return path


def printProgressBar(
    iteration,
    total,
    prefix="",
    suffix="",
    decimals=1,
    length=100,
    fill="█",
    printEnd="\r",
):
    if total == 0:
        sys.stdout.write(f"\r{prefix} Downloading... {suffix}")
        sys.stdout.flush()
    else:
        percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
        filled_length = int(length * iteration // total)
        bar = fill * filled_length + "-" * (length - filled_length)
        sys.stdout.write(f"\r{prefix} |{bar}| {percent}% {suffix}")
        sys.stdout.flush()
    # Print New Line on Complete
    if iteration == total:
        print()


def DownloadZipFile(url, path2save):
    print()
    response = requests.get(url, stream=True)
    if response.status_code == 200:
        total_size = int(response.headers.get("content-length", 0))
        block_size = 1024
        with open(path2save, "wb") as file:
            for data in response.iter_content(block_size):
                file.write(data)
                printProgressBar(
                    file.tell(),
                    total_size,
                    prefix="Progress:",
                    suffix="Complete",
                    length=50,
                )
        if total_size != 0 and os.path.getsize(path2save) != total_size:
            raise Exception("Error downloading the file")
    else:
        raise Exception(f"Error {response.status_code} while downloading the file")


class emccPaths:
    def __init__(self):
        PdWebCompilerPath = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        if platform.system() == "Windows":
            PdWebCompilerPath = PdWebCompilerPath.replace("/", "\\")
            # if first char of PdWebCompilerPath is a space remove it
            if PdWebCompilerPath[0] == " ":
                PdWebCompilerPath = PdWebCompilerPath[1:]
            self.cmake = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emcmake.bat" cmake '
            self.configure = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emconfigure.bat" '
            self.make = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emmake.bat" make '
            self.emcc = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emcc.bat" '
            self.emsdk = f'"{PdWebCompilerPath}\\emsdk\\emsdk.bat" '
            self.emsdk_env = f'"{PdWebCompilerPath}\\emsdk\\emsdk_env.bat" '
        else:
            self.cmake = PdWebCompilerPath + "/emsdk/upstream/emscripten/emcmake cmake "
            self.configure = PdWebCompilerPath + "/emsdk/upstream/emscripten/emconfigure "
            self.make = PdWebCompilerPath + "/emsdk/upstream/emscripten/emmake make "
            self.emcc = PdWebCompilerPath + "/emsdk/upstream/emscripten/emcc "
            self.emsdk = PdWebCompilerPath + "/emsdk/emsdk "
            self.emsdk_env = "source " + PdWebCompilerPath + "/emsdk/emsdk_env.sh "
