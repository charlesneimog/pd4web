import os
import platform
import cmake
import ninja
import subprocess

from .Pd4Web import Pd4Web


class ExternalsCompiler:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.init_vars()
        if not os.path.exists(self.EMCMAKE):
            self.install_emcc()

    def init_vars(self):
        self.EMSDK = self.Pd4Web.APPDATA + "/emsdk/emsdk"
        self.EMSDK_PY = self.Pd4Web.APPDATA + "/emsdk/emsdk.py"
        self.EMCMAKE = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emcmake"
        self.CMAKE = self.get_cmake()
        self.EMCC = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emcc"
        self.NINJA = ninja.BIN_DIR + "/ninja"
        self.CONFIGURE = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emconfigure"
        self.MAKE = self.Pd4Web.APPDATA + "/emsdk/upstream/emscripten/emmake"
        if platform.system() == "Windows":
            self.NINJA += ".exe"
            self.EMCC += ".bat"
            self.CONFIGURE += ".bat"
            self.EMCMAKE += ".bat"
            self.MAKE += ".bat"

    def get_cmake(self):
        cmake_dir = cmake.CMAKE_BIN_DIR
        cmake_bin = os.path.join(cmake_dir, "cmake")
        if platform.system() == "Windows":
            cmake_bin += ".exe"
        if not os.path.exists(cmake_bin):
            self.Pd4Web.exception("Cmake (module) is not installed. Please install it.")
        return cmake_bin

    def install_emcc(self):
        if platform.system() == "Windows":
            command = f"{self.EMSDK}.bat install latest"
            self.Pd4Web.print(
                "Installing emsdk on Windows",
                color="yellow",
                silence=self.Pd4Web.SILENCE,
                pd4web=self.Pd4Web.PD_EXTERNAL,
            )
            result = subprocess.run(command, shell=True, env=self.Pd4Web.env).returncode
            if result != 0:
                self.Pd4Web.exception("Failed to install emsdk")
            command = f"{self.EMSDK}.bat activate latest"
            self.Pd4Web.print(
                "Activating emsdk on Windows",
                color="yellow",
                silence=self.Pd4Web.SILENCE,
                pd4web=self.Pd4Web.PD_EXTERNAL,
            )
            result = subprocess.run(command, shell=True, env=self.Pd4Web.env).returncode
            if result != 0:
                self.Pd4Web.exception("Failed to activate emsdk")

        else:
            self.Pd4Web.print(
                "Installing emsdk", color="yellow", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL
            )
            result = subprocess.run(
                ["chmod", "+x", self.EMSDK], env=self.Pd4Web.env, capture_output=not self.Pd4Web.verbose, text=True
            )

            if result.returncode != 0:
                self.Pd4Web.exception("Failed to make emsdk executable")
            else:
                self.Pd4Web.print(
                    "emsdk is now executable",
                    color="green",
                    silence=self.Pd4Web.SILENCE,
                    pd4web=self.Pd4Web.PD_EXTERNAL,
                )

            self.Pd4Web.print(
                "Installing emsdk, this will take a about 5 minutes, wait!!!",
                color="yellow",
                silence=self.Pd4Web.SILENCE,
                pd4web=self.Pd4Web.PD_EXTERNAL,
            )
            # ──────────────────────────────────────
            result = subprocess.run(
                [self.EMSDK, "install", "latest"],
                capture_output=not self.Pd4Web.verbose,
                env=self.Pd4Web.env,
                text=True,
            )
            if result.returncode != 0:
                self.Pd4Web.exception(
                    f"Failed to install emsdk, result {result.returncode}, cmd: " + " ".join(result.args)
                )
            else:
                self.Pd4Web.print(
                    "emsdk installed", color="green", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL
                )

            # ──────────────────────────────────────
            result = subprocess.run(
                [self.EMSDK, "update"],
                env=self.Pd4Web.env,
                capture_output=not self.Pd4Web.verbose,
                text=True,
            )

            result = subprocess.run(
                [self.EMSDK, "activate", self.Pd4Web.EMSDK_VERSION],
                env=self.Pd4Web.env,
                capture_output=not self.Pd4Web.verbose,
                text=True,
            )

            if result.returncode != 0:
                self.Pd4Web.exception(
                    f"Failed to install emsdk, result {result.returncode}, cmd: " + " ".join(result.args)
                )

    def __str__(self):
        return "< Compiler >"

    def __repr__(self):
        return "< Compiler >"
