import os
import zipfile

import requests
import yaml
import json
import re

from .Pd4Web import Pd4Web
from .Helpers import DownloadZipFile, pd4web_print

# ╭──────────────────────────────────────╮
# │    In this file we have all code     │
# │ related to get the Libraries Source  │
# │          Code, Download it.          │
# ╰──────────────────────────────────────╯


class ExternalLibraries:
    def __init__(self, Pd4Web: Pd4Web) -> None:
        self.Pd4Web = Pd4Web
        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.GetSupportedLibraries()
        return

    def InitVariables(self):
        self.UsedObjects = []
        self.UsedLibraries = []
        self.totalOfLibraries = 0
        self.LibraryNames = []
        self.DownloadSources = {}
        self.SupportedLibraries = {}
        self.LibraryScriptDir = ""

    def GetSupportedLibraries(self):
        """
        It reads yaml file and get all supported libraries.
        """
        libFolder = self.Pd4Web.PD4WEB_LIBRARIES
        externalFile = os.path.join(libFolder, "Libraries.yaml")
        # check if file exists
        if not os.path.exists(externalFile):
            pass

        self.DynamicLibraries = []
        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.DownloadSources = supportedLibraries["Sources"]
            self.SupportedLibraries = supportedLibraries["Libraries"]
            self.LibraryNames = [lib["Name"] for lib in self.SupportedLibraries]
            self.totalOfLibraries = len(supportedLibraries)

    class LibraryClass:
        def __init__(self, LibraryData, DownloadSources) -> None:
            try:
                self.name = LibraryData["Name"]
            except:
                self.valid = False
                return

            self.dev = LibraryData["Developer"]
            self.repo = LibraryData["Repository"]
            self.folder = ""
            self.downloadSources = DownloadSources
            self.downloadLink = ""

            if "Source" in LibraryData:
                self.source = LibraryData["Source"]
            else:
                self.source = False
                if "DirectLink" in LibraryData:
                    self.directLink = LibraryData["DirectLink"]
                else:
                    raise Exception("Error: {self.name} doesn't have a download source")

            self.GetLinkForDownload()

            if "Dependencies" in LibraryData:
                self.dynamicLibraries = LibraryData["Dependencies"]
            else:
                self.dynamicLibraries = []

            if "Unsupported" in LibraryData:
                self.unsupported = LibraryData["Unsupported"]
            else:
                self.unsupported = []

            if "Version" in LibraryData:
                self.version = LibraryData["Version"]
            else:
                self.version = None

            self.usedObjs = []
            self.usedSourceFiles = []
            self.extraFuncExecuted = False
            self.unsupportedObjects = {}
            self.extraFlags = []
            self.valid = True

        def __str__(self):
            return f"< Library: {self.name} >"

        def __repr__(self):
            return self.__str__()

        def GetLinkForDownload(self):
            if self.source:
                if self.source in self.downloadSources:
                    self.downloadLink = self.downloadSources[self.source]
                    return self.downloadLink.format(self.dev, self.repo)
                else:
                    raise Exception(f"Error: {self.name} doesn't have a download source")
            else:
                return self.directLink

    def GetLibrary(self, libName) -> LibraryClass:
        lib = next((lib for lib in self.SupportedLibraries if lib["Name"] == libName), None)
        lib = self.LibraryClass(lib, self.DownloadSources)
        return lib

    def isSupportedLibrary(self, name):
        if name in self.LibraryNames:
            return True
        else:
            return False

    def CheckLibraryLink(self, url: str):
        try:
            response = requests.head(url, allow_redirects=True)
            if response.status_code == 200:
                return True
            else:
                return False
        except:
            return False

    def GetLibrarySourceCode(
        self,
        libName: str,
    ):
        if self.isSupportedLibrary(libName):
            if os.path.exists(os.path.join(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName)):
                return True
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                os.makedirs(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")

            # Library Download
            libData = self.GetLibrary(libName)
            if not os.path.exists(self.Pd4Web.APPDATA + "/Externals"):
                os.mkdir(self.Pd4Web.APPDATA + "/Externals")
            libPath = self.Pd4Web.APPDATA + f"/Externals/{libData.name}/"
            if not os.path.exists(libPath):
                os.mkdir(libPath)

            libZip = f"{self.Pd4Web.APPDATA}/Externals/{libData.name}/{libData.version}.zip"
            if not os.path.exists(libZip):
                pd4web_print(
                    f"Downloading {libData.name} {libData.version}...\n",
                    color="green",
                    silence=self.Pd4Web.SILENCE,
                )
                libSource = f"https://github.com/{libData.dev}/{libData.repo}/archive/refs/tags/{libData.version}.zip"
                if not self.CheckLibraryLink(libSource):
                    libSource = f"https://github.com/{libData.dev}/{libData.repo}/archive/{libData.version}.zip"
                    if not self.CheckLibraryLink(libSource):
                        raise Exception(
                            f"Error: Could not find good download link for {libData.name} version {libData.version}."
                        )
                DownloadZipFile(libSource, libZip)  # <== Download Library

            if not os.path.exists(self.PROJECT_ROOT + f"/Pd4Web/Externals/{libData.name}"):
                with zipfile.ZipFile(libZip, "r") as zip_ref:
                    zip_ref.extractall(self.PROJECT_ROOT + "/Pd4Web/Externals/")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(
                        self.PROJECT_ROOT + "/Pd4Web/Externals/" + extractFolderName,
                        self.PROJECT_ROOT + "/Pd4Web/Externals/" + libData.name,
                    )

            libFolder = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName
            self.Pd4Web.Objects.GetLibraryObjects(libFolder, libName)
            return True

    def AddUsedLibraries(self, LibraryName):
        self.UsedLibraries.append(LibraryName)

    def addToUsed(self, objName):
        self.UsedObjects.append(objName)

    def getUsedObjs(self):
        return self.UsedObjects

    def __repr__(self) -> str:
        return f"< Externals Libraries: {self.totalOfLibraries} >"

    def __str__(self) -> str:
        return self.__repr__()
