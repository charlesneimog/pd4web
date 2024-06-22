import json
import os
import re

from .Helpers import pd4web_print
from .Libraries import ExternalLibraries
from .Pd4Web import Pd4Web


class PdObjects():
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT

        self.InitVariables()

        # Get Supported Libraries
        # self.getSupportedLibraries()

        # Get Used Libraries

    def InitVariables(self):
        self.Libraries = []
        self.SupportedLibraries = []
        self.SearchSupportedObjects()
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

    def get(self, name):
        LibraryData = self.Libraries.GetLibrary(name)
        return LibraryData 

    def isExtraObject(self, name):
        ExtraObjects = ["bob~", "bonk~", "choice", "fiddle~", "loop~", "lrshift~", "pd~", "pique", "sigmund~", "stdout", "hilbert~", "complex-mod~", "rev1~", "output~"]
        if name in ExtraObjects:
            return True
        else:
            return False

    def isUsed(self, name):
        for i in self.UsedLibraries:
            if i.name == name:
                return i
        return False

    def getDownloadURL(self, libraryName, supportedDownloads):
        if libraryName.repoAPI == False:
            return False

        else:
            try:
                return supportedDownloads[libraryName.repoAPI].format(
                    libraryName.repoUser, libraryName.repoName
                )
            except:
                return None

    def executeExtraFunction(self, UsedLibrary):
        if UsedLibrary.extraFunc != None and UsedLibrary in self.UsedLibraries:
            pd4web_print(f"Executing extra configs for {UsedLibrary.name}", color="magenta")
            libraryClass = self.isUsed(UsedLibrary.name)
            extraFunctionStr = UsedLibrary.extraFunc
            function = None
            for module in UsedLibrary.externalsExtraFunctions:
                for definedThing in dir(module):
                    if definedThing == extraFunctionStr:
                        function = getattr(module, extraFunctionStr)
                        break
            if function is None and extraFunctionStr != None:
                pd4web_print(
                    f"Error: {extraFunctionStr} is not defined in {UsedLibrary.name}",
                    color="red",
                )

            if function is not None:
                function(libraryClass)
            else:
                return []
            if libraryClass:
                return libraryClass.extraFlags
            else:
                return []

    def SearchSupportedObjects(self):
        """
        It get all the PureData objects.
        """
        # get path for this file
        file = os.path.realpath(__file__)
        dirname = os.path.dirname(file)
        libpd = os.path.join(dirname, "../libpd")
        if not os.path.exists(libpd):
            raise ValueError("libpd not found")
        else:
            pass

        externalsJson = os.path.join(dirname, "Objects.json")
        if not os.path.exists(externalsJson):
            with open(externalsJson, "w") as file:
                json.dump({}, file, indent=4)
        with open(externalsJson, "r") as file:
            externalsDict = json.load(file)
        externalsDict["puredata"] = {}
        puredataObjs = []
        puredataFolder = os.path.join(dirname, "../libpd/pure-data/")
        for root, _, files in os.walk(puredataFolder):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    with open(os.path.join(root, file), "r") as c_file:
                        file_contents = c_file.read()
                        pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            puredataObjs.append(objectName)
                        creatorPattern = (
                            r'class_addcreator\([^,]+,\s*gensym\("([^"]+)"\)'
                        )
                        creatorMatches = re.finditer(creatorPattern, file_contents)
                        for match in creatorMatches:
                            objectName = match.group(1)
                            puredataObjs.append(objectName)

        # NOTE: objects that not use class_new gensym...

        puredataObjs.append("list")
        extObjs = list(set(puredataObjs))
        externalsDict["puredata"]["objs"] = extObjs
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)
        self.supportedObjects = externalsDict

    def getSupportedObjects(self):
        return self.supportedObjects



