import os
import sys
import zipfile

import requests
import yaml

from .Super import Pd4Web

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
            self.DownloadSources = supportedLibraries["DownloadSources"]
            self.SupportedLibraries = supportedLibraries["SupportedLibraries"]
            self.LibraryNames = [lib["name"] for lib in self.SupportedLibraries]
            self.totalOfLibraries = len(supportedLibraries)

    class LibraryClass:
        def __init__(self, LibraryData, DownloadSources) -> None:
            self.name = LibraryData["name"]
            self.repoUser = LibraryData["repoUser"]
            self.repoName = LibraryData["repoName"]
            self.folder = ""
            self.externalsExtraFunctions = []
            self.DownloadSources = DownloadSources
            self.DownloadLink = ""

            if "downloadSrc" in LibraryData:
                self.repoAPI = LibraryData["downloadSrc"]
            else:
                self.repoAPI = False
                if "directLink" in LibraryData:
                    self.downloadSrc = LibraryData["directLink"]
                else:
                    raise Exception("Error: {self.name} doesn't have a download source")

            self.GetLinkForDownload() # PAREI_AQUI: Implementar construtor do link completo

            if "extraFunction" in LibraryData:
                self.extraFunc = LibraryData["extraFunction"]
            else:
                self.extraFunc = None
                   
            if "singleObject" in LibraryData:
                self.singleObject = LibraryData["singleObject"]
            else:
                self.singleObject = False

            if "dynamicLibraries" in LibraryData:
                self.requireDynamicLibraries = LibraryData["dynamicLibraries"]
            else:
                self.requireDynamicLibraries = False

            if "unsupportedObj" in LibraryData:
                self.unsupportedObj = LibraryData["unsupportedObj"]
            else:
                self.unsupportedObj = []

            self.usedObjs = []
            self.UsedSourceFiles = []
            self.extraFuncExecuted = False
            self.unsupportedObjects = {}
            self.extraFlags = []

        def __str__(self):
            return f"< Library: {self.name} >"

        def __repr__(self):
            return self.__str__()



        def GetLinkForDownload(self):
            if self.repoAPI:
                if self.repoAPI in self.DownloadSources:
                    self.downloadLink = self.DownloadSources[self.repoAPI]
                    return self.downloadLink.format(self.repoUser, self.repoName)
                else:
                    raise Exception(f"Error: {self.name} doesn't have a download source")
            else:
                return self.downloadSrc


    def GetLibrary(self, LibraryName):
        Library = next((lib for lib in self.SupportedLibraries if lib['name'] == LibraryName), None)
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


