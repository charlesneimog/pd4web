import argparse
import datetime
import importlib
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import time
import zipfile
from typing import List

import requests
import yaml

from .externals import PD_SUPPORTED_EXTERNALS, PatchLine, PureDataExternals
from .helpers.helpers import emccPaths, myprint
from .lib.DynamicLibraries import DYNAMIC_LIBRARIES

## ================== EXTERNALS THINGS ================== ##
INDEX_HTML = """
<!doctype html>
<html lang="en-us">
    <body>
    <script>
        window.location.href = 'webpatch/{}';
    </script>
    </body>
</html>
"""


class webpdPatch:
    def __init__(
        self,
        sourcefile="src/template.c",
        pdpatch=None,
        insideaddAbstractions=False,
        runMain=True,
        parent=[],
    ) -> None:
        self.PdWebCompilerPath = os.path.dirname(os.path.realpath(__file__))
        self.emcc = emccPaths()
        # get this folder directory
        parser = argparse.ArgumentParser(
            formatter_class=argparse.RawTextHelpFormatter,
            description="Check the complete docs in https://charlesneimog.github.io/pd4web",
        )
        parser.add_argument(
            "--patch", required=False, help="Patch file (.pd) to compile"
        )

        parser.add_argument(
            "--html",
            required=False,
            help="HTML used to load and render the Web Page. If not defined, we use the default one",
        )
        parser.add_argument(
            "--page-folder",
            required=False,
            help="Folder with html, css, js and all others files to be used in the Web Page. If not defined, we use the default one",
        )
        parser.add_argument(
            "--no_browser", action="store_true", help="Set the flag to True"
        )
        parser.add_argument(
            "--compile-all", action="store_true", help="Set the flag to True"
        )
        parser.add_argument(
            "--active-emcc", action="store_true", help="Set the flag to True"
        )
        parser.add_argument(
            "--confirm",
            action="store_true",
            help="There is some automatic way check if the external is correct, but it is not always accurate. If you want to confirm if the external is correct, use this flag",
        )
        parser.add_argument(
            "--clearTmpFiles",
            required=False,
            default=False,
            help="Remove all TempFiles, like .externals folder",
        )
        parser.add_argument(
            "--server-port",
            required=False,
            default=False,
            help="Set the port to start the server",
        )
        parser.add_argument(
            "--initial-memory",
            required=False,
            default=32,
            help="Set the initial memory of the WebAssembly in MB",
        )
        parser.add_argument(
            "--no-root-html",
            required=False,
            action="store_true",
            default=False,
            help="Do not create the index.html in the root folder",
        )
        parser.add_argument(
            "--replace-helper",
            required=False,
            default=None,
            help="Replace helpers.js file by your own file",
        )
        parser.add_argument("--version", action="version", version="%(prog)s 1.2.2")
        self.args = parser.parse_args()
        if self.args.active_emcc:
            self.activeEmcc()
            sys.exit(0)
        if pdpatch is not None:
            self.args.patch = pdpatch
        if not os.path.isabs(self.args.patch) and not insideaddAbstractions:
            print("\n")
            absolutePath = os.path.dirname(
                os.path.abspath(os.path.join(os.getcwd(), self.args.patch))
            )
            self.patch = os.getcwd() + "/" + self.args.patch
            self.source = sourcefile
            self.PROJECT_ROOT = absolutePath
            os.chdir(absolutePath)
        elif insideaddAbstractions and parent != []:
            self.PROJECT_ROOT = parent.PROJECT_ROOT
        elif os.path.isabs(self.args.patch) and not insideaddAbstractions:
            self.PROJECT_ROOT = os.path.dirname(self.args.patch)
        if not os.path.exists(self.PdWebCompilerPath + "/.lib"):
            os.mkdir(self.PdWebCompilerPath + "/.lib")
        if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
            os.mkdir(self.PdWebCompilerPath + "/.externals")
        self.externalsExtraFunctions = []
        self.addedObjects = []
        self.supportedObjects = {}
        self.unsupportedObjects = {}
        if not insideaddAbstractions:
            self.activeEmcc()
            self.downloadLibPd()
            self.importExternalObjs()
            self.getSupportedLibraries()
            self.C_functionsDeclarationStarted = False
            self.C_functionsCalledStarted = False
            self.uiReceiversSymbol = []
        else:
            self.uiReceiversSymbol = parent.uiReceiversSymbol
            self.downloadSources = parent.downloadSources
            self.externalsExtraFunctions = parent.externalsExtraFunctions
            self.addedObjects = parent.addedObjects
        if self.PROJECT_ROOT[-1] != "/" and (self.PROJECT_ROOT[-1] != "\\"):
            if platform.system() == "Windows":
                self.PROJECT_ROOT = self.PROJECT_ROOT + "\\"
            else:
                self.PROJECT_ROOT = self.PROJECT_ROOT + "/"
        self.FoundExternals = False
        self.jsHelper = self.args.replace_helper
        self.compile_all = self.args.compile_all
        self.html = False
        self.pageFolder = self.args.page_folder
        self.parent = parent
        self.source = sourcefile
        self.PatchLinesProcessed = []
        # case there is some files that need to be compiled in order
        self.sortedSourceFiles = []
        self.PROCESSED_ABSTRACTIONS = []
        self.clearTmpFiles = self.args.clearTmpFiles
        self.uiReceiversSymbol = []
        self.insideaddAbstractions = insideaddAbstractions
        self.lastPrintedLine = ""
        self.memory = self.args.initial_memory
        self.extraFlags = []
        self.externalsDict = {}
        if runMain:
            self.main(pdpatch=pdpatch, insideaddAbstractions=insideaddAbstractions)
        else:
            myinput = input("Do you want to compile the patch? [Y/n]: ")
            if myinput == "Y" or myinput == "y":
                self.main(pdpatch=pdpatch, insideaddAbstractions=insideaddAbstractions)
            else:
                myprint("Bye Bye!", color="green")

    def main(self, pdpatch=None, insideaddAbstractions=False):
        """
        Main functions, it will call all other functions.
        """

        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = self.args.patch
        patchFileName = os.path.basename(self.patch)
        myprint("Patch => " + self.patch, color="blue")
        if self.args.html is not None:
            if not os.path.isabs(self.args.html) and not insideaddAbstractions:
                self.html = os.getcwd() + "/" + self.args.html
        elif self.pageFolder is not None:
            if not os.path.isabs(self.pageFolder) and not insideaddAbstractions:
                self.pageFolder = os.getcwd() + "/" + self.pageFolder
        else:
            self.html = self.PdWebCompilerPath + "/src/index.html"
        if "index.html" not in str(self.html) and not insideaddAbstractions:
            myprint(
                "The name of your html is not index.html, \
                we will copy one index.html for webpatch!",
                color="yellow",
            )
        if not os.path.exists(self.PROJECT_ROOT + "/.backup"):
            os.mkdir(self.PROJECT_ROOT + "/.backup")
        if not insideaddAbstractions:
            if os.path.exists(self.PROJECT_ROOT + "index.html"):
                myprint(
                    "index.html already exists in the root folder, "
                    "please change his name or delete it, making backup and deleting it.",
                    color="yellow",
                )
                shutil.copy(
                    self.PROJECT_ROOT + "index.html",
                    self.PROJECT_ROOT + ".backup/index.html",
                )
            elif not self.args.no_root_html:
                with open(self.PROJECT_ROOT + "/index.html", "w") as file:
                    file.write(INDEX_HTML.format(os.path.basename(str(self.html))))

        if not os.path.exists(self.args.patch):
            notFound = True
            for root, _, files in os.walk(self.PROJECT_ROOT):
                for file in files:
                    if not file.endswith(".pd"):
                        continue
                    if file != patchFileName:
                        continue
                    self.args.patch = os.path.join(root, file)
                    notFound = False
                    break
            if notFound:
                myprint(
                    "Patch not found: The current folder is " + str(os.getcwd()),
                    color="red",
                )
                sys.exit(0)
        with open(self.args.patch, "r") as file:
            self.PatchFileLines = file.readlines()
        self.replaceVisualArray()
        self.processedAbstractions = []
        if not insideaddAbstractions:
            with open(
                os.path.join(self.PdWebCompilerPath, "src/template.c"), "r"
            ) as file:
                self.templateCode = file.readlines()
            if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
                os.mkdir(self.PdWebCompilerPath + "/.externals")
            if not os.path.exists(self.PROJECT_ROOT + "webpatch"):
                os.mkdir(self.PROJECT_ROOT + "webpatch")
            else:
                shutil.rmtree(self.PROJECT_ROOT + "webpatch")
                os.mkdir(self.PROJECT_ROOT + "webpatch")
            if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
                os.mkdir(self.PROJECT_ROOT + "webpatch/data")
            else:
                shutil.rmtree(self.PROJECT_ROOT + "webpatch/data")
                os.mkdir(self.PROJECT_ROOT + "webpatch/data")

            if not os.path.exists(self.PROJECT_ROOT + "webpatch/externals"):
                os.mkdir(self.PROJECT_ROOT + "webpatch/externals")
            else:
                shutil.rmtree(self.PROJECT_ROOT + "webpatch/externals")
                os.mkdir(self.PROJECT_ROOT + "webpatch/externals")
            if not os.path.exists(self.PROJECT_ROOT + "webpatch/includes"):
                os.mkdir(self.PROJECT_ROOT + "webpatch/includes")
            else:
                shutil.rmtree(self.PROJECT_ROOT + "webpatch/includes")
                os.mkdir(self.PROJECT_ROOT + "webpatch/includes")
        else:
            self.templateCode = self.parent.templateCode

        if insideaddAbstractions:
            self.C_functionsDeclarationStarted = (
                self.parent.C_functionsDeclarationStarted
            )
            self.C_functionsCalledStarted = self.parent.C_functionsCalledStarted
        self.librariesFolder = []
        self.confirm = self.args.confirm
        self.mkBackup()
        self.declaredLibraries = []
        self.localAbstractions = []
        self.PatchLinesExternals = []
        self.getAllSupportedObjects()
        self.enumerateLocalAbstractions()
        self.findExternalsObjs()
        self.cfgExternalThing()
        self.addObjSetup()
        self.savePdPatchModified()
        if not insideaddAbstractions:
            self.copyAllDataFiles()
            self.processAbstractions()
            shutil.copy(
                self.PdWebCompilerPath + "/src/index.html",
                self.PROJECT_ROOT + "webpatch/index.html",
            )
            if self.jsHelper is not None:
                # check if the file is relative or absolute
                if not os.path.isabs(self.jsHelper):
                    self.jsHelper = os.getcwd() + "/" + self.jsHelper
                if not os.path.exists(self.jsHelper):
                    myprint(
                        "The file " + self.jsHelper + " does not exist!", color="red"
                    )
                    sys.exit(1)
                shutil.copy(self.jsHelper, self.PROJECT_ROOT + "webpatch/main.js")
            else:
                shutil.copy(
                    self.PdWebCompilerPath + "/src/main.js",
                    self.PROJECT_ROOT + "webpatch/main.js",
                )
            shutil.copy(
                self.PdWebCompilerPath + "/src/enable-threads.js",
                self.PROJECT_ROOT + "webpatch/enable-threads.js",
            )
            shutil.copy(
                self.PdWebCompilerPath + "/src/gui.js",
                self.PROJECT_ROOT + "webpatch/gui.js",
            )
            # check if webpatch/css folder exists
            if not os.path.exists(self.PROJECT_ROOT + "webpatch/css"):
                os.mkdir(self.PROJECT_ROOT + "webpatch/css")

            shutil.copy(
                self.PdWebCompilerPath + "/src/css/main.css",
                self.PROJECT_ROOT + "webpatch/css/main.css",
            )
            shutil.copy(
                self.PdWebCompilerPath + "/src/css/dejavu.css",
                self.PROJECT_ROOT + "webpatch/css/dejavu.css",
            )
            if self.pageFolder is not None:
                shutil.copytree(
                    self.pageFolder,
                    os.path.join(self.PROJECT_ROOT, "webpatch"),
                    dirs_exist_ok=True,
                )

        if insideaddAbstractions:
            [
                self.parent.sortedSourceFiles.append(sourceFile)
                for sourceFile in self.sortedSourceFiles
            ]
            [
                self.parent.PROCESSED_ABSTRACTIONS.append(pdpatch)
                for pdpatch in self.PROCESSED_ABSTRACTIONS
            ]
            [
                self.parent.unsupportedObjects.append(obj)
                for obj in self.unsupportedObjects
            ]
            [
                self.parent.uiReceiversSymbol.append(obj)
                for obj in self.uiReceiversSymbol
            ]
            self.C_functionsDeclarationStarted = (
                self.parent.C_functionsDeclarationStarted
            )
            self.C_functionsCalledStarted = self.parent.C_functionsCalledStarted
            self.parent.addedObjects = self.addedObjects
            self.parent.templateCode = self.templateCode

        if not insideaddAbstractions:
            self.configUiReceivers()
            self.cfgDynamicLibraries()
            self.extraFunctions()
            self.saveMainFile()
            self.emccCompile()
        return True

    def activeEmcc(self):
        """
        This function will download and install emcc if it is not installed.
        """
        if not os.path.exists(self.PdWebCompilerPath + "/emsdk"):
            emccGithub = "https://api.github.com/repos/emscripten-core/emsdk/tags"
            response = requests.get(emccGithub)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            myprint("Downloading emcc...", color="green")
            with open(self.PdWebCompilerPath + "/emcc.zip", "wb") as file:
                file.write(response.content)
            with zipfile.ZipFile(self.PdWebCompilerPath + "/emcc.zip", "r") as zip_ref:
                zip_ref.extractall(self.PdWebCompilerPath)
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.PdWebCompilerPath + "/" + extractFolderName,
                    self.PdWebCompilerPath + "/emsdk",
                )
            if platform.system() == "Windows":
                os.system(f"cmd /C {self.emcc.emsdk} install latest")
                os.system(f"cmd /C {self.emcc.emsdk} activate latest")
            else:
                os.environ["EMSDK_QUIET"] = "1"
                os.system(f"chmod +x {self.emcc.emsdk}")
                os.system(f"{self.emcc.emsdk} install latest")
                os.system(f"{self.emcc.emsdk} activate latest")
                os.system(f"chmod +x {self.emcc.emsdk_env}")
        if self.args.active_emcc:
            # os.system(f"{self.emcc.emsdk} activate latest")
            os.system(f"{self.emcc.emsdk_env}")
            sys.exit(0)

    def importExternalObjs(self):
        """
        Each externals library can use extrafunctions, this function will
        these functions from the externals folders.
        """
        externalFolder = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), "externals"
        )
        module_files = [
            f
            for f in os.listdir(externalFolder)
            if f.endswith(".py") and not f.startswith("__")
        ]
        module_names = [os.path.splitext(f)[0] for f in module_files]
        for module_name in module_names:
            if module_name != "ExternalClass":
                module = importlib.import_module("pd4web.externals." + module_name)
                self.externalsExtraFunctions.append(module)

    def downloadLibPd(self):
        """
        It download and configure the libpd repository, source files and others.
        """
        if shutil.which("git") is None:
            myprint("" + "Git is not installed!", color="red")
            myprint("")
            myprint(
                "Install git using the pd4web Docs https://charlesneimog.github.io/pd4web/patch/#git",
                color="yellow",
            )
            sys.exit(0)
        if not os.path.exists(self.PdWebCompilerPath + "/libpd"):
            myprint("" + "Downloading libpd...", color="yellow")
            os.mkdir(self.PdWebCompilerPath + "/libpd")
            os.system(
                "git clone https://github.com/charlesneimog/libpd.git "
                + f"{self.PdWebCompilerPath}/libpd --recursive"
            )
            os.system(
                f"cd {self.PdWebCompilerPath}/libpd && git switch emscripten-pd54 &&"
                " git submodule init && git submodule update"
                + " && cd pure-data && git submodule init && git submodule update && git switch emscripten-pd54"
            )

    def getSupportedLibraries(self):
        """
        It reads yaml file and get all supported libraries.
        """
        global PD_LIBRARIES
        thisFile = os.path.dirname(os.path.realpath(__file__))
        externalFile = os.path.join(thisFile, "externals/Externals.yaml")
        PD_LIBRARIES = PD_SUPPORTED_EXTERNALS()
        self.DynamicLibraries = []
        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.downloadSources = supportedLibraries["DownloadSources"]
            supportedLibraries = supportedLibraries["SupportedLibraries"]
            for library in supportedLibraries:
                PdLib = PureDataExternals(library, self.PROJECT_ROOT)
                PD_LIBRARIES.add(PdLib)
                self.unsupportedObjects[library["name"]] = PdLib.unsupportedObj
        self.unsupportedObjects["puredata"] = ["bang~"]

    def enumerateExternals(self, libraryFolder, libraryName):
        """
        Recursively enumerate all external objects and save the JSON file.
        """
        externalsJson = os.path.join(self.PdWebCompilerPath, "externals.json")
        if os.path.exists(externalsJson):
            with open(externalsJson, "r") as file:
                externalsDict = json.load(file)
        else:
            externalsDict = {}
        extObjs = []
        absObjs = []
        externalsDict[libraryName] = {}
        for root, _, files in os.walk(libraryFolder):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    with open(os.path.join(root, file), "r") as c_file:
                        file_contents = c_file.read()
                        pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            extObjs.append(objectName)
                if file.endswith(".pd"):
                    if "-help.pd" not in file:
                        absObjs.append(file.split(".pd")[0])
        externalsDict[libraryName]["objs"] = extObjs
        externalsDict[libraryName]["abs"] = absObjs
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)

    def enumeratePureDataObjs(self):
        """
        It get all the PureData externals.
        """
        externalsJson = os.path.join(self.PdWebCompilerPath, "externals.json")
        if not os.path.exists(externalsJson):
            with open(externalsJson, "w") as file:
                json.dump({}, file, indent=4)
        with open(externalsJson, "r") as file:
            externalsDict = json.load(file)
        externalsDict["puredata"] = {}
        puredataObjs = []
        puredataFolder = os.path.join(self.PdWebCompilerPath, "libpd/pure-data/")
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
        # extra objects declared in another way
        # NOTE: objects that not use class_new gensym...
        puredataObjs.append("list")
        extObjs = list(set(puredataObjs))
        externalsDict["puredata"]["objs"] = extObjs
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)
        self.supportedObjects = externalsDict

    def enumerateLocalAbstractions(self):
        """
        This function list all pd patch in the same folder of the main patch.
        """
        localAbstractions = []
        if not os.path.isabs(self.args.patch):
            self.args.patch = os.path.abspath(self.args.patch)
        files = os.listdir(os.path.dirname(self.args.patch))
        for file in files:
            if file.endswith(".pd"):
                localAbstractions.append(file.split(".pd")[0])
        self.localAbstractions = localAbstractions

    def getAllSupportedObjects(self):
        """
        Get the externals objects supported.
        """
        externalsJson = os.path.join(self.PdWebCompilerPath, "externals.json")
        if os.path.exists(externalsJson):
            with open(externalsJson, "r") as file:
                self.supportedObjects = json.load(file)
        else:
            self.enumeratePureDataObjs()
            self.getAllSupportedObjects()

    def searchForSpecialObject(self, patchLine):
        """
        There is some special objects that we need extra configs.
        This function will search for these objects and add the configs.
        """
        if len(patchLine.Tokens) < 5:  # case it is array or float.
            return
        if patchLine.Tokens[4].replace("\n", "") == "clone":
            self.extraConfigClone(patchLine)

    def extraConfigClone(self, patchLine: PatchLine):
        """
        This function execute the extra config for clone object.
        """
        # from patchLine Tokens, remove 4 first Tokens
        usedTokens = patchLine.Tokens[4:]
        # TODO: Needs investigation

    def replaceVisualArray(self):
        """
        Visual arrays are not support by pd4web, this function will replace
        Visual Arrays by [array define] object.
        """
        return
        # BUG: SOLVE THIS BUG
        canvasIndex = False
        coordsIndex = False
        restoreIndex = False
        arrayLastIndex = False
        arrayFirstIndex = False
        arrayName = ""
        arrayLength = ""
        x_y_coords = {"x": "0", "y": "0"}
        for i in range(len(self.PatchFileLines)):
            LineTokens = self.PatchFileLines[i].split(" ")
            if len(LineTokens) < 7:
                continue
            if LineTokens[6] == "(subpatch)":
                canvasIndex = i
            else:
                continue
            LineTokens_Next = self.PatchFileLines[i + 1].split(" ")
            if LineTokens_Next[1] == "array":
                arrayFirstIndex = i + 1
                arrayName = LineTokens_Next[2]
                arrayLength = LineTokens_Next[3]
            j = 2
            while True:
                if self.PatchFileLines[i + j].split(" ")[0] == "#A":
                    j += 1
                else:
                    arrayLastIndex = i + j - 1
                    break
            if self.PatchFileLines[arrayLastIndex + 1].split(" ")[1] == "coords":
                coordsIndex = arrayLastIndex + 1
            if self.PatchFileLines[arrayLastIndex + 2].split(" ")[1] == "restore":
                restoreIndex = arrayLastIndex + 2
                x_y_coords["x"] = self.PatchFileLines[restoreIndex].split(" ")[2]
                x_y_coords["y"] = self.PatchFileLines[restoreIndex].split(" ")[3]
            if canvasIndex and coordsIndex and restoreIndex:
                break
        if canvasIndex and coordsIndex and restoreIndex:
            self.PatchFileLines.pop(canvasIndex)
            arrayDefine = (
                f"#X obj {x_y_coords['x']} {x_y_coords['y']} array define "
                f"{arrayName} {arrayLength};\n"
            )
            self.PatchFileLines.insert(arrayFirstIndex - 1, arrayDefine)
            self.PatchFileLines.pop(arrayFirstIndex)
            self.PatchFileLines.pop(coordsIndex - 1)
            self.PatchFileLines.pop(restoreIndex - 2)
            myprint(
                ""
                + self.args.patch
                + " has VIS array, it is not supported and was replaced by [array define]",
                color="yellow",
            )
            self.replaceVisualArray()
        with open(self.args.patch, "w") as file:
            for line in self.PatchFileLines:
                file.write(line)

    def copyLibAbstraction(self, abstractionfile):
        """
        This function copies the abstractions to webpatch/data folder.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")

        if os.path.exists(
            self.PROJECT_ROOT + "webpatch/data/" + os.path.basename(abstractionfile)
        ):
            print("Abstraction already exists: " + abstractionfile)
            return

        # shutil.copy(abstractionfile, self.PROJECT_ROOT + "webpatch/data")

    def copyAllDataFiles(self):
        """
        This function copies all files from supported folder to webpatch/data folder.
        TODO: Add support to copy folders specified by the user.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")
        for folderName in ["extra", "Extras", "Audios", "Libs", "Abstractions"]:
            if not os.path.exists(folderName):
                continue
            if folderName != "Extras":
                if os.path.exists(self.PROJECT_ROOT + "webpdPatch/data" + folderName):
                    for root, _, files in os.walk(folderName):
                        for file in files:
                            shutil.copy(
                                os.path.join(root, file),
                                self.PROJECT_ROOT + "webpatch/data",
                            )
                else:
                    shutil.copytree(
                        folderName, self.PROJECT_ROOT + "webpatch/data/" + folderName
                    )
            else:
                shutil.copytree(folderName, self.PROJECT_ROOT + "webpatch/Extras")

    def checkIfIsSupportedObject(self, patchLine: PatchLine):
        """
        Double check for unsupported objects.
        """
        pdClass = patchLine.Tokens[1]
        if (
            pdClass == "array"
        ):  # If something go wrong and the array is not replaced, we will replace it here
            myprint(
                "Visual Arrays are not supported, " + "use [array define] object",
                color="red",
            )
            sys.exit(1)
        objName = patchLine.name
        objLib = patchLine.library
        if objLib in self.unsupportedObjects:
            if objName in self.unsupportedObjects[objLib]:
                myprint(
                    "The object ["
                    + objName
                    + "] from the library '"
                    + objLib
                    + "' is not supported!, please remove it!",
                    color="red",
                )
                sys.exit(1)


    def addGuiReceivers(self, patchLine: PatchLine):
        if patchLine.Tokens[4] not in ['bng', 'tgl', 'vsl', 'hsl', 'vradio', 'hradio']: 
            return

        goodGuiObjs = {"vradio": [20, 10], "hradio": [20, 10], "vsl": [23, 12], "hsl": [23, 12], "nbx": 
                       [23, 12], "tgl": [19, 7], "bng": [19, 7]}
        
        goodArgs = goodGuiObjs[patchLine.completName][0]
        if len(patchLine.Tokens) != goodArgs:
            return

        receiverSymbolIndex = goodGuiObjs[patchLine.completName][1]
        receiverSymbol = patchLine.Tokens[receiverSymbolIndex]
        myprint(
            "GUI Receiver detected: " + receiverSymbol, color="blue"
        )
        self.uiReceiversSymbol.append(receiverSymbol)


    def findExternalsObjs(self):
        """
        This function will find all externals objects in the patch.
        """
        for line in enumerate(self.PatchFileLines):
            patchLine = PatchLine()
            patchLine.index, patchLine.completLine = line
            patchLine.isExternal = False
            patchLine.Tokens = patchLine.completLine.split(" ")
            if len(patchLine.Tokens) < 5 or patchLine.Tokens[1] != "obj":
                self.isDeclareObj(patchLine)
                self.PatchLinesProcessed.append(patchLine)
                continue
            patchLine.completName = (
                patchLine.Tokens[4].replace("\n", "").replace(";", "").replace(",", "")
            )

            self.addGuiReceivers(patchLine)

            if (
                patchLine.Tokens[0] == "#X"
                and patchLine.Tokens[1] == "obj"
                and "/" in patchLine.Tokens[4]
            ) and self.checkIfIsSlash(patchLine):
                patchLine.isExternal = True
                patchLine.library = patchLine.Tokens[4].split("/")[0]
                patchLine.name = patchLine.completName.split("/")[-1]
                patchLine.objGenSym = (
                    'class_new(gensym("' + patchLine.completName + '")'
                )
                self.isLocalAbstraction(patchLine)  # update the patchLine
                self.isLibAbstraction(patchLine)  # update the patchLine

                # Get Gui Receivers
                

            elif self.checkIfIsUniqueObj(patchLine):
                patchLine.isExternal = True
                patchLine.library = patchLine.completName
                patchLine.name = patchLine.library
                if os.path.exists(patchLine.library + ".pd"):
                    myprint("It is an abstraction", color="red")
                patchLine.objGenSym = 'gensym("' + patchLine.library + '")'
                patchLine.singleObject = True


            elif "s" == patchLine.Tokens[4] or "send" == patchLine.Tokens[4]:
                receiverSymbol = (
                    patchLine.Tokens[5]
                    .replace("\n", "")
                    .replace(";", "")
                    .replace(",", "")
                )
                if "ui_" in receiverSymbol:
                    patchLine.uiReceiver = True
                    patchLine.uiSymbol = receiverSymbol
                    self.uiReceiversSymbol.append(receiverSymbol)
                    myprint(
                        "UI Sender object detected: " + receiverSymbol, color="blue"
                    )
                patchLine.name = patchLine.completName

            # TODO: Floats and Intergers are accepted as objects

            else:
                if patchLine.completName in self.supportedObjects["puredata"]["objs"]:
                    patchLine.name = patchLine.completName
                elif patchLine.completName in self.localAbstractions:
                    patchLine.name = patchLine.completName
                    myprint("Local Abstraction: " + patchLine.name, color="green")
                else:
                    myprint("Object not found: " + patchLine.completName, color="red")
                    sys.exit(1)
            self.checkIfIsSupportedObject(patchLine)
            self.searchForSpecialObject(patchLine)
            patchLine.addToUsedObject(PD_LIBRARIES)
            self.PatchLinesExternals.append(patchLine)
            self.PatchLinesProcessed.append(patchLine)

    def checkIfIsSlash(self, line: PatchLine):
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

    def checkIfIsUniqueObj(self, patchLine):
        """
        This function check if the object has the same name as the library.
        """
        patchLine = patchLine.Tokens
        if patchLine[1] == "obj":
            nameOfTheObject = patchLine[4].replace(";", "").replace("\n", "")
            nameOfTheObject = nameOfTheObject.replace(",", "")
            if nameOfTheObject in PD_LIBRARIES.LibraryNames:
                LibraryClass = PD_LIBRARIES.get(nameOfTheObject)
                if LibraryClass is None:
                    myprint("Library not found: " + nameOfTheObject, color="red")
                    return False
                if LibraryClass and LibraryClass.singleObject:
                    return True
        return False

    def isDeclareObj(self, patchLine: PatchLine):
        """
        This function will check if the current line is a declare.
        """
        tokens = patchLine.Tokens
        if tokens[0] == "#X" and tokens[1] == "declare" and tokens[2] == "-lib":
            libName = tokens[3].replace("\n", "").replace(";", "").replace(",", "")
            self.declaredLibraries.append(libName)

    def isLocalAbstraction(self, patchLine: PatchLine):
        """
        It searches for local abstractions that initially will be detected as externals.
        """
        absPath = (
            self.PROJECT_ROOT + "/" + patchLine.library + "/" + patchLine.name + ".pd"
        )
        if os.path.exists(absPath):
            patchLine.isAbstraction = True
            patchLine.isLocalAbstraction = True
            patchLine.objFound = True
            patchLine.isExternal = False
            return True
        return False

    def isLibAbstraction(self, patchLine):
        """
        It searches for library abstractions, very common in the else library for example.
        """
        libraryName = patchLine.library
        objName = patchLine.name
        libPath = self.PdWebCompilerPath + "/.externals/" + libraryName
        if not os.path.exists(libPath):
            return False
        for root, _, files in os.walk(libPath):
            for file in files:
                if file.endswith(".pd") and objName == file.split(".pd")[0]:
                    # check if file already in webpatch/data
                    if os.path.exists(
                        self.PROJECT_ROOT + "webpatch/data/" + file
                    ) or os.path.exists(
                        self.PROJECT_ROOT + "webpatch/data/" + file + ".pd"
                    ):
                        pass
                    else:
                        shutil.copy(
                            os.path.join(root, file),
                            self.PROJECT_ROOT + "/webpatch/data/",
                        )
                    patchLine.isAbstraction = True
                    patchLine.objFound = True
                    patchLine.isExternal = False
                    patchLine.library = libraryName
                    patchLine.name = objName
                    externalSpace = 7 - len(patchLine.name)
                    absName = patchLine.name + (" " * externalSpace)
                    myprint(
                        f"Found Abstraction: {absName}  | Lib: {patchLine.library}",
                        color="green",
                    )
                    return True
        return False

    def cfgExternalThing(self):
        for lineInfo in self.PatchLinesExternals:
            if lineInfo.isExternal:
                foundLibrary = self.downloadExternalLibrarySrc(lineInfo.library)
                if foundLibrary:
                    for root, _, files in os.walk(
                        self.PdWebCompilerPath + "/.externals/" + lineInfo.library
                    ):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.searchCFunction(lineInfo, root, file)
                            elif file.endswith(".pd"):
                                if lineInfo.name == file.split(".pd")[0]:
                                    lineInfo.isAbstraction = True
                                    lineInfo.objFound = True
                                    self.copyLibAbstraction(os.path.join(root, file))
                else:
                    lineInfo.objFound = False
                    myprint("Could not find " + lineInfo.library, color="red")
                if lineInfo.objFound and lineInfo.isAbstraction:
                    externalSpace = 7 - len(lineInfo.name)
                    absName = lineInfo.name + (" " * externalSpace)
                    myprint(
                        f"Found Abstraction: {absName}  | Lib: {lineInfo.library}",
                        color="green",
                    )

                elif lineInfo.objFound and not lineInfo.isAbstraction:
                    externalSpace = 10 - len(lineInfo.name)
                    objName = lineInfo.name + (" " * externalSpace)
                    myprint(
                        f"Found External: {objName}  | Lib: {lineInfo.library}",
                        color="green",
                    )
                else:
                    myprint("Could not find " + lineInfo.name, color="red")

    def searchCFunction(self, lineInfo, root, file):
        """
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        """
        functionName = lineInfo.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e")  # else use . as 0x2e
        self.regexSearch(lineInfo, functionName, os.path.join(root, file))
        if not lineInfo.objFound:
            functionName = lineInfo.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.regexSearch(lineInfo, functionName, os.path.join(root, file))

    def regexSearch(self, lineInfo: PatchLine, functionName, file):
        """
        This search for the setup function using regex.
        """
        with open(
            file,
            "r",
            encoding="utf-8",
            errors="ignore",
        ) as C_file:
            try:
                file_contents = C_file.read()
            except Exception as e:
                myprint("Could not read file: " + file + " using utf-8", color="red")
                myprint(str(e), color="red")
                sys.exit(1)
            patterns = [r"void\s*{}\s*\(\s*void\s*\)", r"void\s+{}\s*\(\s*\)"]
            for pattern in patterns:
                pattern = pattern.format(re.escape(functionName))
                matches = re.finditer(pattern, file_contents, re.DOTALL)
                listMatches = list(matches)
                if len(listMatches) > 0:
                    shutil.copy(C_file.name, self.PROJECT_ROOT + "webpatch/externals")
                    lineInfo.objFound = True
                    lineInfo.functionName = functionName
                    if lineInfo.library not in self.externalsDict:
                        self.externalsDict[lineInfo.library] = [C_file.name]
                    else:
                        self.externalsDict[lineInfo.library].append(C_file.name)

    def addObjSetup(self):
        """
        This function will add the obj_setup() inside the main.c file
        """
        for lineInfo in self.PatchLinesExternals:
            if lineInfo.functionName not in self.addedObjects:
                self.addedObjects.append(lineInfo.functionName)
                if lineInfo.isExternal and lineInfo.objFound:
                    start_index = None
                    for i, line in enumerate(self.templateCode):
                        if "#include <z_libpd.h>" in line:
                            start_index = i + 1
                        if start_index is not None:
                            functionName = "void " + lineInfo.functionName + "(void);\n"
                            if self.C_functionsDeclarationStarted == False:
                                functionName += "\n"
                                self.C_functionsDeclarationStarted = True
                            self.templateCode.insert(start_index + 1, functionName)
                            break
                    start_index = None
                    for i, line in enumerate(self.templateCode):
                        if "libpd_init();" in line:
                            start_index = i + 1
                        if start_index is not None:
                            functionName = lineInfo.functionName
                            functionName = "  " + functionName + "();\n"
                            if self.C_functionsCalledStarted == False:
                                functionName += "\n"
                                self.C_functionsCalledStarted = True
                            self.templateCode.insert(start_index + 1, functionName)
                            break

    def configUiReceivers(self):
        """
        This function add ui receivers to save data in the window.pd4webUiValues

        """
        threadMutexIndex = None
        for i, line in enumerate(self.templateCode):
            if "pthread_mutex_t WriteReadMutex = PTHREAD_MUTEX_INITIALIZER" in line:
                threadMutexIndex = i
                break

        if threadMutexIndex is not None:
            lenUIReceiver = len(self.uiReceiversSymbol)
            self.templateCode.insert(threadMutexIndex + 1, "")
            htmlIds = "char* HTML_IDS[] = {"
            for i, uiReceiver in enumerate(self.uiReceiversSymbol):
                if i == lenUIReceiver - 1:
                    htmlIds += '"' + uiReceiver + '"'
                else:
                    htmlIds += '"' + uiReceiver + '", '
            htmlIds += "};\n"
            self.templateCode.insert(threadMutexIndex + 2, htmlIds)
            self.templateCode.insert(
                threadMutexIndex + 3,
                "int HTML_IDS_SIZE = " + str(lenUIReceiver) + ";\n",
            )
        return True

    def saveMainFile(self):
        with open(self.PROJECT_ROOT + "webpatch/main.c", "w") as file:
            for line in self.templateCode:
                file.write(line)

    def usedLibraries(self, libraryName):
        """
        It adds the used libraries for the patch, it can be accessed by the extra functions.
        """
        if libraryName not in PD_LIBRARIES.UsedLibrariesNames:
            PD_LIBRARIES.UsedLibrariesNames.append(libraryName)
            PD_LIBRARIES.UsedLibraries.append(PD_LIBRARIES.get(libraryName))

    def downloadExternalLibrarySrc(self, libraryName):
        responseJson = {"message": "Unknown error"}
        if libraryName in PD_LIBRARIES.LibraryNames:
            try:
                self.usedLibraries(libraryName)
                LibraryClass = PD_LIBRARIES.get(libraryName)
                if LibraryClass is None:
                    myprint("Could not find " + libraryName, color="red")
                    sys.exit(-1)
                LibraryClass.PROJECT_ROOT = self.PROJECT_ROOT
                if os.path.exists(
                    os.path.join(self.PdWebCompilerPath + "/.externals/" + libraryName)
                ):
                    LibraryClass.folder = os.path.join(
                        self.PdWebCompilerPath + "/.externals/" + libraryName
                    )
                    return True
                GithutAPI = PD_LIBRARIES.getDownloadURL(
                    LibraryClass, self.downloadSources
                )
                myprint("Downloading " + libraryName, color="yellow")
                if GithutAPI is None:
                    myprint("LibURL is not a string or None", color="red")
                    sys.exit(-1)
                elif GithutAPI == False:  # means that is a direct link
                    response = requests.get(LibraryClass.directLink)
                elif isinstance(GithutAPI, str):  # is a GithubAPI link
                    response = requests.get(GithutAPI)
                    responseJson = response.json()
                    sourceCodeLink = responseJson[0]["zipball_url"]
                    response = requests.get(sourceCodeLink)
                else:
                    myprint(
                        "The link of the srcs of " + libraryName + " is not valid",
                        color="red",
                    )
                    sys.exit(-1)
                if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
                    os.mkdir(self.PdWebCompilerPath + "/.externals")

                with open(
                    self.PdWebCompilerPath + "/.externals/" + libraryName + ".zip", "wb"
                ) as file:
                    file.write(response.content)

                with zipfile.ZipFile(
                    self.PdWebCompilerPath + "/.externals/" + libraryName + ".zip", "r"
                ) as zip_ref:
                    zip_ref.extractall(self.PdWebCompilerPath + "/.externals")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(
                        self.PdWebCompilerPath + "/.externals/" + extractFolderName,
                        self.PdWebCompilerPath + "/.externals/" + libraryName,
                    )

                LibraryClass.folder = os.path.join(
                    os.getcwd(), self.PdWebCompilerPath + "/.externals/" + libraryName
                )
                self.librariesFolder.append(
                    os.path.join(
                        os.getcwd(),
                        self.PdWebCompilerPath + "/.externals/" + libraryName,
                    )
                )
                os.remove(
                    self.PdWebCompilerPath + "/.externals/" + libraryName + ".zip"
                )
                self.enumerateExternals(LibraryClass.folder, libraryName)
                self.enumeratePureDataObjs()
                self.getAllSupportedObjects()
                return True
            except Exception as e:
                myprint("" + str(e), color="red")
                myprint("" + str(responseJson["message"]), color="red")
                myprint("" + str(e), color="red")
                sys.exit(1)
        else:
            return False

    def mkBackup(self):
        """
        This function makes a backup of the patch file.
        """
        # delete all files in .backup folder created more than 3 day ago
        for root, _, files in os.walk(self.PROJECT_ROOT + "/.backup"):
            for file in files:
                file = os.path.join(root, file)
                if os.path.getmtime(file) < time.time() - 3 * 86400:
                    os.remove(file)
        if not os.path.exists(self.PROJECT_ROOT + "/.backup"):
            os.mkdir(self.PROJECT_ROOT + "/.backup")
        Hour = datetime.datetime.now().hour
        Minute = datetime.datetime.now().minute
        Day = datetime.datetime.now().day
        patchName = os.path.basename(self.args.patch)
        patchName = patchName.split(".pd")[0]
        backPatchName = (
            patchName + "-" + str(Day) + "-" + str(Hour) + "-" + str(Minute) + ".pd"
        )
        backPatchPath = self.PROJECT_ROOT + ".backup/" + backPatchName
        if platform.system() == "Windows":
            backPatchPath = backPatchPath.replace("/", "\\")
        shutil.copy(self.args.patch, backPatchPath)

    def extraFunctions(self):
        """
        This function try to execute extra functions for the library.
        In this function, there is headers configurations, extra flags and
        others.
        """
        print("")
        for usedLibrary in PD_LIBRARIES.UsedLibraries:
            usedLibrary.webpdPatch = self
            if usedLibrary.name in self.externalsDict:
                usedLibrary.UsedSourceFiles = self.externalsDict[usedLibrary.name]

            if usedLibrary.extraFuncExecuted == True:
                continue
            usedLibrary.externalsExtraFunctions = self.externalsExtraFunctions
            extraFlags = PD_LIBRARIES.executeExtraFunction(usedLibrary)
            if extraFlags is not None:
                for flag in extraFlags:
                    self.extraFlags.append(flag)

    def removeLibraryPrefix(self, patchfile, patchLines: List[PatchLine]):
        """
        This function remove the library prefix from the patch file: else/counter => counter.
        because after the compilation, all the externals become embedded objects.
        """
        patchWithoutPrefix = []
        for line in patchLines:
            if (
                not len(line.Tokens) < 5
                and "/" in line.Tokens[4]
                and not line.objwithSlash
            ):
                if (
                    line.isAbstraction or line.isExternal
                ) and not line.isLocalAbstraction:
                    line.Tokens[4] = line.Tokens[4].split("/")[1]
                    patchWithoutPrefix.append(line.Tokens)
                else:
                    patchWithoutPrefix.append(line.Tokens)
            else:
                patchWithoutPrefix.append(line.Tokens)

        with open(patchfile, "w") as file:
            for line in patchWithoutPrefix:
                file.write(" ".join(line))

    def savePdPatchModified(self):
        """
        After remove all the prefix, and others things (like remove visual arrays),
        we save the patch file that will be used by the website.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")
        if self.insideaddAbstractions:
            PatchFile = self.args.patch
        else:
            PatchFile = self.PROJECT_ROOT + "webpatch/data/index.pd"

        if "webpatch" not in PatchFile:
            return

        with open(PatchFile, "w") as file:
            finalPatch = []
            for obj in self.PatchLinesExternals:
                if obj.isExternal and not obj.singleObject and not obj.isAbstraction:
                    patchLine = obj.completLine
                    patchLineList = patchLine.split(" ")
                    patchLineList[4] = patchLineList[4].split("/")[1]
                    file.write(" ".join(patchLineList))
                else:
                    finalPatch.append(obj.completLine)

    def processAbstractions(self):
        """
        The function process libraries functions Abstractions of libraries.
        """
        before_files = os.listdir(self.PROJECT_ROOT + "webpatch/data")
        for dir, _, files in os.walk(self.PROJECT_ROOT + "webpatch/data"):
            for patchfile in files:
                if patchfile.endswith(".pd") and patchfile != "index.pd":
                    if patchfile not in self.PROCESSED_ABSTRACTIONS:
                        abstraction = webpdPatch(
                            sourcefile=self.PROJECT_ROOT + "webpatch/main.c",
                            pdpatch=self.PROJECT_ROOT + "webpatch/data/" + patchfile,
                            insideaddAbstractions=True,
                            runMain=True,
                            parent=self,
                        )
                        patchPath = dir + "/" + patchfile
                        self.removeLibraryPrefix(
                            patchPath, abstraction.PatchLinesProcessed
                        )
                        self.PROCESSED_ABSTRACTIONS.append(patchfile)
        after_files = os.listdir(self.PROJECT_ROOT + "webpatch/data")
        if before_files == after_files:
            return
        self.processAbstractions()

    def cfgDynamicLibraries(self):
        """
        Configures dynamic libraries for compilation.

        This method iterates through the list of used libraries in the `PD_LIBRARIES.UsedLibraries`
        and checks if they require dynamic libraries specified in Externals.yaml. If dynamic libraries are required, it attempts
        to locate and configure them using the supported libraries in `DYNAMIC_LIBRARIES`.

        This method provides a way to configure dynamic libraries necessary for building a project
        with external dependencies.
        """
        for library in PD_LIBRARIES.UsedLibraries:
            requiredLibraries = library.requireDynamicLibraries
            if requiredLibraries != False:
                for dyn_library in requiredLibraries:
                    try:
                        function = DYNAMIC_LIBRARIES[dyn_library]
                        function(self)  # call the function
                    except Exception as e:
                        myprint("Could not find " + dyn_library, color="red")
                        myprint("" + str(e), color="red")
                        sys.exit(1)

    def emccCompile(self):
        """
        This function create the emcc command, run it and if the user passed
        the --server-port flag, it will run the server.
        """
        self.removeLibraryPrefix(
            self.PROJECT_ROOT + "webpatch/data/index.pd", self.PatchLinesProcessed
        )
        memory = self.memory
        if platform.system() == "Windows":
            self.target = self.PROJECT_ROOT + "webpatch\\libpd.js"
            self.libpd_dir = self.PdWebCompilerPath + "\\libpd"
            self.src_files = self.PROJECT_ROOT + "webpatch\\main.c"
            os.chdir(self.PROJECT_ROOT)
            emcc = self.emcc.emcc
            command = [
                emcc,
                "-I ",
                self.PROJECT_ROOT + "webpatch\\includes\\",
                "-I ",
                self.libpd_dir + "\\pure-data\\src\\",
                "-I ",
                self.libpd_dir + "\\libpd_wrapper\\",
                "-L ",
                self.PdWebCompilerPath + "\\lib\\compiled\\",
                "-DPD",
                "-lpd",
                "-O3",
                "-s",
                f"INITIAL_MEMORY={memory}mb",
                "-s",
                "AUDIO_WORKLET=1",
                "-s",
                "WASM_WORKERS=1",
                "-s",
                "WASM=1",
                "-s",
                "USE_PTHREADS=1",
                "--preload-file",
                "webpatch\\data\\",
            ]
        else:
            self.target = self.PROJECT_ROOT + "webpatch/libpd.js"
            self.libpd_dir = self.PdWebCompilerPath + "/libpd"
            self.src_files = self.PROJECT_ROOT + "webpatch/main.c"
            emcc = self.PdWebCompilerPath + "/emsdk/upstream/emscripten/emcc"
            os.chdir(self.PROJECT_ROOT)
            command = [
                emcc,
                "-I",
                self.PROJECT_ROOT + "webpatch/includes/",
                "-I",
                self.libpd_dir + "/pure-data/src/",
                "-I",
                self.libpd_dir + "/libpd_wrapper/",
                "-L",
                self.PdWebCompilerPath + "/lib/compiled/",
                "-DPD",
                "-lpd",
                "-O3",
                "-s",
                f"INITIAL_MEMORY={memory}mb",
                # '-s', 'ALLOW_MEMORY_GROWTH=1', # TODO: wait to solve problem
                "-s",
                "AUDIO_WORKLET=1",
                "-s",
                "WASM_WORKERS=1",
                "-s",
                "WASM=1",
                "-s",
                "USE_PTHREADS=1",
                "--preload-file",
                "webpatch/data/",
            ]
        indexFlag = 0
        for flag in self.extraFlags:
            command.insert(11 + indexFlag, flag)
            indexFlag += 1
        for root, _, files in os.walk(self.PROJECT_ROOT + "webpatch/externals"):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    sourcefile = os.path.join(root, file)
                    if platform.system() == "Windows":
                        sourcefile = sourcefile.replace("/", "\\")
                    command.append(sourcefile)
        for source in self.sortedSourceFiles:
            if platform.system() == "Windows":
                source = source.replace("/", "\\")
            command.append(source)
        command.append(self.src_files)
        command.append("-o")
        command.append(self.target)
        commandToPrint = command.copy()
        for line in enumerate(commandToPrint):
            commandToPrint[line[0]] = line[1].replace(self.PdWebCompilerPath, "")
            commandToPrint[line[0]] = line[1].replace(self.PROJECT_ROOT, "")
        print("")
        myprint(" ".join(commandToPrint), color="light_grey")
        print("")
        if platform.system() == "Windows":
            command.insert(0, "/C")
            command.insert(0, "cmd")
            command = " ".join(command)
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )
        _, stderr = process.communicate()
        error = False
        if isinstance(stderr, str):
            stderrTOKENS = stderr.lower().split("\n")
            for key in stderrTOKENS:
                if "warning:" in key:
                    print("")
                    myprint(key, color="yellow")
                elif "error" in key and isinstance(key, str):
                    error = True
                    print("")
                    myprint(key, color="red")
                else:
                    myprint(key)
            if error:
                myprint("There was an error compiling, READ the output", color="red")
                sys.exit(1)
            else:
                myprint(
                    ("=" * 10) + " Compiled with success " + ("=" * 10) + "\n",
                    color="green",
                )

        process.wait()
        if isinstance(self.html, str):
            shutil.copy(self.html, self.PROJECT_ROOT + "webpatch")
        if self.args.server_port:
            myprint(
                "Starting server on port " + str(self.args.server_port), color="green"
            )
            emrun = (
                self.PdWebCompilerPath
                + f"/emsdk/upstream/emscripten/emrun --port {self.args.server_port} "
            )
            if self.args.no_browser:
                emrun += "--no_browser "
            emrun += " . "
            os.system(emrun)
        sys.exit(0)
