import os
import subprocess
import sys

import requests

from .Helpers import pd4web_print


class Pd4Web():
    OUTCHS_COUNT: int = 0
    INCHS_COUNT: int = 0

    def __init__(self, patch):
        from .Builder import GetAndBuildExternals
        from .Compilers import ExternalsCompiler
        from .Libraries import ExternalLibraries
        from .Patch import Patch

        self.CheckDependencies() # git and cmake

        #╭──────────────────────────────────────╮
        #│    NOTE: Sobre a recursivade para    │
        #│      patch, talvez não chamar o      │
        #│        construtor de Pd4Web,         │
        #│mas some a mesma ordem para __init__. │
        #╰──────────────────────────────────────╯

        self.Patch = patch

        # ───────────── Init Classes ─────────────
        self.InitVariables()

        self.Compiler = ExternalsCompiler(self)
        self.Libraries = ExternalLibraries(self)

        # ──────────── Process Patch ──────────
        self.ProcessedPatch = Patch(self) # Recursively in case of Abstraction 

        # ──────────── Build Externals ──────────
        self.ExternalsBuilder = GetAndBuildExternals(self) 

        # TODO: Build main app
        # TODO: Build webpatch folder



    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)
        IsRepo = os.path.isdir(os.path.join(self.PROJECT_ROOT, '.git'))
        if not IsRepo:
            os.system(f"cd {self.PROJECT_ROOT} && git init --initial-branch=main")
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        self.CWD = os.getcwd()

        # Core Numbers
        self.Cores = os.cpu_count()

        # Used Objects
        self.UsedObjects = []

        # Compiler and Code Variables
        self.UiReceiversSymbol = []
        self.ExternalsSourceCode = []
        self.ExternalsExtraFlags = []

        # Externals Objects
        self.ExternalsLinkLibraries = []
        self.ExternalsLinkLibrariesFolders = []
        self.ExternalsSetupFunctions = []
        self.Verbose = False

    def CheckDependencies(self):
        try:
            subprocess.check_output(['git', '--version'])
        except subprocess.CalledProcessError:
            raise Exception("Git is not installed. Please install it.")
        OK = os.system("cmake --version > /dev/null")
        if OK != 0:
            raise Exception("\n\nCmake is not installed. Please install it.")

    def DownloadZip(self, url, filename, what=""):
        pd4web_print(f"Downloading {what}...", color="green")
        response = requests.get(url, stream=True)
        if response.status_code != 200:
            raise Exception(f"Error: {response.status_code}")
        total_size = response.headers.get('content-length')
        total_size = int(total_size) if total_size is not None else None
        chunk_size = 1024
        num_bars = 40
        with open(filename, 'wb') as file:
            downloaded_size = 0
            for data in response.iter_content(chunk_size):
                file.write(data)
                downloaded_size += len(data)
                if total_size:
                    progress = downloaded_size / total_size
                    num_hashes = int(progress * num_bars)
                    progress_bar = '#' * num_hashes + '-' * (num_bars - num_hashes)
                    sys.stdout.write(f'\r    🟢 |{progress_bar}| {progress:.2%}')
                else:
                    num_hashes = int(downloaded_size / chunk_size) % num_bars
                    progress_bar = '#' * num_hashes + '-' * (num_bars - num_hashes)
                    sys.stdout.write(f'\r    🟢 |{progress_bar}| {downloaded_size} bytes')
                sys.stdout.flush()
        print()
        return True

    
