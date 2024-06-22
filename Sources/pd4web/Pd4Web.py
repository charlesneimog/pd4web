import os
import subprocess
import sys

import requests

from .Helpers import pd4web_print


class Pd4Web():
    def __init__(self, patch, Recursive=False):
        from .Compiler import Compiler
        from .GetCode import GetCode
        from .Libraries import ExternalLibraries
        from .Patch import Patch

        self.CheckDependencies()


        # Sobre a recursivade para patch, talvez não chamar o construtor de Pd4Web, 
        # mas some a mesma ordem para __init__.
        self.Patch = patch
        self.InitVariables()

        self.Compiler = Compiler(self)
        self.Libraries = ExternalLibraries(self)
        self.ProcessedPatch = Patch(self)
        self.CodeRetrieved = GetCode(self)

    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
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
        self.ExternalsLinkLibraries = []

    def CheckDependencies(self):
        # check if Git is installed
        try:
            # Use subprocess to run 'git --version' command
            subprocess.check_output(['git', '--version'])
            return True
        except subprocess.CalledProcessError:
            # Git is not installed or not found in PATH
            return False
        


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

    
