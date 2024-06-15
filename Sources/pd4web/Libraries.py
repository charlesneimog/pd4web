import os
import sys
import zipfile

import requests
import yaml

#╭──────────────────────────────────────╮
#│    In this file we have all code     │
#│ related to get the Libraries Source  │
#│          Code, Download it.          │
#╰──────────────────────────────────────╯

class ExternalLibraries:
    def __init__(self, projectRoot : str) -> None:
        self.PROJECT_ROOT = projectRoot
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
        def __init__(self, LibraryData) -> None:
            self.name = LibraryData["name"]
            self.repoUser = LibraryData["repoUser"]
            self.repoName = LibraryData["repoName"]

            self.folder = ""
            self.externalsExtraFunctions = []
            try:
                self.repoAPI = LibraryData["downloadSrc"]
            except:
                self.repoAPI = False
                try:
                    self.directLink = LibraryData["directLink"]
                except:
                    # print in red
                    raise Exception("Error: {self.name} doesn't have a download source")
                   
            try:
                self.extraFunc = LibraryData["extraFunction"]
            except:
                self.extraFunc = None
            try:
                self.singleObject = LibraryData["singleObject"]
            except:
                self.singleObject = False
            try:
                self.requireDynamicLibraries = LibraryData["dynamicLibraries"]
            except:
                self.requireDynamicLibraries = False

            try:
                self.unsupportedObj = LibraryData["unsupportedObj"]
            except:
                self.unsupportedObj = []

            self.usedObjs = []
            self.UsedSourceFiles = []
            self.extraFuncExecuted = False
            self.unsupportedObjects = {}
            self.extraFlags = []


    def GetLibrary(self, LibraryName):
        Library = next((lib for lib in self.SupportedLibraries if lib['name'] == LibraryName), None)
        Library = self.LibraryClass(Library)
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

    # def __repr__(self) -> str:
    #     return f"<Dev: {self.repoUser} | User: {self.repoName}>"
    #
    # def __str__(self) -> str:
    #     return f"<Dev: {self.repoUser} | User: {self.repoName}>"


