import os
import sys
import zipfile

import requests
import yaml

from .Pd4Web import Pd4Web

#╭──────────────────────────────────────╮
#│    In this file we have all code     │
#│ related to get the Libraries Source  │
#│          Code, Download it.          │
#╰──────────────────────────────────────╯

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
        externalFile = os.path.join(self.LibraryScriptDir, "Libraries.yaml")
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
                self.Name = LibraryData["Name"]
            except:
                self.valid = False
                return

            self.Developer = LibraryData["Developer"]
            self.Repository = LibraryData["Repository"]
            self.Folder = ""
            self.DownloadSources = DownloadSources
            self.DownloadLink = ""

            if "Source" in LibraryData:
                self.Source = LibraryData["Source"]
            else:
                self.Source = False
                if "DirectLink" in LibraryData:
                    self.DirectLink = LibraryData["DirectLink"]
                else:
                    raise Exception("Error: {self.name} doesn't have a download source")

            self.GetLinkForDownload()

            if "Dependencies" in LibraryData:
                self.DynamicLibraries = LibraryData["Dependencies"]
            else:
                self.DynamicLibraries = []

            if "Unsupported" in LibraryData:
                self.Unsupported = LibraryData["Unsupported"]
            else:
                self.Unsupported = []

            if "Version" in LibraryData:
                self.Version = LibraryData["Version"]
            else:
                self.Version = None

            self.usedObjs = []
            self.UsedSourceFiles = []
            self.extraFuncExecuted = False
            self.unsupportedObjects = {}
            self.extraFlags = []
            self.valid = True

        def __str__(self):
            return f"< Library: {self.Name} >"

        def __repr__(self):
            return self.__str__()

        def GetLinkForDownload(self):
            if self.Source:
                if self.Source in self.DownloadSources:
                    self.downloadLink = self.DownloadSources[self.Source]
                    return self.downloadLink.format(self.Developer, self.Repository)
                else:
                    raise Exception(f"Error: {self.Name} doesn't have a download source")
            else:
                return self.DirectLink


    def GetLibrary(self, LibraryName):
        Library = next((lib for lib in self.SupportedLibraries if lib['Name'] == LibraryName), None)
        Library = self.LibraryClass(Library, self.DownloadSources)
        return Library 



    def isSupportedLibrary(self, name):
        if name in self.LibraryNames:
            return True
        else:
            return False

    def AddUsedLibraries(self, LibraryName):
        self.UsedLibraries.append(LibraryName)

    # def add(self, PureDataExternals):
    #     self.PureDataExternals.append(PureDataExternals)
    #     self.LibraryNames.append(PureDataExternals.name)
    #     self.totalOfLibraries += 1

    def addToUsed(self, objName):
        self.UsedObjects.append(objName)

    def getUsedObjs(self):
        return self.UsedObjects

    def __repr__(self) -> str:
        return f"< Externals Libraries: {self.totalOfLibraries} >"

    def __str__(self) -> str:
        return self.__repr__()


