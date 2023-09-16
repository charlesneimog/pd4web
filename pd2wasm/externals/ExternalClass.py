import os
import sys
import importlib


thisFile = os.path.abspath(__file__)
module_files = [f for f in os.listdir(os.path.dirname(thisFile)) if f.endswith('.py') and not f.startswith('__')]
module_names = [os.path.splitext(f)[0] for f in module_files]


for module_name in module_names:
    if module_name != 'ExternalClass':
        module = importlib.import_module('pd2wasm.externals.' + module_name)
        globals().update(vars(module))


class PureDataExternals:
    def __init__(self, library) -> None:
        self.name = library['name']
        self.repoUser = library['repoUser']
        self.repoName = library['repoName']
        self.webpdPatch = None
        try:
            self.repoAPI = library['download_source']
        except:
            self.repoAPI = False
            try:
                self.directLink = library['direct_link']
            except:
                # print in red
                print(f"\033[91mError: {self.name} doesn't have a download source\033[0m")
                sys.exit()

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
        self.PROJECT_ROOT = os.getcwd()
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
                return supportedDownloads[libraryName.repoAPI].format(libraryName.repoUser, libraryName.repoName)
            except:
                return None


    def executeExtraFunction(self, UsedLibrary):
        if UsedLibrary.extraFunc != None and UsedLibrary in self.UsedLibraries:
            print("\033[95m" + f"    Executing extra configs for {UsedLibrary.name}")
            libraryClass = self.isUsed(UsedLibrary.name)
            extraFunctionStr = UsedLibrary.extraFunc
            executedFunction = f"{extraFunctionStr}" + '(libraryClass)'
            exec(executedFunction)
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


    def __repr__(self) -> str:
        if self.isExternal:
            return "<Obj: " + self.name + " | Lib: " + self.library + ">"
        else:
            if self.Tokens[0] == '#X':
                return "<Pd Object: " + self.Tokens[4] + " | " + self.Tokens[1] + ">"
                
            else:
                return "<Special Pd Object: " + self.Tokens[0] + ">"
            


    def addToUsedObject(self, PD_LIBS):
        if self.isExternal:
            LibraryClass = PD_LIBS.get(self.library)
            LibraryClass.addToUsed(self.name)
