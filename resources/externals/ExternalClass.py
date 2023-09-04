import os
import importlib

thisFile = os.path.abspath(__file__)
module_files = [f for f in os.listdir(os.path.dirname(thisFile)) if f.endswith('.py') and not f.startswith('__')]
module_names = [os.path.splitext(f)[0] for f in module_files]

for module_name in module_names:
    if module_name != 'ExternalClass':
        module = importlib.import_module('externals.' + module_name)
        globals().update(vars(module))


class PureDataExternals:
    def __init__(self, repoAPI, user, repo, name, extraFunc='', single=False) -> None:
        self.usedObjs = []
        self.repoAPI = repoAPI
        self.username = user
        self.repo = repo
        self.name = name
        self.folder = ''
        self.singleObject = single
        self.extraFunc = extraFunc
        self.requireDynamicLibraries = False
        self.dynamicLibraries = []
        self.extraFuncExecuted = False
        self.PROJECT_ROOT = os.getcwd()
        self.extraFlags = []

    def addToUsed(self, objName):
        self.usedObjs.append(objName)
        

    def getUsedObjs(self):
        return self.usedObjs

    def __repr__(self) -> str:
        return f"<Dev: {self.username} | User: {self.repo}>"

    def __str__(self) -> str:
        return f"<Dev: {self.username} | User: {self.repo}>"



class PD_SUPPORTED_EXTERNALS:
    def __init__(self) -> None:
        self.PureDataExternals = []
        self.LibraryNames = []
        self.UsedLibraries = []
        self.UsedLibrariesNames = []


    def add(self, PureDataExternals):
        self.PureDataExternals.append(PureDataExternals)
        self.LibraryNames.append(PureDataExternals.name)


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


    def getDownloadURL(self, name):
        for i in self.PureDataExternals:
            if i.name == name:
                return i.repoAPI.format(i.username, i.repo)
        return None


    def executeExtraFunction(self):
        for i in self.PureDataExternals:
            if i.extraFunc != None and i in self.UsedLibraries:
                libraryClass = self.isUsed(i.name)
                extraFunctionStr = i.extraFunc
                executedFunction = f"{extraFunctionStr}" + '(libraryClass)'
                print(executedFunction)
                exec(executedFunction)
                return libraryClass.extraFlags
    def __repr__(self) -> str:
        return f"<PD_EXTERNALS>"


    def __str__(self) -> str:
        return f"<PD_EXTERNALS>"



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
            
    def __str__(self) -> str:
        if self.isExternal:
            return "<Obj: " + self.name + " | Lib: " + self.library + ">"
        else:
            return "<Not an external>"


    def __repr__(self) -> str:
        if self.isExternal:
            return "<Obj: " + self.name + " | Lib: " + self.library + ">"
        else:
            return "<Not an external>"


    def addToUsedObject(self, PD_LIBRARIES):
        if self.isExternal:
            LibraryClass = PD_LIBRARIES.get(self.library)
            LibraryClass.addToUsed(self.name)
