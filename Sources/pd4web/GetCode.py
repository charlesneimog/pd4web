import os
import re
import shutil
import sys
import zipfile

import requests

from .Helpers import getPrintValue, pd4web_print
from .Patch import PatchLine
from .Super import Pd4Web


class GetCode():
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web

        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.InitVariables()

        self.Patch = Pd4Web.ProcessedPatch
        self.Libraries = Pd4Web.Libraries
        self.GetExternalsSourceCode()

        #╭──────────────────────────────────────╮
        #│  Here, we already have most part of  │
        #│           the source code.           │
        #│  Now we need to check if we need to  │
        #│run extra functions for some objects. │
        #│  These extra function include extra  │
        #│    .c|.cpp files, includes, also     │
        #│          extraflags, etc...          │
        #╰──────────────────────────────────────╯

        self.CheckIfNeedExtraFunctions()

    def InitVariables(self):
        self.ExternalsSourceCode = {}
        self.ExternalsExtraFlags = {}
        self.ExternalsDynamicLibraries = {}

    def DownloadLink(self, PatchLine: PatchLine):
        LibraryClass = PatchLine.GetLibraryData()
        return LibraryClass.GetLinkForDownload()

    def DownloadLibrarySourceCode(self, PatchLine: PatchLine):
        LibraryName = PatchLine.Library
        if self.Libraries.isSupportedLibrary(LibraryName):
            if os.path.exists(os.path.join(self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName)):
                return True
            if not os.path.exists(self.Pd4Web.PD4WEB_ROOT + "/Externals"):
                os.makedirs(self.Pd4Web.PD4WEB_ROOT + "/Externals")

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
                with open(self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName + ".zip", 'wb') as file:
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

            with zipfile.ZipFile(self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName + ".zip", "r") as zip_ref:
                zip_ref.extractall(self.Pd4Web.PD4WEB_ROOT + "/Externals")
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.Pd4Web.PD4WEB_ROOT + "/Externals/" + extractFolderName,
                    self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName,
                )
            LibraryFolder = self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName
            os.remove(self.Pd4Web.PD4WEB_ROOT + "/Externals/" + LibraryName + ".zip")
            PatchLine.GetLibraryExternals(LibraryFolder, LibraryName)
            return True

    def RegexSearch(self, PatchLine: PatchLine, functionName, file):
        """
        This search for the setup function using regex.
        """
        with open(
            file,
            "r",
            encoding="utf-8",
            errors="ignore",
        ) as C_file:
            try:
                file_contents = C_file.read()
            except:
                raise Exception(f"Could not read file: {file} using utf-8")

            patterns = [r"void\s*{}\s*\(\s*void\s*\)", r"void\s+{}\s*\(\s*\)"]
            for pattern in patterns:
                pattern = pattern.format(re.escape(functionName))
                matches = re.finditer(pattern, file_contents, re.DOTALL)
                listMatches = list(matches)
                if len(listMatches) > 0:
                    ExternalsPath = os.path.join(self.PROJECT_ROOT, "webpatch/externals")
                    if not os.path.exists(ExternalsPath):
                        os.makedirs(ExternalsPath)
                    shutil.copy(C_file.name, ExternalsPath)
                    PatchLine.objFound = True
                    PatchLine.functionName = functionName
                    if PatchLine.Library not in self.ExternalsSourceCode:
                        self.ExternalsSourceCode[PatchLine.Library] = [C_file.name]
                    else:
                        self.ExternalsSourceCode[PatchLine.Library].append(C_file.name)

    def SearchCFunction(self, lineInfo, root, file):
        """
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        """
        functionName = lineInfo.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        # NOTE: Maybe there is another cases like this
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e")  # else use . as 0x2e
        self.RegexSearch(lineInfo, functionName, os.path.join(root, file))
        if not lineInfo.objFound:
            functionName = lineInfo.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.RegexSearch(lineInfo, functionName, os.path.join(root, file))

    def CopyLibAbstraction(self, abstractionfile):
        """
        This function copies the abstractions to webpatch/data folder.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")

        if os.path.exists(self.PROJECT_ROOT + "webpatch/data/" + os.path.basename(abstractionfile)):
            return

        shutil.copy(abstractionfile, self.PROJECT_ROOT + "webpatch/data")


    def GetExternalsSourceCode(self):
        for PatchLine in self.Patch.PatchLinesProcessed:
            if PatchLine.isExternal:
                foundLibrary = self.DownloadLibrarySourceCode(PatchLine)
                if foundLibrary:
                    for root, _, files in os.walk(self.Pd4Web.PD4WEB_ROOT + "/Externals/" + PatchLine.Library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.SearchCFunction(PatchLine, root, file)
                            elif file.endswith(".pd"):
                                if PatchLine.name == file.split(".pd")[0]:
                                    PatchLine.isAbstraction = True
                                    PatchLine.objFound = True
                                    self.CopyLibAbstraction(os.path.join(root, file))
                else:
                    raise Exception(f"Error: Could not find {PatchLine.Library} in the supported libraries")

                if PatchLine.objFound and PatchLine.isAbstraction:
                    externalSpace = 7 - len(PatchLine.name)
                    absName = PatchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found Abstraction: {absName}  | Lib: {PatchLine.Library}",
                        color="green",
                    )

                elif PatchLine.objFound and not PatchLine.isAbstraction:
                    externalSpace = 10 - len(PatchLine.name)
                    objName = PatchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found External: {objName}  | Lib: {PatchLine.Library}",
                        color="green",
                    )
                else:
                    raise Exception("Could not find " + PatchLine.name)

    def CheckIfNeedExtraFunctions(self):
        for UsedObjects in self.Pd4Web.UsedObjects:
            LibUsedObjects = [item["Obj"] for item in self.Pd4Web.UsedObjects if item['Lib'] == UsedObjects['Lib']]
            # Execute extra function


            # set that the extra function was executed

    def __repr__(self) -> str:
        return f"< GetCode Object >"

    def __str__(self) -> str:
        return self.__repr__()

