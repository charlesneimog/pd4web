import os
import platform
import zipfile

import requests

from .Helpers import pd4web_print
from .Pd4Web import Pd4Web


class Compiler:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.InitVariables()
        if not os.path.exists(Pd4Web.PD4WEB_ROOT + "/emsdk"):
            emccGithub = "https://api.github.com/repos/emscripten-core/emsdk/tags"
            response = requests.get(emccGithub)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            EmccZip = self.Pd4Web.PD4WEB_ROOT + "/emcc.zip"
            OK = Pd4Web.DownloadZip(sourceCodeLink, EmccZip, "emcc")
            if not OK:
                raise Exception("Failed to download emcc")
            with zipfile.ZipFile(EmccZip, 'r') as zip_ref:
                zip_ref.extractall(Pd4Web.PD4WEB_ROOT)
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.Pd4Web.PD4WEB_ROOT + "/" + extractFolderName,
                    self.Pd4Web.PD4WEB_ROOT + "/emsdk",
                )
                os.remove(EmccZip)
            self.InstallEMCC()

    def InitVariables(self):
        self.EMSDK = self.Pd4Web.PD4WEB_ROOT + "/emsdk/emsdk"
        self.EMCMAKE = self.Pd4Web.PD4WEB_ROOT + "/emsdk/upstream/emscripten/emcmake"

    def InstallEMCC(self):
        if platform.system() == "Windows":
            os.system(f"cmd /C {self.EMSDK} install latest")
            os.system(f"cmd /C {self.EMSDK} activate latest")
        else:
            os.environ["EMSDK_QUIET"] = "1"
            os.system(f"chmod +x {self.EMSDK}")
            os.system(f"{self.EMSDK} install latest")
            os.system(f"{self.EMSDK} activate latest")

            # os.system(f"chmod +x {self.emcc.emsdk_env}")

        # if self.args.active_emcc:
        #     os.system(f"{self.emcc.emsdk} activate")
        #     sys.exit(0)
    def __str__(self):
        return "< EMCC >"

    def __repr__(self):
        return "< EMCC >"
