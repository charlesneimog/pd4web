import os
import platform
import sys
import subprocess
import argparse
import requests
import zipfile
import shutil
import datetime
import yaml
import re
from .externals.ExternalClass import PD_SUPPORTED_EXTERNALS, PureDataExternals, PatchLine
from .lib.main import DYNAMIC_LIBRARIES

## ================== EXTERNALS THINGS ================== ##

PROCESSED_ABSTRACTIONS = []


class webpdPatch():
    def __init__(self, sourcefile="src/template.c", pdpatch=None,
                 insideaddAbstractions=False, runMain=True) -> None:
        self.PdWebCompilerPath = os.path.dirname(os.path.realpath(__file__))

        if runMain and  not insideaddAbstractions:
            if os.path.exists("webpatch"):
                shutil.rmtree("webpatch")


        if not insideaddAbstractions:
            self.downloadEmcc()
            self.downloadLibPd()

        # get this folder directory
        parser = argparse.ArgumentParser(
            formatter_class=argparse.RawTextHelpFormatter, description="Check the complete docs in https://www.charlesneimog.com/PdWebCompiler")
        parser.add_argument('--patch', required=True,
                            help='Patch file (.pd) to compile')
        parser.add_argument(
            '--html',
            required=False,
            help='HTML used to load and render the Web Page. If not defined, we use the default one')
        parser.add_argument('--confirm', required=False,
                            help='There is some automatic way check if the external is correct, but it is not always accurate. If you want to confirm if the external is correct, use this flag')
        parser.add_argument('--clearTmpFiles', required=False,
                            default=False, help='Remove all TempFiles, like .externals folder')

        parser.add_argument('--server-port', required=False,
                            default=False, help='Set the port to start the server')

        parser.add_argument('--initial-memory', required=False,
                            default=32, help='Set the initial memory of the WebAssembly in MB')
        
        # parser.add_argument('--gui', required=False,
                            # default=False, help='Set the port to start the server')

        parser.add_argument('--version', action='version',
                            version='%(prog)s 1.0.8')

        self.args = parser.parse_args()

        self.FoundExternals = False
        self.html = False
        self.source = sourcefile
        self.clearTmpFiles = self.args.clearTmpFiles
        self.uiReceiversSymbol = []
        self.insideaddAbstractions = insideaddAbstractions
        self.lastPrintedLine = ""
        self.memory = self.args.initial_memory
        self.extraFlags = []
        self.externalsDict = {}

        if runMain:
            self.main(
                pdpatch=pdpatch,
                insideaddAbstractions=insideaddAbstractions)

        else:
            myinput = input("Do you want to compile the patch? [Y/n]: ")
            if myinput == "Y" or myinput == "y":
                self.main(
                    pdpatch=pdpatch,
                    insideaddAbstractions=insideaddAbstractions)
            else:
                print("Bye Bye!")


    def main(self, pdpatch=None, insideaddAbstractions=False):
        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = self.args.patch

        # patch file name
        patchFileName = os.path.basename(self.patch)
        self.print("\n    • Patch => " + patchFileName + "\n", color='cyan')
        if self.args.html is not None:
            if not os.path.isabs(self.args.html) and not insideaddAbstractions:
                absolutePath = os.path.dirname(os.path.abspath(
                    os.path.join(os.getcwd(), self.args.html)))
                self.html = os.getcwd() + "/" + self.args.html
        else:

            self.html = self.PdWebCompilerPath + "/src/index.html"

        if not os.path.isabs(self.patch) and not insideaddAbstractions:
            absolutePath = os.path.dirname(os.path.abspath(
                os.path.join(os.getcwd(), self.patch)))
            self.patch = os.getcwd() + "/" + self.patch
            self.source = os.getcwd() + "/" + self.source
            self.PROJECT_ROOT = absolutePath
            os.chdir(absolutePath)

        else:
            self.PROJECT_ROOT = os.getcwd()

        print(self.PROJECT_ROOT + "/webpatch")

        # if self.patch not exist look recursively in subfolders
        if not os.path.exists(self.patch):
            notFound = True
            # look inside subfolders of the PROJECT_ROOT
            for root, _, files in os.walk(self.PROJECT_ROOT):
                for file in files:
                    if not file.endswith(".pd"):
                        continue

                    if file != patchFileName:
                        continue

                    self.patch = os.path.join(root, file)
                    notFound = False
                    break
            
                
            if notFound:
                self.print("    " + "Patch not found", color='red')
                sys.exit(-1)


        with open(self.patch, "r") as file:
            self.PatchLines = file.readlines()

        self.replaceVISArray()

        # read externals
        self.getSupportedLibraries()
        # ==============

        self.PROJECT_ROOT = os.getcwd()
        self.processedAbstractions = []


        # template code
        if not insideaddAbstractions:
            with open(os.path.join(self.PdWebCompilerPath, "src/template.c"), "r") as file:
                self.templateCode = file.readlines()
        else:
            with open("webpatch/main.c", "r") as file:
                self.templateCode = file.readlines()

        if not insideaddAbstractions:
            if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
                os.mkdir(self.PdWebCompilerPath + "/.externals")

            if not os.path.exists("webpatch"):
                os.mkdir("webpatch")
            else:
                shutil.rmtree("webpatch")
                os.mkdir("webpatch")

            if not os.path.exists("webpatch/externals"):
                os.mkdir("webpatch/externals")
            else:
                shutil.rmtree("webpatch/externals")
                os.mkdir("webpatch/externals")

            if not os.path.exists("webpatch/includes"):
                os.mkdir("webpatch/includes")
            else:
                shutil.rmtree("webpatch/includes")
                os.mkdir("webpatch/includes")

        self.librariesFolder = []
        self.confirm = self.args.confirm
        self.getPatchPath()
        self.mkBackup()
        self.PatchLinesExternalFound = []
        self.findExternals()
        self.cfgExternals()
        self.addObjSetup()
        
        self.savePdPatchModified()

        self.saveMainFile()
        self.extraFunctions()
        if not insideaddAbstractions:
            self.copyAllDataFiles()
            self.addAbstractions()

        shutil.copy(self.PdWebCompilerPath +
                    "/src/index.html", "webpatch/index.html")
        shutil.copy(self.PdWebCompilerPath +
                    "/src/helpers.js", "webpatch/helpers.js")
        shutil.copy(self.PdWebCompilerPath +
                    "/src/enable-threads.js", "webpatch/enable-threads.js")

        self.getDynamicLibraries()

        if not insideaddAbstractions:
            self.emccCompile()

        if not insideaddAbstractions:
            print("")

        return True

    def downloadEmcc(self):
        if not os.path.exists(self.PdWebCompilerPath + "/emsdk"):
            emccGithub = "https://api.github.com/repos/emscripten-core/emsdk/tags"
            response = requests.get(emccGithub)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            self.print("    Downloading emcc...", color="green")
            with open(self.PdWebCompilerPath + "/emcc.zip", "wb") as file:
                file.write(response.content)

            with zipfile.ZipFile(self.PdWebCompilerPath + "/emcc.zip", 'r') as zip_ref:
                zip_ref.extractall(self.PdWebCompilerPath)
                extractFolderName = zip_ref.namelist()[0]
                os.rename(
                    self.PdWebCompilerPath +
                    "/" +
                    extractFolderName,
                    self.PdWebCompilerPath +
                    "/emsdk")

            if platform.system() == "Windows":
                subprocess.run([self.PdWebCompilerPath +
                                "/emsdk/emsdk.bat", "install", "latest"])
                subprocess.run([self.PdWebCompilerPath +
                                "/emsdk/emsdk.bat", "activate", "latest"])
            else:
                os.environ["EMSDK_QUIET"] = "1"
                os.system(
                    "chmod +x " +
                    self.PdWebCompilerPath +
                    "/emsdk/emsdk")
                os.system(
                    self.PdWebCompilerPath +
                    "/emsdk/emsdk install latest")
                os.system(
                    self.PdWebCompilerPath +
                    "/emsdk/emsdk activate latest")

                os.system(
                    "chmod +x " +
                    self.PdWebCompilerPath +
                    "/emsdk/emsdk_env.sh")

        if platform.system() == "Windows":
            subprocess.run(["set", "EMSDK_QUIET=1"], shell=True)
            subprocess.run([self.PdWebCompilerPath +
                           "/emsdk/emsdk_env.bat"], shell=True)
        else:
            os.environ["EMSDK_QUIET"] = "1"
            subprocess.run([self.PdWebCompilerPath +
                           "/emsdk/emsdk_env.sh"], shell=True)

    def downloadLibPd(self):
        # see if Git is installed
        if shutil.which("git") is None:
            self.print("    " + "Git is not installed", color='red')
            if platform.system() == 'Windows':
                # os.system("start https://git-scm.com/download/win")
                self.print("")
                self.print(
                    "    " +
                    "Download Git from https://git-scm.com/download/win",
                    color='yellow')
                sys.exit(-1)
            elif platform.system() == 'Linux':
                self.print("")
                self.print(
                    "    " +
                    "Download Git from https://git-scm.com/download/linux",
                    color='yellow')
                sys.exit(-1)
            elif platform.system() == 'Darwin':
                self.print("")
                os.system("git --version")
                self.print(
                    "    " +
                    "Download Git from https://git-scm.com/download/mac",
                    color='yellow')
                sys.exit(-1)

            else:
                self.print(
                    "    " +
                    "Git is not installed, install it to proceed",
                    color='red')
                sys.exit(-1)

        if not os.path.exists(self.PdWebCompilerPath + "/libpd"):
            self.print("    " + "Downloading libpd...", color='yellow')
            os.mkdir(self.PdWebCompilerPath + "/libpd")
            os.system("git clone https://github.com/charlesneimog/libpd.git " +
                      f"{self.PdWebCompilerPath}/libpd --recursive")
            os.system(f"cd {self.PdWebCompilerPath}/libpd && git switch emscripten-pd54 &&"
                      " git submodule init && git submodule update" +
                      " && cd pure-data && git submodule init && git submodule update && git switch emscripten-pd54")

    def getValue(self, dictionary, key):
        if key in dictionary:
            return dictionary[key]
        else:
            return ''

    def getSupportedLibraries(self):
        ''' Read yaml file and get all supported libraries '''
        global PD_LIBRARIES
        thisFile = os.path.dirname(os.path.realpath(__file__))
        externalFile = os.path.join(thisFile, "Externals.yaml")
        PD_LIBRARIES = PD_SUPPORTED_EXTERNALS()
        self.DynamicLibraries = []

        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.downloadSources = supportedLibraries['DownloadSources']
            supportedLibraries = supportedLibraries['SupportedLibraries']
            for library in supportedLibraries:
                PdLib = PureDataExternals(library)
                PD_LIBRARIES.add(PdLib)

    def print(self, str, color=None):
        if color is None:
            print("    " + str)
            return

        if color == 'red':
            print("\033[91m" + str + "\033[0m")
        elif color == 'green':
            print("\033[92m" + str + "\033[0m")
        elif color == 'yellow':
            print("\033[93m" + str + "\033[0m")
        elif color == 'blue':
            print("\033[94m" + str + "\033[0m")
        elif color == 'magenta':
            print("\033[95m" + str + "\033[0m")
        elif color == 'cyan':
            print("\033[96m" + str + "\033[0m")
        else:
            print(str)

    def searchForSpecialObject(self, line):
        if len(line.Tokens) < 5:
            return

        if line.Tokens[4].replace("\n", "") != "clone":
            return

        for token in line.Tokens:
            if not "-" in token:
                absName = token.replace(",", "") + ".pd"
                for root, _, files in os.walk(self.PdWebCompilerPath):
                    for file in files:
                        if not file.endswith(".pd") and absName not in file:
                            continue

                        if absName != file:
                            continue

                        if not os.path.exists("webpatch/data/" + absName):
                            self.print("    " + "Copying " +
                                       absName + " to webpatch/data", color='yellow')
                            shutil.copy(
                                os.path.join(
                                    root, file), "webpatch/data")

    def replaceVISArray(self):
        canvasIndex = False
        coordsIndex = False
        restoreIndex = False
        arrayLastIndex = False
        arrayFirstIndex = False
        arrayName = ''
        arrayLength = ''
        x_y_coords = {'x': '0', 'y': '0'}
        for i in range(len(self.PatchLines)):
            LineTokens = self.PatchLines[i].split(" ")
            if len(LineTokens) < 7:
                continue
            if LineTokens[6] == "(subpatch)":
                canvasIndex = i
            else:
                continue
            LineTokens_Next = self.PatchLines[i + 1].split(" ")
            if LineTokens_Next[1] == "array":
                arrayFirstIndex = i + 1
                arrayName = LineTokens_Next[2]
                arrayLength = LineTokens_Next[3]

            j = 2
            while True:
                if self.PatchLines[i + j].split(" ")[0] == "#A":
                    j += 1
                else:
                    arrayLastIndex = i + j - 1
                    break

            if self.PatchLines[arrayLastIndex + 1].split(" ")[1] == "coords":
                coordsIndex = arrayLastIndex + 1
            if self.PatchLines[arrayLastIndex + 2].split(" ")[1] == "restore":
                restoreIndex = arrayLastIndex + 2
                x_y_coords['x'] = self.PatchLines[restoreIndex].split(" ")[2]
                x_y_coords['y'] = self.PatchLines[restoreIndex].split(" ")[3]

            if canvasIndex and coordsIndex and restoreIndex:
                break

        if canvasIndex and coordsIndex and restoreIndex:
            # from self.PatchLines, remove canvasIndex value
            self.PatchLines.pop(canvasIndex)
            arrayDefine = f"#X obj {x_y_coords['x']} {x_y_coords['y']} array define " \
                f"{arrayName} {arrayLength};\n"
            self.PatchLines.insert(arrayFirstIndex - 1, arrayDefine)
            self.PatchLines.pop(arrayFirstIndex)
            self.PatchLines.pop(coordsIndex - 1)
            self.PatchLines.pop(restoreIndex - 2)
            self.print("    " + self.patch +
                       " has VIS array, it is not supported and was replaced by [array define]", color='yellow')
            self.replaceVISArray()

        # save self.PatchLines to file
        with open(self.patch, "w") as file:
            for line in self.PatchLines:
                file.write(line)

    def configForAbstraction(self, abstractionfile):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")
        shutil.copy(abstractionfile, "webpatch/data")

    def copyAllDataFiles(self):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")

        for folderName in ["extra", "Extras", "Audios", "libs", "Abstractions"]:
            if not os.path.exists(folderName):
                continue

            if folderName != "Extras":
                shutil.copytree(folderName, "webpatch/data/" + folderName)
            else:
                shutil.copytree(folderName, "webpatch/Extras")

            # for root, _, files in os.walk(folderName):
                # for file in files:
                    # shutil.copy(os.path.join(root, file), "webpatch/data")


    def checkIfIsSupportedObject(self, patchLine):
        pdClass = patchLine[1]
        if pdClass == "array":
            self.print("    " +
                       "Visual Arrays are not supported, use [array define] object", color='red')

    def findExternals(self):
        for line in enumerate(self.PatchLines):
            patchLine = PatchLine()
            patchLine.patchLineIndex = line[0]
            patchLine.patchLine = line[1]
            patchLine.isExternal = False
            patchLine.Tokens = patchLine.patchLine.split(" ")
            # lineArgs = patchLine.patchLine.split(" ")
            if len(patchLine.Tokens) < 5:
                continue

            # here for especial objects (externals, not supported objects,
            # abstractions)
            objName = patchLine.Tokens[4].replace(
                "\n", "").replace(";", "").replace(",", "")

            self.checkIfIsSupportedObject(patchLine.Tokens)

            if (patchLine.Tokens[0] == "#X" and patchLine.Tokens[1] == "obj"
                    and "/" in patchLine.Tokens[4]) and objName != "/":
                patchLine.isExternal = True
                patchLine.library = patchLine.Tokens[4].split("/")[0]
                patchLine.name = patchLine.Tokens[4].split(
                    "/")[1].replace("\n", "").replace(";", "").replace(",", "")
                patchLine.objGenSym = 'class_new(gensym("' + patchLine.Tokens[4].split("/")[
                    1].replace("\n", "").replace(";", "") + '")'

            elif self.checkIsObjIsSingle(patchLine.Tokens):
                patchLine.isExternal = True
                patchLine.library = patchLine.Tokens[4].replace(
                    ";", "").replace("\n", "").replace(",", "")
                patchLine.name = patchLine.library
                patchLine.objGenSym = 'gensym("' + patchLine.library + '")'
                patchLine.singleObject = True

            elif ("s" == patchLine.Tokens[4] or "send" == patchLine.Tokens[4]):
                receiverSymbol = patchLine.Tokens[5].replace(
                    "\n", "").replace(";", "").replace(",", "")
                if ("ui_" in receiverSymbol):
                    patchLine.uiReceiver = True
                    patchLine.uiSymbol = receiverSymbol
                    self.uiReceiversSymbol.append(receiverSymbol)
                patchLine.name = patchLine.Tokens[4].replace(
                    ";", "").replace("\n", "")

            else:
                patchLine.name = patchLine.Tokens[4].replace(
                    ";", "").replace("\n", "")

            self.searchForSpecialObject(patchLine)
            patchLine.addToUsedObject(PD_LIBRARIES)
            self.PatchLinesExternalFound.append(patchLine)

    def checkIsObjIsSingle(self, patchLine):
        if patchLine[1] == "obj":
            nameOfTheObject = patchLine[4].replace(";", "").replace("\n", "")
            nameOfTheObject = nameOfTheObject.replace(",", "")
            if nameOfTheObject in PD_LIBRARIES.LibraryNames:
                LibraryClass = PD_LIBRARIES.get(nameOfTheObject)
                if LibraryClass and LibraryClass.singleObject:
                    return True
        return False

    def cfgExternals(self):
        for lineInfo in self.PatchLinesExternalFound:
            if lineInfo.isExternal:
                foundLibrary = self.downloadExternalLibrarySrc(
                    lineInfo.library)
                if foundLibrary:
                    for root, _, files in os.walk(
                            self.PdWebCompilerPath + "/.externals/" + lineInfo.library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.searchCFunction(lineInfo, root, file)
                            elif file.endswith(".pd"):
                                if lineInfo.name == file.split(".pd")[0]:
                                    lineInfo.isAbstraction = True
                                    lineInfo.objFound = True
                                    self.configForAbstraction(
                                        os.path.join(root, file))

                else:
                    lineInfo.objFound = False
                    self.print("    " +
                               "Could not find " + lineInfo.library, color='red')

                if lineInfo.objFound and lineInfo.isAbstraction:
                    self.print("    " + "Found Abstraction: " +
                               lineInfo.name, color='green')

                elif lineInfo.objFound and not lineInfo.isAbstraction:
                    self.print("    " + "Found External: " +
                               lineInfo.name, color='green')
                else:
                    self.print("    " + "Could not find " +
                               lineInfo.name, color='red')

    def searchCFunction(self, lineInfo, root, file):
        functionName = lineInfo.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e")
        self.regexSearch(lineInfo, functionName, os.path.join(root, file))
        if not lineInfo.objFound:
            functionName = lineInfo.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.regexSearch(lineInfo, functionName, os.path.join(root, file))

    def regexSearch(self, lineInfo, functionName, file):
        with open(file, "r") as C_file:
            file_contents = C_file.read()
            pattern = r'void\s*{}\s*\(\s*void\s*\)'.format(
                re.escape(functionName))
            matches = re.finditer(pattern, file_contents, re.DOTALL)
            listMatches = list(matches)
            if len(listMatches) > 0:
                shutil.copy(C_file.name, "webpatch/externals")
                lineInfo.objFound = True
                lineInfo.functionName = functionName
                if lineInfo.library not in self.externalsDict:
                    self.externalsDict[lineInfo.library] = [C_file.name]
                else:
                    self.externalsDict[lineInfo.library].append(C_file.name)

    def addObjSetup(self):
        '''
        This function will add the obj_setup() inside the main.c file
        '''
        addedFunctions = []
        for lineInfo in self.PatchLinesExternalFound:
            if lineInfo.functionName not in addedFunctions:
                addedFunctions.append(lineInfo.functionName)
                if lineInfo.isExternal and lineInfo.objFound:
                    start_index = None
                    end_index = None
                    for i, line in enumerate(self.templateCode):
                        if "// Externals Objects Declarations" in line:
                            start_index = i
                        if "// ====================" in line:
                            end_index = i
                        if start_index is not None and end_index is not None:
                            functionName = "void " + \
                                lineInfo.functionName + "(void);\n"
                            self.templateCode.insert(
                                start_index + 1, functionName)
                            break

                    start_index = None
                    end_index = None
                    for i, line in enumerate(self.templateCode):
                        if "// WebPd Load Externals" in line:
                            start_index = i
                        if "// ====================" in line:
                            end_index = i
                        if start_index is not None and end_index is not None:
                            functionName = lineInfo.functionName
                            functionName = "    " + functionName + "();\n"
                            self.templateCode.insert(
                                start_index + 1, functionName)
                            break

        HTML_IDS = None
        HTML_IDS_SIZE = None
        for i, line in enumerate(self.templateCode):
            if "char* HTML_IDS[] = {};" in line:
                HTML_IDS = i
            if "int HTML_IDS_SIZE = 0;" in line:
                HTML_IDS_SIZE = i
            if HTML_IDS is not None and HTML_IDS_SIZE is not None:
                lenUIReceiver = len(self.uiReceiversSymbol)
                self.templateCode[HTML_IDS] = "char* HTML_IDS[] = {"
                for i, uiReceiver in enumerate(self.uiReceiversSymbol):
                    if i == lenUIReceiver - 1:
                        self.templateCode[HTML_IDS] += '"' + uiReceiver + '"'
                    else:
                        self.templateCode[HTML_IDS] += '"' + uiReceiver + '", '
                self.templateCode[HTML_IDS] += "};\n"
                self.templateCode[HTML_IDS_SIZE] = "int HTML_IDS_SIZE = " + \
                    str(lenUIReceiver) + ";\n"
                break
        return True

    def saveMainFile(self):
        with open("webpatch/main.c", "w") as file:
            for line in self.templateCode:
                file.write(line)

    def usedLibraries(self, libraryName):
        '''
        It adds the used libraries for the patch, it can be accessed by the extra functions.
        '''
        if libraryName not in PD_LIBRARIES.UsedLibrariesNames:
            PD_LIBRARIES.UsedLibrariesNames.append(libraryName)
            PD_LIBRARIES.UsedLibraries.append(PD_LIBRARIES.get(libraryName))

    def downloadExternalLibrarySrc(self, libraryName):
        responseJson = {'message': 'Unknown error'}
        if libraryName in PD_LIBRARIES.LibraryNames:
            try:
                self.usedLibraries(libraryName)
                LibraryClass = PD_LIBRARIES.get(libraryName)
                if LibraryClass is None:
                    self.print("    Could not find " +
                               libraryName, color='red')
                    sys.exit(-1)

                LibraryClass.PROJECT_ROOT = self.PROJECT_ROOT

                if os.path.exists(os.path.join(
                        os.getcwd(), self.PdWebCompilerPath + "/.externals/" + libraryName)):
                    LibraryClass.folder = os.path.join(
                        self.PdWebCompilerPath + "/.externals/" + libraryName)
                    return True

                GithutAPI = PD_LIBRARIES.getDownloadURL(
                    LibraryClass, self.downloadSources)
                if GithutAPI is None:
                    self.print(
                        "    LibURL is not a string or None", color='red')
                    sys.exit(-1)

                elif GithutAPI == False:  # means that is a direct link
                    response = requests.get(LibraryClass.directLink)

                elif isinstance(GithutAPI, str):  # is a GithubAPI link
                    response = requests.get(GithutAPI)
                    responseJson = response.json()
                    sourceCodeLink = responseJson[0]["zipball_url"]
                    response = requests.get(sourceCodeLink)

                else:
                    self.print("    The link of the srcs of " +
                               libraryName + " is not valid", color='red')
                    sys.exit(-1)

                self.print("    Downloading " +
                           libraryName, color='yellow')

                if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
                    os.mkdir(self.PdWebCompilerPath + "/.externals")

                with open(self.PdWebCompilerPath + "/.externals/" + libraryName + ".zip", "wb") as file:
                    file.write(response.content)

                with zipfile.ZipFile(self.PdWebCompilerPath + "/.externals/" + libraryName + ".zip", 'r') as zip_ref:
                    zip_ref.extractall(self.PdWebCompilerPath + "/.externals")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(self.PdWebCompilerPath + "/.externals/" + extractFolderName,
                              self.PdWebCompilerPath + "/.externals/" + libraryName)

                LibraryClass.folder = os.path.join(
                    os.getcwd(), self.PdWebCompilerPath + "/.externals/" + libraryName)
                self.librariesFolder.append(os.path.join(
                    os.getcwd(), self.PdWebCompilerPath + "/.externals/" + libraryName))
                os.remove(
                    self.PdWebCompilerPath +
                    "/.externals/" +
                    libraryName +
                    ".zip")
                return True

            except Exception as e:
                self.print("    " + str(responseJson["message"]), color='red')
                self.print("    " + str(e), color='red')
                return False
        else:
            return False

    def getPatchPath(self):
        if os.path.isabs(self.patch):
            self.patch = self.patch
        else:
            self.patch = os.path.join(os.getcwd(), self.patch)

    def mkBackup(self):
        if not os.path.exists(self.PdWebCompilerPath + "/.backup"):
            os.mkdir(self.PdWebCompilerPath + "/.backup")
        Hour = datetime.datetime.now().hour
        Minute = datetime.datetime.now().minute
        Day = datetime.datetime.now().day
        Month = datetime.datetime.now().month
        patchName = self.patch.split("/")[-1].split(".")[0]
        backPatchName = patchName + "_" + \
            str(Day) + "_" + str(Month) + "_" + \
            str(Hour) + "_" + str(Minute) + ".pd"
        try:
            shutil.copy(
                self.patch,
                self.PdWebCompilerPath +
                "/.backup/" +
                backPatchName)
        except Exception as e:
            self.print("    " + str(e), color='red')

    def savePdPatchModified(self):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")


        if self.insideaddAbstractions:
            PatchFile = self.patch

        else:
            PatchFile = 'webpatch/data/index.pd'

        with open(PatchFile, "w") as file:
            finalPatch = []
            thereIsAbstraction = False
            for obj in self.PatchLinesExternalFound:
                if obj.isExternal and not obj.singleObject and not obj.isAbstraction:
                    patchLine = obj.patchLine
                    patchLineList = patchLine.split(" ")
                    patchLineList[4] = patchLineList[4].split("/")[1]
                    finalPatch.append(patchLineList)
                elif obj.isAbstraction:
                    patchLine = obj.patchLine
                    patchLineList = patchLine.split(" ")
                    patchLineList[4] = patchLineList[4].split("/")[1]
                    finalPatch.append(patchLineList)
                    thereIsAbstraction = True
                else:
                    patchLineList = obj.patchLine.split(" ")
                    finalPatch.append(patchLineList)

            for newLine in finalPatch:
                if newLine[0] == "#N" and newLine[1] == 'canvas' and thereIsAbstraction:
                    newLine = " ".join(newLine)
                    file.write(newLine)
                    file.write("#X declare -path data;\n")
                else:
                    newLine = " ".join(newLine)
                    file.write(newLine)


    def extraFunctions(self):
        '''
        This function will execute the second argument
        '''
        for usedLibrary in PD_LIBRARIES.UsedLibraries:
            usedLibrary.webpdPatch = self
            if usedLibrary.name in self.externalsDict:
                usedLibrary.UsedSourceFiles = self.externalsDict[usedLibrary.name]
            
            if usedLibrary.extraFuncExecuted == True:
                continue

            
            extraFlags = PD_LIBRARIES.executeExtraFunction(usedLibrary)
            if extraFlags is not None:
                for flag in extraFlags:
                    self.extraFlags.append(flag)

    def removeLibraryPrefix(self, patchfile):
        patchWithoutPrefix = []
        with open(patchfile, "r") as file:
            patchLines = file.readlines()
            for line in patchLines:
                lineTokens = line.split(" ")
                if not len(lineTokens) < 5 and "/" in lineTokens[4]:
                    lineTokens[4] = lineTokens[4].split("/")[1]
                    patchWithoutPrefix.append(lineTokens)
                else:
                    patchWithoutPrefix.append(lineTokens)

        with open(patchfile, "w") as file:
            for line in patchWithoutPrefix:
                file.write(" ".join(line))

    def addAbstractions(self):
        # list all files in webpatch/data
        before_files = os.listdir("webpatch/data")
        for dir, _, files in os.walk("webpatch/data"):
            for patchfile in files:
                if patchfile.endswith(".pd") and patchfile != "index.pd":
                    # check if patch is not in PROCESSED_ABSTRACTIONS
                    if patchfile not in PROCESSED_ABSTRACTIONS:
                        webpdPatch(sourcefile="webpatch/main.c",
                                   pdpatch="webpatch/data/" + patchfile,
                                   insideaddAbstractions=True,
                                   runMain=True)
                        self.removeLibraryPrefix(dir + "/" + patchfile)
                        PROCESSED_ABSTRACTIONS.append(patchfile)
                    
        after_files = os.listdir("webpatch/data")
        if before_files == after_files:
            return

        self.addAbstractions()

    def getDynamicLibraries(self):
        '''

        '''
        for library in PD_LIBRARIES.UsedLibraries:
            requiredLibraries = library.requireDynamicLibraries
            if requiredLibraries != False:
                for dyn_library in requiredLibraries:
                    try:
                        function = DYNAMIC_LIBRARIES[dyn_library]
                        function(self)  # call the function
                    except BaseException:
                        self.print("    " + "Could not find " +
                                   dyn_library, color='red')

    def emccCompile(self):
        '''
        This is where the code is compiled.
        '''

        self.target = 'webpatch/libpd.js'
        self.libpd_dir = self.PdWebCompilerPath + '/libpd'
        self.src_files = 'webpatch/main.c'
        memory = self.memory
        if platform.system() == "Windows":
            emcc = '"' + self.PdWebCompilerPath + '\\emsdk\\upstream\\emscripten\\emcc"'
            command = [emcc,
                       '-I "', 'webpatch\\includes\\"',
                       '-I "', self.libpd_dir + '\\pure-data/src\\"',
                       '-I "', self.libpd_dir + '\\libpd_wrapper\\"',
                       '-L "', self.PdWebCompilerPath + '\\lib/compiled\\"',
                       '-lpd',
                       '-O3',
                       '-s', f'INITIAL_MEMORY={memory}mb',
                       # '-s', 'ALLOW_MEMORY_GROWTH=1', # wait to solve problem
                       '-s', 'AUDIO_WORKLET=1',
                       '-s', 'WASM_WORKERS=1',
                       '-s', 'WASM=1',
                       '-s', 'USE_PTHREADS=1',
                       '--preload-file', 'webpatch\\data\\',
                       ]
        else:
            emcc = self.PdWebCompilerPath + '/emsdk/upstream/emscripten/emcc'
            command = [emcc,
                       '-I', 'webpatch/includes/',
                       '-I', self.libpd_dir + '/pure-data/src/',
                       '-I', self.libpd_dir + '/libpd_wrapper/',
                       '-L', self.PdWebCompilerPath + '/lib/compiled/',
                       '-lpd',
                       '-O3',
                       '-s', f'INITIAL_MEMORY={memory}mb',
                       # '-s', 'ALLOW_MEMORY_GROWTH=1', # TODO: wait to solve problem
                       '-s', 'AUDIO_WORKLET=1',
                       '-s', 'WASM_WORKERS=1',
                       '-s', 'WASM=1',
                       '-s', 'USE_PTHREADS=1',
                       '--preload-file', 'webpatch/data/',
                       ]

        indexFlag = 0
        for flag in self.extraFlags:
            # add in command after -O3, it must be added in orde, so it must be
            # after -O3
            command.insert(10 + indexFlag, flag)
            indexFlag += 1

        command.append(self.src_files)
        command.append("-o")
        command.append(self.target)

        for root, _, files in os.walk("webpatch/externals"):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    command.append(os.path.join(root, file))

        print("")
        self.print("    " + " ".join(command), color='blue')
        print("")

        if platform.system() == "Windows":
            os.system(" ".join(command))

        else:
            process = subprocess.Popen(
                command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            _, stderr = process.communicate()
            if stderr:
                if "warning" in stderr:
                    # split by new line
                    stderr = stderr.split("\n")
                    for line in stderr:
                        if "warning:" in line:
                            print("")
                            self.print("     " + line, color='yellow')
                            print("")
                        else:
                            print("     " + line)
                if "error" in stderr:
                    stderr = stderr.split("\n")
                    for line in stderr:
                        if "error:" in line:
                            print("")
                            self.print("    " + line, color='red')
                            print("")
                            sys.exit(-1)
                        else:
                            self.print(line, color='red')
                else:
                    self.print("    " + ("=" * 10) +
                               " Compiled with success " + ("=" * 10), color='green')

            process.wait()
        if isinstance(self.html, str):
            shutil.copy(self.html, "webpatch")

        if self.args.server_port:
            emrun = self.PdWebCompilerPath + \
                f'/emsdk/upstream/emscripten/emrun --port {self.args.server_port} webpatch'
            os.system(emrun)
        sys.exit(0)


if __name__ == "__main__":
    webpdPatch()
