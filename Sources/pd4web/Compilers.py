import os
import platform
import cmake

import pygit2

from .Helpers import pd4web_print
from .Pd4Web import Pd4Web


class ExternalsCompiler:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.InitVariables()
        if not os.path.exists(Pd4Web.APPDATA + "/emsdk"):
            pd4web_print("Cloning emsdk", color="yellow")
            emsdk_path = Pd4Web.APPDATA + "/emsdk"
            emsdk_git = "https://github.com/emscripten-core/emsdk"
            pygit2.clone_repository(emsdk_git, emsdk_path)
            libRepo: pygit2.Repository = pygit2.Repository(emsdk_path)
            tag_name = Pd4Web.EMSDK_VERSION

            # commit
            tag_ref = libRepo.references.get(f"refs/tags/{tag_name}")
            if isinstance(tag_ref.peel(), pygit2.Tag):
                commit = tag_ref.peel().target
            else:
                commit = tag_ref.peel()
            libRepo.set_head(commit.id)
            libRepo.checkout_tree(commit)
            libRepo.reset(commit.id, pygit2.GIT_RESET_HARD)
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
