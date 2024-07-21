import os
import sys
import zipfile

import requests
import yaml

from .Pd4Web import Pd4Web

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
        self.LibraryScriptDir = os.path.dirname(os.path.realpath(__file__))
        externalFile = os.path.join(
            self.LibraryScriptDir, "../Libraries/Libraries.yaml")
        self.DynamicLibraries = []
        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.DownloadSources = supportedLibraries["Sources"]
            self.SupportedLibraries = supportedLibraries["Libraries"]
            self.LibraryNames = [lib["Name"]
                                 for lib in self.SupportedLibraries]
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
                    raise Exception(
                        "Error: {self.name} doesn't have a download source")

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
                    raise Exception(
                        f"Error: {self.name} doesn't have a download source")
            else:
                return self.directLink

    def GetLibrary(self, LibraryName):
        Library = next(
            (lib for lib in self.SupportedLibraries if lib["Name"] == LibraryName), None)
        Library = self.LibraryClass(Library, self.DownloadSources)
        return Library

    def isSupportedLibrary(self, name):
        if name in self.LibraryNames:
            return True
        else:
            return False

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
