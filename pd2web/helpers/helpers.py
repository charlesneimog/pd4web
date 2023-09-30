import os
import platform

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
        print("    " + str)

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
            self.configure = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emconfigure.bat" '
            self.make = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emmake.bat" make '
            self.emcc = f'"{PdWebCompilerPath}\\emsdk\\upstream\\emscripten\\emcc.bat" '
            self.emsdk = f'"{PdWebCompilerPath}\\emsdk\\emsdk.bat" '
            self.emsdk_env = f'"{PdWebCompilerPath}\\emsdk\\emsdk_env.bat" '
        else:
            self.cmake = PdWebCompilerPath + '/emsdk/upstream/emscripten/emcmake cmake '
            self.configure = PdWebCompilerPath + '/emsdk/upstream/emscripten/emconfigure '
            self.make = PdWebCompilerPath + '/emsdk/upstream/emscripten/emmake make '
            self.emcc = PdWebCompilerPath + '/emsdk/upstream/emscripten/emcc '
            self.emsdk = PdWebCompilerPath + '/emsdk/emsdk '
            self.emsdk_env = PdWebCompilerPath + '/emsdk/emsdk_env.sh '


                        
        
