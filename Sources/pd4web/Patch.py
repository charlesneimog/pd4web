import json
import os
import re

from .Helpers import getPrintValue, pd4web_print
from .Libraries import ExternalLibraries
from .Objects import PdObjects
from .Pd4Web import Pd4Web


class PatchLine:
    def __init__(self):
        self.InitVariables()

    def InitVariables(self):
        self.isExternal = False
        self.isAbstraction = False
        self.isLocalAbstraction = False
        self.objwithSlash = False  # for objects like /~ / and //
        self.completLine = ""
        self.name = ""
        self.completName = ""
        self.Library = "puredata"
        self.index = 0
        self.objGenSym = ""
        self.setupFunction = ""
        self.singleObject = False
        self.genSymIndex = 0
        self.functionName = ""
        self.objFound = False
        self.uiReceiver = False
        self.uiSymbol = ""
        self.Tokens = []
        self.SpecialObjects = ["adc~", "dac~"]

    def GetChOutCount(self):
        pass

    def GetChInCount(self):
        pass

    def GetLibraryExternals(self, LibraryFolder: str, LibraryName: str):
        """
        Recursively enumerate all external and abstractions and save the JSON file.
        """
        self.LibraryScriptDir = os.path.dirname(os.path.realpath(__file__))
        externalsJson = os.path.join(self.LibraryScriptDir, "Objects.json")
        if os.path.exists(externalsJson):
            with open(externalsJson, "r") as file:
                externalsDict = json.load(file)
        else:
            externalsDict = {}
        extObjs = []
        absObjs = []
        externalsDict[LibraryName] = {}
        for root, _, files in os.walk(LibraryFolder):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    with open(os.path.join(root, file), "r", encoding='utf-8') as c_file:
                        file_contents = c_file.read()
                        pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            extObjs.append(objectName)
                if file.endswith(".pd"):
                    if "-help.pd" not in file:
                        absObjs.append(file.split(".pd")[0])
        externalsDict[LibraryName]["objs"] = extObjs
        externalsDict[LibraryName]["abs"] = absObjs
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)

    def addToUsedObject(self, PdObjects: PdObjects):
        if self.Library != "puredata":
            self.LibraryData = PdObjects.get(self.Library)
            if self.LibraryData is None:
                raise ValueError(getPrintValue("red") + "Library not supported: " + self.Library + getPrintValue("reset"))
        else:
            self.TotalObjects = PdObjects.getSupportedObjects()

    # ──────────────────────────────────────
    def GetLibraryData(self) -> ExternalLibraries.LibraryClass:
        # TODO: Need to revise this code
        return self.LibraryData 

    # ──────────────────────────────────────
    def __str__(self) -> str:
        if self.isExternal:
            return "< External Obj: " + self.name + " | Lib: " + self.Library + " >"
        else:
            if self.Tokens[0] == "#X":
                if self.Tokens[1] == "obj":
                    objName = self.Tokens[4].replace("\n", "").replace(";", "")
                    if objName in self.SpecialObjects:
                        return "< Pd Special Object: " + objName + " >"
                    else:
                        return (
                            "< Pd Object: " + self.Tokens[1] + " | " + objName + " >"
                        )
                elif self.Tokens[1] == "connect":
                    return "< Pd Connection >"
                elif self.Tokens[1] == "text":
                    return "< Pd Text >"
                elif self.Tokens[1] == "msg":
                    return "< Pd Message >"
                elif self.Tokens[1] == "floatatom":
                    return "< Pd Float >"
                elif self.Tokens[1] == "restore":
                    return "< Pd Restore >"
                else:
                    return "< Pd Object: " + str(self.Tokens) + " >"

            elif self.Tokens[0] == "#A":
                return "< Array Data >"

            else:
                return "< Special Pd Object: " + self.Tokens[0] + " >"

    # ──────────────────────────────────────
    def __repr__(self):
        return self.__str__()

    # ──────────────────────────────────────


