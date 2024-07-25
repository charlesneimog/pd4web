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
                    with open(os.path.join(root, file), "r", encoding="utf-8") as c_file:
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
            if self.LibraryData is None or not self.LibraryData.valid:
                raise ValueError("Library not supported: " + self.Library)
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
                        return "< Pd Object: " + self.Tokens[1] + " | " + objName + " >"
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

    def __repr__(self):
        return self.__str__()


# ╭──────────────────────────────────────╮
# │    This function will process the    │
# │    patch and get all informations    │
# │           about externals            │
# ╰──────────────────────────────────────╯
class Patch:
    def __init__(self, Pd4Web: Pd4Web, abs=False, patch=None):
        self.Pd4Web = Pd4Web
        self.isAbstraction = abs

        if patch is not None:
            self.patchFile = patch
        else:
            if os.path.exists(Pd4Web.Patch):
                self.patchFile = Pd4Web.Patch
            else:
                raise ValueError("Patch not found")

        # Read Line by Line
        with open(self.patchFile, "r") as file:
            self.patchLines = file.readlines()

        # Init Supported Libraries
        self.InitVariables()
        self.pdObjects = PdObjects(Pd4Web)

        # Main Functions
        self.Execute()

        # Rewrite the patches
        self.ReConfigurePatch()

        if not abs:
            for self.absProcessed in self.absProcessed:
                self.patchLinesProcessed += self.absProcessed.patchLinesProcessed

    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.abspath(self.patchFile))
        self.libraryClass = None
        self.localAbstractions = []
        self.patchLinesExternals = []
        self.absProcessed = []
        self.patchLinesProcessed = []
        self.uiReceiversSymbol = []
        self.needExtra = False

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
        self.localAbstractions = LocalAbstractions

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

            # get higher value
            for i in range(5, len(PatchLine.Tokens)):
                try:
                    outChCount = int(PatchLine.Tokens[i])
                    if outChCount > self.Pd4Web.OUTCHS_COUNT:
                        self.Pd4Web.OUTCHS_COUNT = outChCount
                except:
                    break
            if N_CH_OUT > self.Pd4Web.OUTCHS_COUNT:
                self.Pd4Web.OUTCHS_COUNT = N_CH_OUT

        if "adc~" == ObjName:
            N_CH_IN = len(PatchLine.Tokens) - 5
            if N_CH_IN == 0 and self.Pd4Web.OUTCHS_COUNT == 0:
                N_CH_IN = 1
            for i in range(5, len(PatchLine.Tokens)):
                try:
                    outChCount = int(PatchLine.Tokens[i])
                    if outChCount > self.Pd4Web.INCHS_COUNT:
                        self.Pd4Web.INCHS_COUNT = outChCount
                except:
                    break
            if N_CH_IN > self.Pd4Web.OUTCHS_COUNT:
                self.Pd4Web.INCHS_COUNT = N_CH_IN

    def TokenIsFloat(self, token):
        token = token.replace("\n", "").replace(";", "").replace(",", "")
        try:
            return float(token)
        except:
            return float("inf")

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
        for line in enumerate(self.patchLines):
            patchLine = PatchLine()
            patchLine.index, patchLine.completLine = line
            Tokens = patchLine.completLine.replace("\n", "")
            Tokens = Tokens.replace(";", "")
            Tokens = Tokens.replace(",", "")  # when width is specificied
            patchLine.Tokens = Tokens.split(" ")

            if len(patchLine.Tokens) < 5 or patchLine.Tokens[1] != "obj":
                if len(patchLine.Tokens) == 4 and patchLine.Tokens[1] == "declare":
                    if patchLine.Tokens[2] == "-lib":
                        raise Exception(
                            "[declare -lib <LIB> is not implemented yet!")
                    elif patchLine.Tokens[2] == "-path":
                        raise Exception(
                            "[declare -path <PATH> is not implemented yet!")
                else:
                    self.patchLinesProcessed.append(patchLine)
            else:
                self.PatchObject(patchLine)

    def ReConfigurePatch(self):
        """
        Reconfigures the patch file for the project.
        Pd4Web compile as objects as native objects, so we need to remove
        the preffix of the Libraries.
        """
        if not self.isAbstraction:
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/WebPatch/"):
                os.mkdir(self.Pd4Web.PROJECT_ROOT + "/WebPatch/")
            patchFile = self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.pd"
        else:
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/.tmp/"):
                os.mkdir(self.Pd4Web.PROJECT_ROOT + "/.tmp/")
            patchFile = self.Pd4Web.PROJECT_ROOT + \
                "/.tmp/" + os.path.basename(self.patchFile)

        with open(patchFile, "w") as f:
            for line in self.patchLinesProcessed:
                if line.isExternal and not line.objwithSlash:
                    obj = line.Tokens[4].split("/")
                    if len(obj) > 1:
                        line.Tokens[4] = obj[-1]

                    # check if last two tokens are f and one int
                    if line.Tokens[-2] == "f" and line.Tokens[-1].isdigit():
                        line.Tokens[-3] = line.Tokens[-3] + ","

                    f.write(" ".join(line.Tokens) + ";\n")
                else:
                    f.write(line.completLine)

    def PatchObject(self, patchLine: PatchLine):
        patchLine.completName = patchLine.Tokens[4]
        if (
            patchLine.Tokens[0] == "#X" and patchLine.Tokens[1] == "obj" and "/" in patchLine.Tokens[4]
        ) and self.CheckIfIsSlash(patchLine):
            if os.path.exists(self.PROJECT_ROOT + "/" + patchLine.Tokens[4] + ".pd"):
                # Process abstraction
                pd4web_print(
                    f"Found Abstraction: {patchLine.Tokens[4]}", color="green", silence=self.Pd4Web.SILENCE)
                patchLine.isAbstraction = True
                abs = Patch(
                    self.Pd4Web,
                    abs=True,
                    patch=self.PROJECT_ROOT + "/" +
                    patchLine.Tokens[4] + ".pd",
                )
                self.absProcessed.append(abs)

            else:
                patchLine.isExternal = True
                patchLine.Library = patchLine.Tokens[4].split("/")[0]
                patchLine.name = patchLine.completName.split("/")[-1]
                patchLine.objGenSym = 'class_new(gensym("' + \
                    patchLine.name + '")'

        elif "s" == patchLine.Tokens[4] or "send" == patchLine.Tokens[4]:
            receiverSymbol = patchLine.Tokens[5]
            if "ui_" in receiverSymbol:
                patchLine.uiReceiver = True
                patchLine.uiSymbol = receiverSymbol
                self.Pd4Web.uiReceiversSymbol.append(receiverSymbol)
                pd4web_print("UI Sender object detected: " + receiverSymbol,
                             color="blue", silence=self.Pd4Web.SILENCE)
            patchLine.name = patchLine.completName

        elif self.TokenIsFloat(patchLine.Tokens[4]) != float("inf"):
            patchLine.name = patchLine.completName
            patchLine.isExternal = False

        elif self.TokenIsDollarSign(patchLine.Tokens[4]):
            patchLine.name = patchLine.completName
            patchLine.isExternal = False

        else:
            if patchLine.completName in self.pdObjects.getSupportedObjects()["puredata"]["objs"]:
                patchLine.name = patchLine.completName
                patchLine.isExternal = False

            elif patchLine.completName in self.localAbstractions:
                patchLine.name = patchLine.completName
                pd4web_print("Local Abstraction: " + patchLine.name,
                             color="green", silence=self.Pd4Web.SILENCE)
            else:
                raise ValueError(
                    "\n\n"
                    + getPrintValue("red")
                    + "Object not found: "
                    + patchLine.completName
                    + getPrintValue("reset")
                )

        self.SearchForSpecialObject(patchLine)
        patchLine.addToUsedObject(self.pdObjects)
        self.patchLinesProcessed.append(patchLine)
        self.AddUsedObject(patchLine)

    def AddUsedObject(self, PatchLine: PatchLine):
        if not any(obj["Lib"] == PatchLine.Library and obj["Obj"] == PatchLine.name for obj in self.Pd4Web.usedObjects):
            self.Pd4Web.usedObjects.append(
                {
                    "Lib": PatchLine.Library,
                    "Obj": PatchLine.name,
                    "SetupFunction": PatchLine.setupFunction,
                }
            )

    def SearchForExtraObjects(self):
        for obj in self.patchLinesProcessed:
            if self.needExtra == False:
                self.needExtra = self.pdObjects.isExtraObject(obj)

    def __str__(self):
        return f"< Patch: {os.path.basename(self.patchFile)} | {len(self.patchLinesProcessed)} objects >"

    def __repr__(self):
        return self.__str__()
