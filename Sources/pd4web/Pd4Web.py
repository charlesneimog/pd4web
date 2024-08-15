import argparse
import os
import sys
import zipfile
import pygit2
import cmake
import shutil

import requests

from .Helpers import DownloadZipFile, pd4web_print


class Pd4Web:
    # Paths
    PD4WEB_LIBRARIES: str = ""
    PD_EXTERNAL: bool = False
    # Dev
    BYPASS_UNSUPPORTED: bool = False
    SILENCE: bool = False
    PD_VERSION: str = "0.55-0"

    # Compiler
    MEMORY_SIZE: int = 512

    # Audio
    OUTCHS_COUNT: int = 0
    INCHS_COUNT: int = 0

    # Gui
    FPS: int = 60
    GUI: bool = True
    AUTO_THEME: bool = True

    # Midi
    MIDI: bool = False

    def __init__(self, Patch=""):
        self.Patch = Patch
        self.verbose = False
        # self.InitVariables()

    def argParse(self):
        print()
        parser = argparse.ArgumentParser(
            description="Compile Pure Data externals for the web.",
            usage="pd4web.py <PureData Patch>",
        )
        parser.add_argument("patch_file", type=str, help="The patch file to be compiled")
        parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode")
        parser.add_argument(
            "-m",
            "--initial-memory",
            required=False,
            default=128,
            type=int,
            help="Initial memory size in MB",
        )
        parser.add_argument(
            "-nogui",
            "--nogui",
            required=False,
            action="store_true",
            default=False,
            help="If set to False, it will not load the GUI interface.",
        )
        parser.add_argument(
            "--pd-version",
            required=False,
            default="0.55-0",
            type=str,
            help="Pure Data version to use",
        )
        parser.add_argument(
            "--run-browser",
            required=False,
            action="store_true",
            default=False,
            help="Use Emscripten to run the Browser",
        )
        parser.add_argument(
            "--clear",
            required=False,
            action="store_true",
            default=False,
            help="Clear the cache before running",
        )

        # Dev Flags
        parser.add_argument(
            "--bypass-unsupported",
            required=False,
            default=False,
            action="store_true",
            help="Bypass unsupported objects in libraries",
        )
        self.Parser = parser.parse_args()

        # get complete path of the patch file
        completePath = os.path.abspath(self.Parser.patch_file)
        self.Patch = completePath
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)

        # check if file exists
        if not os.path.isfile(self.Patch):
            raise Exception("\n\nError: Patch file not found")

        if self.Parser.run_browser:
            self.RunBrowser()
            return

        self.verbose = self.Parser.verbose
        self.MEMORY_SIZE = self.Parser.initial_memory
        self.GUI = not self.Parser.nogui
        self.PD_VERSION = self.Parser.pd_version
        self.BYPASS_UNSUPPORTED = self.Parser.bypass_unsupported

        if self.Parser.clear:
            shutil.rmtree(os.path.join(self.PROJECT_ROOT, "build"), ignore_errors=True)
            shutil.rmtree(os.path.join(self.PROJECT_ROOT, "Pd4Web"), ignore_errors=True)
            shutil.rmtree(os.path.join(self.PROJECT_ROOT, "WebPatch"), ignore_errors=True)

        self.Execute()

    def Execute(self):
        from .Builder import GetAndBuildExternals
        from .Compilers import ExternalsCompiler
        from .Patch import Patch

        if self.Patch == "":
            raise Exception("You must set a patch file")

        self.InitVariables()
        self.CheckDependencies()  # git and cmake

        # ╭──────────────────────────────────────╮
        # │    NOTE: Sobre a recursivade para    │
        # │      patch, talvez não chamar o      │
        # │        construtor de Pd4Web,         │
        # │mas some a mesma ordem para __init__. │
        # ╰──────────────────────────────────────╯

        # ───────────── Init Classes ─────────────
        self.GetPdSourceCode()

        self.Compiler = ExternalsCompiler(self)

        # ──────────── Process Patch ──────────
        self.ProcessedPatch: Patch = Patch(self)  # Recursively in case of Abstraction

        # ──────────── Build Externals ──────────
        self.ExternalsBuilder = GetAndBuildExternals(self)

        # Extra Configs

    def RunBrowser(self):
        self.CWD = os.getcwd()
        if sys.platform == "win32":
            self.APPDATA = os.path.join(os.getenv("APPDATA"), "pd4web")
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(os.path.expanduser("~/.local/share"), "pd4web")
        else:
            raise RuntimeError("Unsupported platform")
        emccRun = self.APPDATA + "/emsdk/upstream/emscripten/emrun"
        # check if emrun exists
        if not os.path.exists(emccRun):
            raise Exception("emrun not found. Please install Emscripten")

        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch)) + "/index.html"
        if not os.path.exists(self.PROJECT_ROOT):
            raise Exception("index.html not found, had you compiled the patch?")
        os.chdir(os.path.dirname(self.PROJECT_ROOT))
        os.system(f"{emccRun} index.html")

    def InitVariables(self):
        from .Objects import Objects
        from .Libraries import ExternalLibraries

        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        if self.PD4WEB_LIBRARIES == "":
            self.PD4WEB_LIBRARIES = os.path.join(self.PD4WEB_ROOT, "../Libraries")

        self.CWD = os.getcwd()
        if sys.platform == "win32":
            self.APPDATA = os.path.join(os.getenv("APPDATA"), "pd4web")
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(os.path.expanduser("~/.local/share"), "pd4web")
        else:
            raise RuntimeError("Unsupported platform")

        if not os.path.exists(self.APPDATA):
            os.makedirs(self.APPDATA)

        try:
            self.PROJECT_GIT = pygit2.Repository(self.PROJECT_ROOT)
        except pygit2.GitError:
            self.PROJECT_GIT = pygit2.init_repository(self.PROJECT_ROOT, bare=False)

        # Core Numbers
        self.cpuCores = os.cpu_count()

        # Used Objects
        self.usedObjects = []
        self.patchLinesProcessed = []

        # Compiler and Code Variables
        self.uiReceiversSymbol = []
        self.externalsSourceCode = []

        # Externals Objects
        self.externalsLinkLibraries = []
        self.externalsLinkLibrariesFolders = []
        self.externalsSetupFunctions = []

        # Classes
        self.Libraries = ExternalLibraries(self)
        self.Objects: Objects = Objects(self)

    def CheckDependencies(self):
        cmake_dir = cmake.__file__
        cmake_dir = os.path.dirname(cmake_dir)
        cmake_dir = os.path.join(cmake_dir, "data", "bin")
        cmake_bin = os.path.join(cmake_dir, "cmake")
        cmake_bin = os.path.abspath(cmake_bin)
        if not os.path.exists(cmake_bin):
            raise Exception("Cmake (module) is not installed. Please install it.")

    def DownloadZip(self, url, filename, what=""):
        pd4web_print(f"Downloading {what}...", color="green", silence=self.SILENCE)
        response = requests.get(url, stream=True)
        if response.status_code != 200:
            raise Exception(f"Error: {response.status_code}")
        total_size = response.headers.get("content-length")
        total_size = int(total_size) if total_size is not None else None
        chunk_size = 1024
        num_bars = 40
        with open(filename, "wb") as file:
            downloaded_size = 0
            for data in response.iter_content(chunk_size):
                file.write(data)
                downloaded_size += len(data)
                if total_size:
                    progress = downloaded_size / total_size
                    num_hashes = int(progress * num_bars)
                    progress_bar = "#" * num_hashes + "-" * (num_bars - num_hashes)
                    sys.stdout.write(f"\r    🟢 |{progress_bar}| {progress:.2%}")
                else:
                    num_hashes = int(downloaded_size / chunk_size) % num_bars
                    progress_bar = "#" * num_hashes + "-" * (num_bars - num_hashes)
                    sys.stdout.write(f"\r    🟢 |{progress_bar}| {downloaded_size} bytes")
                sys.stdout.flush()
        print()
        return True

    def GetPdSourceCode(self):
        if not os.path.exists(self.APPDATA + "/Pd"):
            os.mkdir(self.APPDATA + "/Pd")

        if not os.path.exists(self.APPDATA + f"/Pd/{self.PD_VERSION}.zip"):
            pd4web_print(
                f"Downloading Pure Data {self.PD_VERSION}...",
                color="green",
                silence=self.SILENCE,
            )
            pdVersionSource = f"https://github.com/pure-data/pure-data/archive/refs/tags/{self.PD_VERSION}.zip"
            pdVersionZip = self.APPDATA + f"/Pd/{self.PD_VERSION}.zip"
            DownloadZipFile(pdVersionSource, pdVersionZip)
        else:
            pdVersionZip = self.APPDATA + f"/Pd/{self.PD_VERSION}.zip"

        # Zip file to self.Pd4Web.PROJECT_ROOT /Pd4Web/pure-data
        if not os.path.exists(self.PROJECT_ROOT + "/Pd4Web/"):
            os.mkdir(self.PROJECT_ROOT + "/Pd4Web/")

        if not os.path.exists(self.PROJECT_ROOT + "/Pd4Web/pure-data"):
            with zipfile.ZipFile(pdVersionZip, "r") as zip_ref:
                zip_ref.extractall(self.PROJECT_ROOT + "/Pd4Web/")
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.PROJECT_ROOT + "/Pd4Web/" + extractFolderName,
                    self.PROJECT_ROOT + "/Pd4Web/pure-data",
                )

    def Silence(self):
        self.SILENCE = True
