from .Pd4Web import Pd4Web
from .Helpers import pd4web_print

import os
import re
import json


class Objects:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.InitVariables()

    def InitVariables(self):
        self.Libraries = []
        self.SupportedLibraries = []
        self.LibraryNames = []
        self.UsedLibraries = []
        self.UsedLibrariesNames = []
        self.Libraries = self.Pd4Web.Libraries

        self.LibraryNames = []
        self.unsupportedObjects = {}
        self.TotalOfLibraries = 0

    def __repr__(self) -> str:
        return f"<PD_OBJECTS | Total: {self.TotalOfLibraries}>"

    def __str__(self) -> str:
        return self.__repr__()

    def isExtraObject(self, name):
        ExtraObjects = [
            "bob~",
            "bonk~",
            "choice",
            "fiddle~",
            "loop~",
            "lrshift~",
            "pd~",
            "pique",
            "sigmund~",
            "stdout",
            "hilbert~",
            "complex-mod~",
            "rev1~",
            "output~",
        ]
        if name in ExtraObjects:
            return True
        else:
            return False

    def isUsed(self, name):
        for i in self.UsedLibraries:
            if i.name == name:
                return i
        return False

    def GetDownloadURL(self, libraryName, supportedDownloads):
        if libraryName.repoAPI == False:
            return False
        else:
            try:
                return supportedDownloads[libraryName.repoAPI].format(libraryName.repoUser, libraryName.repoName)
            except:
                return None

    def GetLibraryObjects(self, libFolder: str, libName: str):
        """
        Recursively enumerate all external and abstractions and save the JSON file.
        """
        pd4web_print(
            f"Listing all external supported by {libName}, this may take a while...",
            color="blue",
            silence=self.Pd4Web.SILENCE,
            pd4web=self.Pd4Web.PD_EXTERNAL,
        )

        externalsJson = os.path.join(self.PROJECT_ROOT, "Pd4Web/Externals/Objects.json")
        directory = os.path.dirname(externalsJson)
        if not os.path.exists(directory):
            os.makedirs(directory)

        if os.path.exists(externalsJson):
            with open(externalsJson, "r") as file:
                externalsDict = json.load(file)
        else:
            externalsDict = {}
        extObjs = []
        absObjs = []
        externalsDict[libName] = {}
        for root, _, files in os.walk(libFolder):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    with open(os.path.join(root, file), "r", encoding="utf-8") as c_file:
                        file_contents = c_file.read()
                        pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            extObjs.append(objectName)

                        pattern = r'class_addcreator\s*\(\s*\([^,]*,\s*gensym\s*\(\s*"([^"]*)"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            extObjs.append(objectName)

                if file.endswith(".pd"):
                    if "-help.pd" not in file:
                        absObjs.append(file.split(".pd")[0])

        if libName == "pure-data":
            extObjs.append("pointer")
            extObjs.append("float")
            extObjs.append("symbol")
            extObjs.append("bang")
            extObjs.append("list")
        externalsDict[libName]["objs"] = extObjs
        externalsDict[libName]["abs"] = absObjs
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)

    def GetSupportedObjects(self, libName: str):
        if libName == "pure-data":
            libFolder = os.path.join(self.PROJECT_ROOT, "Pd4Web/", libName)
        else:
            libFolder = os.path.join(self.PROJECT_ROOT, "Pd4Web/Libraries", libName)

        externalsJson = os.path.join(self.PROJECT_ROOT, "Pd4Web/Externals/Objects.json")
        if not os.path.exists(externalsJson):
            self.GetLibraryObjects(libFolder, libName)
        with open(externalsJson, "r") as file:
            externalsDict = json.load(file)
        if libName not in externalsDict:
            self.GetLibraryObjects(libFolder, libName)
            with open(externalsJson, "r") as file:
                externalsDict = json.load(file)
        return externalsDict[libName]["objs"]
