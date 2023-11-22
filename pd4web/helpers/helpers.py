import os
import platform


def myprint(text, color=None, bright=False):
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

        tab = " " * 4
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


def fixPaths(path):
    if platform.system() == "Windows":
        path = path.replace("/", "\\")
        return path
    else:
        path = path.replace("\\", "/")
        return path


class emccPaths:
    def __init__(self):
        PdWebCompilerPath = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
        if platform.system() == "Windows":
            PdWebCompilerPath = PdWebCompilerPath.replace("/", "\\")
            # if first char of PdWebCompilerPath is a space remove it
            if PdWebCompilerPath[0] == " ":
                PdWebCompilerPath = PdWebCompilerPath[1:]
            self.cmake = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emcmake.bat" cmake '
            self.configure = (
                f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emconfigure.bat" '
            )
            self.make = (
                f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emmake.bat" make '
            )
            self.emcc = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emcc.bat" '
            self.emsdk = f'"{PdWebCompilerPath}\\emsdk\\emsdk.bat" '
            self.emsdk_env = f'"{PdWebCompilerPath}\\emsdk\\emsdk_env.bat" '
        else:
            self.cmake = PdWebCompilerPath + "/emsdk/upstream/emscripten/emcmake cmake "
            self.configure = (
                PdWebCompilerPath + "/emsdk/upstream/emscripten/emconfigure "
            )
            self.make = PdWebCompilerPath + "/emsdk/upstream/emscripten/emmake make "
            self.emcc = PdWebCompilerPath + "/emsdk/upstream/emscripten/emcc "
            self.emsdk = PdWebCompilerPath + "/emsdk/emsdk "
            self.emsdk_env = "source " + PdWebCompilerPath + "/emsdk/emsdk_env.sh "
