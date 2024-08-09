import os
import platform
import zipfile
import cmake

import requests

from .Helpers import pd4web_print
from .Pd4Web import Pd4Web


class ExternalsCompiler:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.InitVariables()
        if not os.path.exists(Pd4Web.APPDATA + "/emsdk"):
            emccGithub = "https://api.github.com/repos/emscripten-core/emsdk/tags"
            response = requests.get(emccGithub)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            EmccZip = self.Pd4Web.APPDATA + "/emcc.zip"
            OK = Pd4Web.DownloadZip(sourceCodeLink, EmccZip, "emcc")
            if not OK:
                raise Exception("Failed to download emcc")
            with zipfile.ZipFile(EmccZip, "r") as zip_ref:
                zip_ref.extractall(Pd4Web.APPDATA)
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.Pd4Web.APPDATA + "/" + extractFolderName,
                    self.Pd4Web.APPDATA + "/emsdk",
                )
                try:
                    os.remove(EmccZip)
                except:
                    pd4web_print(
                        "Failed to remove emcc.zip",
                        color="yellow",
                        silence=self.Pd4Web.SILENCE,
                        pd4web=self.Pd4Web.PD_EXTERNAL,
                    )
            self.InstallEMCC()

    def InitVariables(self):
        self.EMSDK = self.Pd4Web.APPDATA + "/emsdk/emsdk"
        if platform.system() == "Windows":
            self.EMCMAKE = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emcmake.bat"
        else:
            self.EMCMAKE = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emcmake"
        self.CMAKE = self.GetCmake()
        self.EMCC = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emcc"

    def GetCmake(self):
        cmake_dir = cmake.__file__
        cmake_dir = os.path.dirname(cmake_dir)
        cmake_dir = os.path.join(cmake_dir, "data", "bin")
        cmake_bin = os.path.join(cmake_dir, "cmake")
        cmake_bin = os.path.abspath(cmake_bin)
        if not os.path.exists(cmake_bin):
            raise Exception("Cmake (module) is not installed. Please install it.")
        else:
            return cmake_bin

    def InstallEMCC(self):
        if platform.system() == "Windows":
            os.system(f"cmd /C {self.EMSDK} install latest")
            os.system(f"cmd /C {self.EMSDK} activate latest")
        else:
            os.environ["EMSDK_QUIET"] = "1"
            os.system(f"chmod +x {self.EMSDK}")
            os.system(f"{self.EMSDK} install latest")
            os.system(f"{self.EMSDK} activate latest")

    def __str__(self):
        return "< Compiler >"

    def __repr__(self):
        return "< Compiler >"
