import os
import sys
import zipfile

import requests

from .Helpers import getPrintValue, pd4web_print
from .Libraries import ExternalLibraries
from .Patch import Patch, PatchLine


class GetCode():
    def __init__(self, ProcessedPatch: Patch, Libraries: ExternalLibraries):
        self.PROJECT_ROOT = ProcessedPatch.PROJECT_ROOT
        self.InitVariables()

        self.Libraries = Libraries
        for i in ProcessedPatch.PatchLinesProcessed:
            if i.isExternal:
                self.DownloadLibrarySourceCode(i)
            else:
                pass

    def InitVariables(self):
        self.LibraryScriptDir = os.path.dirname(os.path.realpath(__file__))

    def DownloadLink(self, PatchLine: PatchLine):
        LibraryClass = PatchLine.GetLibraryData()
        return LibraryClass.GetLinkForDownload()

    def DownloadLibrarySourceCode(self, PatchLine: PatchLine):
        LibraryName = PatchLine.Library
        if self.Libraries.isSupportedLibrary(LibraryName):
            if os.path.exists(os.path.join(self.LibraryScriptDir + "/Externals/" + LibraryName)):
                return True

            # check if self.Libraries.SupportedLibraries/ExternalLibraries exist
            if not os.path.exists(self.LibraryScriptDir + "/Externals"):
                os.makedirs(self.LibraryScriptDir + "/Externals")

            LinkForSourceCode = self.DownloadLink(PatchLine)
            response = requests.get(LinkForSourceCode)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            try:
                response = requests.get(sourceCodeLink, stream=True)
                if response.status_code != 200:
                    raise Exception(f"Error: {response.status_code}")
                total_size = int(response.headers.get('content-length', 0))
                chunk_size = 1024
                num_bars = 40
                pd4web_print(f"Downloading {LibraryName}...", color="yellow")
                with open(self.LibraryScriptDir + "/Externals/" + LibraryName + ".zip", 'wb') as file:
                    downloaded_size = 0
                    for data in response.iter_content(chunk_size):
                        file.write(data)
                        downloaded_size += len(data)
                        try:
                            progress = downloaded_size / total_size
                            num_hashes = int(progress * num_bars)
                            progress_bar = '#' * num_hashes + '-' * (num_bars - num_hashes)
                            
                            # Print progress bar
                            sys.stdout.write(f'\r    🟡 |{progress_bar}| {progress:.2%}')
                            sys.stdout.flush()
                        except ZeroDivisionError:
                            pass
                print()
            except Exception as e:
                raise Exception(f"Error: {e}")

            with zipfile.ZipFile(self.LibraryScriptDir + "/Externals/" + LibraryName + ".zip", "r") as zip_ref:
                zip_ref.extractall(self.LibraryScriptDir + "/Externals")
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.LibraryScriptDir + "/Externals/" + extractFolderName,
                    self.LibraryScriptDir + "/Externals/" + LibraryName,
                )
            LibraryFolder = self.LibraryScriptDir + "/Externals/" + LibraryName
            os.remove(self.LibraryScriptDir + "/Externals/" + LibraryName + ".zip")
            PatchLine.GetLibraryExternals(LibraryFolder, LibraryName)
            return True


