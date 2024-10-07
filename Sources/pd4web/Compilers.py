import os
import platform
import cmake
import ninja
import subprocess
import tarfile

import pygit2

from .Helpers import pd4web_print
from .Pd4Web import Pd4Web


class ExternalsCompiler:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.InitVariables()

        if not os.path.exists(Pd4Web.APPDATA + "/emsdk"):
            pd4web_print("Cloning emsdk", color="yellow", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL)
            emsdk_path = Pd4Web.APPDATA + "/emsdk"
            emsdk_git = "https://github.com/emscripten-core/emsdk"
            ok = pygit2.clone_repository(emsdk_git, emsdk_path)
            if not ok:
                raise Exception("Failed to clone emsdk")
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
        self.NINJA = ninja.BIN_DIR + "/ninja"
        if platform.system() == "Windows":
            self.NINJA += ".exe"
            self.EMCC += ".bat"

    def GetCmake(self):
        cmake_dir = cmake.CMAKE_BIN_DIR
        cmake_bin = os.path.join(cmake_dir, "cmake")
        if platform.system() == "Windows":
            cmake_bin += ".exe"
        if not os.path.exists(cmake_bin):
            raise Exception("Cmake (module) is not installed. Please install it.")
        return cmake_bin

    def InstallEMCC(self):
        pd4web_print("Installing emcc", color="yellow", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL)
        if platform.system() == "Windows":
            os.system(f"cmd /C {self.EMSDK} install latest")
            os.system(f"cmd /C {self.EMSDK} activate latest")
            return

        elif platform.system() == "Linux":
            pass

        os.environ["EMSDK_QUIET"] = "1"
        result = subprocess.run(["chmod", "+x", self.EMSDK], capture_output=not self.Pd4Web.verbose, text=True)
        if result.returncode != 0:
            raise Exception("Failed to make emsdk executable")

        # ──────────────────────────────────────
        result = subprocess.run(
            [self.EMSDK, "install", "latest"],
            capture_output=not self.Pd4Web.verbose,
            text=True,
        )
        if result.returncode != 0:
            raise Exception(f"Failed to install emsdk, result {result.returncode}, cmd: " + " ".join(result.args))

        # ──────────────────────────────────────
        result = subprocess.run(
            [self.EMSDK, "activate", self.Pd4Web.EMSDK_VERSION], capture_output=not self.Pd4Web.verbose, text=True
        )
        if result.returncode != 0:
            raise Exception(f"Failed to install emsdk, result {result.returncode}, cmd: " + " ".join(result.args))

    def __str__(self):
        return "< Compiler >"

    def __repr__(self):
        return "< Compiler >"
