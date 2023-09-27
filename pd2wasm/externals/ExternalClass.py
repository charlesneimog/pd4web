import sys
from typing import Optional
from ..helpers import myprint

class PureDataExternals:
    def __init__(self, library, projectRoot) -> None:
        self.name = library['name']
        self.repoUser = library['repoUser']
        self.repoName = library['repoName']
        from ..pd2wasm import webpdPatch
        self.webpdPatch: Optional[webpdPatch]
        self.folder = ''
        self.externalsExtraFunctions = []
        try:
            self.repoAPI = library['download_source']
        except:
            self.repoAPI = False
            try:
                self.directLink = library['direct_link']
            except:
                # print in red
                myprint(f"Error: {self.name} doesn't have a download source", color="red")
                sys.exit(1)

        try:
            self.extraFunc = library['extraFunction']
        except:
            self.extraFunc = None

        try:
            self.singleObject = library['singleObject']
        except:
            self.singleObject = False

        try:
            self.requireDynamicLibraries = library['dynamicLibraries']
        except:
            self.requireDynamicLibraries = False

        self.usedObjs = []
        self.UsedSourceFiles = []
        self.extraFuncExecuted = False
        self.PROJECT_ROOT = projectRoot
        self.extraFlags = []

    def addToUsed(self, objName):
        self.usedObjs.append(objName)

    def getUsedObjs(self):
        return self.usedObjs

    def __repr__(self) -> str:
        return f"<Dev: {self.repoUser} | User: {self.repoName}>"

    def __str__(self) -> str:
        return f"<Dev: {self.repoUser} | User: {self.repoName}>"


class PD_SUPPORTED_EXTERNALS:
    def __init__(self) -> None:
        self.PureDataExternals = []
        self.LibraryNames = []
        self.UsedLibraries = []
        self.UsedLibrariesNames = []
        self.totalOfLibraries = 0

    def add(self, PureDataExternals):
        self.PureDataExternals.append(PureDataExternals)
        self.LibraryNames.append(PureDataExternals.name)
        self.totalOfLibraries += 1


    def get(self, name):
        for i in self.PureDataExternals:
            if i.name == name:
                return i
        return None


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
                # print("LINK: ", supportedDownloads[libraryName.repoAPI].format(libraryName.repoUser, libraryName.repoName))
                return supportedDownloads[libraryName.repoAPI].format(libraryName.repoUser, libraryName.repoName)
            except:
                return None


    def executeExtraFunction(self, UsedLibrary):
        if UsedLibrary.extraFunc != None and UsedLibrary in self.UsedLibraries:
            myprint(f"Executing extra configs for {UsedLibrary.name}", color="magenta")
            libraryClass = self.isUsed(UsedLibrary.name)
            extraFunctionStr = UsedLibrary.extraFunc
            function = None
            for module in UsedLibrary.externalsExtraFunctions:
                for definedThing in dir(module):
                    if definedThing == extraFunctionStr:
                        function = getattr(module, extraFunctionStr)
                        break
            if function is None and extraFunctionStr != None:
                myprint(f"Error: {extraFunctionStr} is not defined in {UsedLibrary.name}", color="red")

            if function is not None:      
                function(libraryClass)
            else:
                return []

            if libraryClass:
                return libraryClass.extraFlags
            else:
                return []

    def __repr__(self) -> str:

        return f"<PD_EXTERNALS | Total: {self.totalOfLibraries}>"


    def __str__(self) -> str:
        return f"<PD_EXTERNALS | Total: {self.totalOfLibraries}>"



class PatchLine:
    def __init__(self):
        self.isExternal = False
        self.isAbstraction = False
        self.name = ''
        self.library = ''
        self.patchLineIndex = 0
        self.patchLine = ''
        self.objGenSym = ''
        self.singleObject = False
        self.genSymIndex = 0
        self.functionName = ''
        self.objFound = False
        self.uiReceiver = False
        self.uiSymbol = ''
        self.Tokens = []

    def __str__(self) -> str:
        if self.isExternal:
            return "<Obj: " + self.name + " | Lib: " + self.library + ">"
        else:
            if self.Tokens[0] == '#X':
                if self.Tokens[1] == 'obj':
                    removeNewLines = self.Tokens[4].replace('\n', '')
                    return "<Pd Object: " + self.Tokens[1] + " | " + removeNewLines + ">"
                elif self.Tokens[1] == 'connect': 
                    return "<Pd Connection>"
                elif self.Tokens[1] == 'text':
                    return "<Pd Text>"
                elif self.Tokens[1] == 'msg':
                    return "<Pd Message>"
                elif self.Tokens[1] == 'floatatom':
                    return "<Pd Float>"
                elif self.Tokens[1] == 'restore':
                    return "<Pd Restore>"
                else:
                    return "<Pd Object: " + str(self.Tokens) + ">"
                
            elif self.Tokens[0] == '#A':
                return "<Array Data>"

            else:
                return "<Special Pd Object: " + self.Tokens[0] + ">"


    def addToUsedObject(self, PD_LIBS):
        if self.isExternal:
            LibraryClass = PD_LIBS.get(self.library)
            LibraryClass.addToUsed(self.name)
