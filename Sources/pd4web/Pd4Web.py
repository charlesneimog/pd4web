import argparse
import os
import sys
import zipfile
import shutil

import requests

from .Helpers import DownloadZipFile, pd4web_print


class Pd4Web:
    OUTCHS_COUNT: int = 0
    INCHS_COUNT: int = 0
    MEMORY_SIZE: int = 128
    GUI: bool = True
    PD_VERSION: str = "0.55-0"
    SILENCE: bool = False

    def __init__(self, Patch=""):
        self.Patch = Patch
        self.InitVariables()

    def argParse(self):
        parser = argparse.ArgumentParser(
            description="Compile Pure Data externals for the web.",
            usage="pd4web.py <PureData Patch>",
        )
        parser.add_argument("patch_file", type=str,
                            help="The patch file to be compiled")
        parser.add_argument("-v", "--verbose",
                            action="store_true", help="Enable verbose mode")
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
        self.Parser = parser.parse_args()
        # where is the patch file

        # get complete path of the patch file
        completePath = os.path.abspath(self.Parser.patch_file)
        self.Patch = completePath
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)

        # check if file exists
        if not os.path.isfile(self.Patch):
            raise Exception("\n\nError: Patch file not found")

        self.verbose = self.Parser.verbose
        self.MEMORY_SIZE = self.Parser.initial_memory
        self.GUI = not self.Parser.nogui
        self.PD_VERSION = self.Parser.pd_version
        self.Execute()

    def Execute(self):
        from .Builder import GetAndBuildExternals
        from .Compilers import ExternalsCompiler
        from .Libraries import ExternalLibraries
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
        self.Libraries = ExternalLibraries(self)

        # ──────────── Process Patch ──────────
        self.ProcessedPatch = Patch(self)  # Recursively in case of Abstraction

        # ──────────── Build Externals ──────────
        self.ExternalsBuilder = GetAndBuildExternals(self)

        # Extra Configs

    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        self.CWD = os.getcwd()
        if sys.platform == "win32":
            self.APPDATA = os.path.join(os.getenv("APPDATA"), "pd4web")
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(
                os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(
                os.path.expanduser("~/.local/share"), "pd4web")
        else:
            raise RuntimeError("Unsupported platform")

        if not os.path.exists(self.APPDATA):
            os.makedirs(self.APPDATA)

        isRepo = os.path.isdir(os.path.join(self.PROJECT_ROOT, ".git"))
        if not isRepo:
            os.system(
                f"cd {self.PROJECT_ROOT} && git init --initial-branch=main")

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
        self.verbose = False

    def CheckDependencies(self):
        OK = shutil.which("git")
        if OK is None:
            raise Exception("Git is not installed. Please install it.")
        OK = shutil.which("cmake")
        if OK is None:
            raise Exception("Cmake is not installed. Please install it.")

    def DownloadZip(self, url, filename, what=""):
        pd4web_print(f"Downloading {what}...",
                     color="green", silence=self.SILENCE)
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
                    progress_bar = "#" * num_hashes + \
                        "-" * (num_bars - num_hashes)
                    sys.stdout.write(
                        f"\r    🟢 |{progress_bar}| {progress:.2%}")
                else:
                    num_hashes = int(downloaded_size / chunk_size) % num_bars
                    progress_bar = "#" * num_hashes + \
                        "-" * (num_bars - num_hashes)
                    sys.stdout.write(
                        f"\r    🟢 |{progress_bar}| {downloaded_size} bytes")
                sys.stdout.flush()
        print()
        return True

    def GetPdSourceCode(self):
        if not os.path.exists(self.APPDATA + "/Pd"):
            os.mkdir(self.APPDATA + "/Pd")

        if not os.path.exists(self.APPDATA + f"/Pd/{self.PD_VERSION}.zip"):
            pd4web_print(
                f"Downloading Pure Data {self.PD_VERSION}...", color="green", silence=self.SILENCE)
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