#╭──────────────────────────────────────╮
#│    This function will process the    │
#│    patch and get all informations    │
#│           about externals            │
#╰──────────────────────────────────────╯
class Patch():
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web

        if os.path.exists(Pd4Web.Patch):
            self.patchFile = Pd4Web.Patch
        else:
            raise ValueError("Patch not found")

        # Read Line by Line
        with open(self.patchFile, "r") as file:
            self.PatchLines = file.readlines()

        # Init Supported Libraries
        self.InitVariables()
        self.PdObjects = PdObjects(Pd4Web)

        # Main Functions
        self.Execute()

    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.abspath(self.patchFile))
        self.LibraryClass = None
        self.LocalAbstractions = []
        self.PatchLinesExternals = []
        self.PatchLinesProcessed = []
        self.UiReceiversSymbol = []
        self.NeedExtra = False

    def Execute(self):
        # Abstractions
        self.GetAbstractions()

        # Find Externals in Patch 
        self.ProcessPatch()
        self.SearchForExtraObjects()

    # ╭──────────────────────────────────────╮
    # │             Abstractions             │
    # ╰──────────────────────────────────────╯
    def GetAbstractions(self):
        """
        This function list all pd patch in the same folder of the main patch.
        """
        LocalAbstractions = []
        files = os.listdir(os.path.dirname(self.PROJECT_ROOT))

        for file in files:
            if file.endswith(".pd"):
                LocalAbstractions.append(file.split(".pd")[0])

        self.LocalAbstractions = LocalAbstractions

    # ╭──────────────────────────────────────╮
    # │        Find Externals Objects        │
    # ╰──────────────────────────────────────╯
    def CheckIfIsSlash(self, line: PatchLine):
        """
        The function search for objects like else/count and others, what split
        the library and the object name. For objects like /, //, /~ and //~ this
        function will return False.
        """
        objName = line.completName
        if objName == "/" or objName == "//" or objName == "/~" or objName == "//~":
            line.objwithSlash = True
            return False
        return True

    def CheckIfObjIsLibrary(self, patchLine):
        """
        This function check if the object has the same name as the library.
        For example, earplug~ for earplug~ Library.
        """
        patchLine = patchLine.Tokens
        if patchLine[1] == "obj":
            nameOfTheObject = patchLine[4].replace(";", "").replace("\n", "")
            nameOfTheObject = nameOfTheObject.replace(",", "")
            if nameOfTheObject in self.PdObjects.LibraryNames:
                LibraryClass = self.PdObjects.get(nameOfTheObject)
                if LibraryClass is None:
                    pd4web_print("Library not found: " + nameOfTheObject, color="red")
                    return False

                # if LibraryClass and LibraryClass.SingleObject: # TODO: Revisar esse código
                #     return True
        return False

    def SearchForSpecialObject(self, PatchLine: PatchLine):
        """
        There is some special objects that we need extra configs.
        This function will search for these objects and add the configs.
        """
        # There is no special object for now
        ObjName = PatchLine.name
        if "dac~" == ObjName:
            N_CH_OUT = len(PatchLine.Tokens) - 5
            if N_CH_OUT == 0 and self.Pd4Web.OUTCHS_COUNT == 0:
                self.Pd4Web.OUTCHS_COUNT = 2
            if N_CH_OUT > self.Pd4Web.OUTCHS_COUNT:
                self.Pd4Web.OUTCHS_COUNT = N_CH_OUT
        if "adc~" == ObjName:
            N_CH_IN = len(PatchLine.Tokens) - 5
            if N_CH_IN == 0:
                N_CH_IN = 1

    def TokenIsFloat(self, token):
        token = token.replace("\n", "").replace(";", "").replace(",", "")
        try:
            return float(token)
        except:
            return float('inf')

    def TokenIsDollarSign(self, token):
        if token[0] == "$":
            return True
        elif token[0] == "\\" and token[1] == "$":
            return True
        else: 
            return False

    def ProcessPatch(self):
        """
        This function will find all externals objects in the patch.
        """
        for line in enumerate(self.PatchLines):
            patchLine = PatchLine()
            patchLine.index, patchLine.completLine = line
            Tokens = patchLine.completLine.replace("\n", "")
            Tokens = Tokens.replace(";", "")
            Tokens = Tokens.replace(",", "") # when width is specificied
            patchLine.Tokens = Tokens.split(" ")

            if len(patchLine.Tokens) < 5 or patchLine.Tokens[1] != "obj":
                if len(patchLine.Tokens) == 4 and patchLine.Tokens[1] == "declare":
                    if patchLine.Tokens[2] == "-lib":
                        raise Exception("[declare -lib <LIB> is not implemented yet!")
                    elif patchLine.Tokens[2] == "-path":
                        raise Exception("[declare -path <PATH> is not implemented yet!")
                else:
                    pass
                    # print(patchLine.Tokens)
            else:
                self.PatchObject(patchLine)


    def PatchObject(self, patchLine: PatchLine):

        patchLine.completName = patchLine.Tokens[4]
        if (
            patchLine.Tokens[0] == "#X"
            and patchLine.Tokens[1] == "obj"
            and "/" in patchLine.Tokens[4]
        ) and self.CheckIfIsSlash(patchLine):
            patchLine.isExternal = True
            patchLine.Library = patchLine.Tokens[4].split("/")[0]
            patchLine.name = patchLine.completName.split("/")[-1]
            patchLine.objGenSym = (
                'class_new(gensym("' + patchLine.name + '")'
            )

        elif self.CheckIfObjIsLibrary(patchLine):
            # TODO: I don't thing this is necessary anymore
            patchLine.isExternal = True
            patchLine.Library = patchLine.completName
            patchLine.name = patchLine.Library
            if os.path.exists(patchLine.Library + ".pd"):
                pd4web_print("It is an abstraction", color="red")
            patchLine.objGenSym = 'gensym("' + patchLine.name + '")'
            patchLine.singleObject = True

        elif "s" == patchLine.Tokens[4] or "send" == patchLine.Tokens[4]:
            receiverSymbol = patchLine.Tokens[5]
            if "ui_" in receiverSymbol:
                patchLine.uiReceiver = True
                patchLine.uiSymbol = receiverSymbol
                self.Pd4Web.UiReceiversSymbol.append(receiverSymbol) 
                pd4web_print(
                    "UI Sender object detected: " + receiverSymbol, color="blue"
                )
            patchLine.name = patchLine.completName

        elif self.TokenIsFloat(patchLine.Tokens[4]) != float('inf'):
            patchLine.name = patchLine.completName
            patchLine.isExternal = False

        elif self.TokenIsDollarSign(patchLine.Tokens[4]): 
            patchLine.name = patchLine.completName
            patchLine.isExternal = False

        else:
            if patchLine.completName in self.PdObjects.getSupportedObjects()["puredata"]["objs"]:
                patchLine.name = patchLine.completName
                patchLine.isExternal = False
            elif patchLine.completName in self.LocalAbstractions:
                patchLine.name = patchLine.completName
                pd4web_print("Local Abstraction: " + patchLine.name, color="green")
            else:
                raise ValueError("\n\n" + getPrintValue("red") + "Object not found: " + patchLine.completName + getPrintValue("reset"))

        self.SearchForSpecialObject(patchLine)
        patchLine.addToUsedObject(self.PdObjects)
        self.PatchLinesProcessed.append(patchLine)
        self.AddUsedObject(patchLine)

    def AddUsedObject(self, PatchLine: PatchLine):
        if not any(obj["Lib"] == PatchLine.Library and obj["Obj"] == PatchLine.name for obj in self.Pd4Web.UsedObjects):
            self.Pd4Web.UsedObjects.append({"Lib": PatchLine.Library, "Obj": PatchLine.name, "SetupFunction": PatchLine.setupFunction})


    def SearchForExtraObjects(self):
        for obj in self.PatchLinesProcessed:
            if self.NeedExtra == False:
                self.NeedExtra = self.PdObjects.isExtraObject(obj)

    def __str__(self):
        return f"< Patch: {os.path.basename(self.patchFile)} | {len(self.PatchLinesProcessed)} objects >"

    def __repr__(self):
        return self.__str__()

