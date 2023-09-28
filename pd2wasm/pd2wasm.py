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
import importlib
import json

from .externals import PD_SUPPORTED_EXTERNALS, PureDataExternals, PatchLine
from .lib.DynamicLibraries import DYNAMIC_LIBRARIES
from .helpers.helpers import myprint, emccPaths


## ================== EXTERNALS THINGS ================== ##

INDEX_HTML = '''
<!doctype html>
<html lang="en-us">
    <body>
    <script>
        window.location.href = 'webpatch/{}';
    </script>
    </body>
</html>
'''


class webpdPatch():
    def __init__(self, sourcefile="src/template.c", pdpatch=None,
                 insideaddAbstractions=False, runMain=True, parent=[]) -> None:
        self.PdWebCompilerPath = os.path.dirname(os.path.realpath(__file__))
        self.emcc = emccPaths()
        
        # get this folder directory
        parser = argparse.ArgumentParser(
            formatter_class=argparse.RawTextHelpFormatter, description="Check the complete docs in https://www.charlesneimog.com/PdWebCompiler")
        parser.add_argument('--patch', required=True,
                            help='Patch file (.pd) to compile')
        parser.add_argument(
            '--html',
            required=False,
            help='HTML used to load and render the Web Page. If not defined, we use the default one')

        parser.add_argument(
            '--page-folder',
            required=False,
            help='Folder with html, css, js and all others files to be used in the Web Page. If not defined, we use the default one')


        parser.add_argument('--confirm', required=False,
                            help='There is some automatic way check if the external is correct, but it is not always accurate. If you want to confirm if the external is correct, use this flag')
        parser.add_argument('--clearTmpFiles', required=False,
                            default=False, help='Remove all TempFiles, like .externals folder')

        parser.add_argument('--server-port', required=False,
                            default=False, help='Set the port to start the server')

        parser.add_argument('--initial-memory', required=False,
                            default=32, help='Set the initial memory of the WebAssembly in MB')
        
        parser.add_argument('--version', action='version',
                            version='%(prog)s 1.0.9')

        self.args = parser.parse_args()

        if not os.path.isabs(self.args.patch) and not insideaddAbstractions:
            absolutePath = os.path.dirname(os.path.abspath(
                os.path.join(os.getcwd(), self.args.patch)))
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
        if not insideaddAbstractions:
            self.activeEmcc()
            self.downloadLibPd()
            self.importExternalObjs()
            self.getSupportedLibraries()

        else:
            self.externalsExtraFunctions = parent.externalsExtraFunctions        

        if self.PROJECT_ROOT[-1] != "/" and (self.PROJECT_ROOT[-1] != "\\"):
            if platform.system() == "Windows":
                self.PROJECT_ROOT = self.PROJECT_ROOT + "\\"
            else:
                self.PROJECT_ROOT = self.PROJECT_ROOT + "/"

        self.FoundExternals = False
        self.html = False
        self.pageFolder = self.args.page_folder
        self.parent = parent
        self.source = sourcefile
        self.sortedSourceFiles = [] # case there is some files that need to be compiled in order
        self.PROCESSED_ABSTRACTIONS = []
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
                myprint("Bye Bye!", color="green")


    def main(self, pdpatch=None, insideaddAbstractions=False):
        '''
        Main functions, it will call all other functions.
        '''

        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = self.args.patch
        patchFileName = os.path.basename(self.patch)
        myprint("\n    • Patch => " + patchFileName + "\n", color='cyan')
        if self.args.html is not None:
            if not os.path.isabs(self.args.html) and not insideaddAbstractions:
                self.html = os.getcwd() + "/" + self.args.html
        elif self.pageFolder is not None:
            if not os.path.isabs(self.pageFolder) and not insideaddAbstractions:
                self.pageFolder = os.getcwd() + "/" + self.pageFolder 
        else:
            self.html = self.PdWebCompilerPath + "/src/index.html"

        if "index.html" not in str(self.html) and not insideaddAbstractions:
            myprint("The name of your html is not index.html, we will copy one index.html for webpatch!", color="red")

        if not os.path.exists(self.PROJECT_ROOT + "/.backup"):
            os.mkdir(self.PROJECT_ROOT + "/.backup")

        if not insideaddAbstractions:
            if os.path.exists(self.PROJECT_ROOT + "index.html"):
                myprint("index.html already exists in the root folder, " \
                        "please change his name or delete it, making backup and deleting it.", color="red")
                shutil.copy(self.PROJECT_ROOT + "index.html", self.PROJECT_ROOT + ".backup/index.html")
            else:
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
                myprint("Patch not found: The current folder is " + str(os.getcwd()), color="red")
                sys.exit(0)

        with open(self.args.patch, "r") as file:
            self.PatchLines = file.readlines()
        self.replaceVISArray()
        self.processedAbstractions = []

        if not insideaddAbstractions:
            with open(os.path.join(self.PdWebCompilerPath, "src/template.c"), "r") as file:
                self.templateCode = file.readlines()
            if not os.path.exists(self.PdWebCompilerPath + "/.externals"):
                os.mkdir(self.PdWebCompilerPath + "/.externals")
            if not os.path.exists(self.PROJECT_ROOT + "webpatch"):
                os.mkdir(self.PROJECT_ROOT + "webpatch")
            else:
                shutil.rmtree(self.PROJECT_ROOT + "webpatch")
                os.mkdir(self.PROJECT_ROOT + "webpatch")
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
            with open(self.PROJECT_ROOT + "webpatch/main.c", "r") as file:
                self.templateCode = file.readlines()
                
        self.librariesFolder = []
        self.confirm = self.args.confirm
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
                    "/src/index.html", self.PROJECT_ROOT + "webpatch/index.html")
            shutil.copy(self.PdWebCompilerPath +
                    "/src/helpers.js", self.PROJECT_ROOT + "webpatch/helpers.js")
            shutil.copy(self.PdWebCompilerPath +
                    "/src/enable-threads.js", self.PROJECT_ROOT + "webpatch/enable-threads.js")
            if self.pageFolder is not None:
                for root, dir, files in os.walk(self.pageFolder):
                    for file in files:
                        shutil.copy(os.path.join(root, file), self.PROJECT_ROOT + "webpatch")
                    for folder in dir:
                        shutil.copytree(os.path.join(root, folder), self.PROJECT_ROOT + "webpatch/" + folder)
        self.getDynamicLibraries()
        if insideaddAbstractions:
            for sourceFile in self.sortedSourceFiles:
                self.parent.sortedSourceFiles.append(sourceFile)
            for pdpatch in self.PROCESSED_ABSTRACTIONS:
                self.parent.PROCESSED_ABSTRACTIONS.append(pdpatch)
        if not insideaddAbstractions:
            self.emccCompile()
        if not insideaddAbstractions:
            print("")
        return True


    def activeEmcc(self):
        '''
        This function will download and install emcc if it is not installed.
        '''
        if not os.path.exists(self.PdWebCompilerPath + "/emsdk"):
            emccGithub = "https://api.github.com/repos/emscripten-core/emsdk/tags"
            response = requests.get(emccGithub)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            myprint("Downloading emcc...", color="green")
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
                os.system(f"cmd /C {self.emcc.emsdk} install latest")
                os.system(f"cmd /C {self.emcc.emsdk} activate latest")
            else:
                os.environ["EMSDK_QUIET"] = "1"
                os.system(f"chmod +x {self.emcc.emsdk}")
                os.system(f"{self.emcc.emsdk} install latest")
                os.system(f"{self.emcc.emsdk} activate latest")
                os.system(f"chmod +x {self.emcc.emsdk_env}")


    def importExternalObjs(self):
        '''
        Each externals library can use extrafunctions, this function will
        these functions from the externals folders.
        '''
        externalFolder = os.path.join(os.path.dirname(os.path.abspath(__file__)), "externals")
        module_files = [f for f in os.listdir(externalFolder) if f.endswith('.py') and not f.startswith('__')]
        module_names = [os.path.splitext(f)[0] for f in module_files]
        for module_name in module_names:
            if module_name != 'ExternalClass':
                module = importlib.import_module('pd2wasm.externals.' + module_name)
                self.externalsExtraFunctions.append(module)


    def downloadLibPd(self):
        '''
        It download and configure the libpd repository, source files and others.
        '''
        if shutil.which("git") is None:
            myprint("" + "Git is not installed!", color='red')
            myprint("")
            myprint("Install git using the pd2wasm Docs https://charlesneimog.github.io/PdWebCompiler/patch/#git", color='yellow')
            sys.exit(0)
        if not os.path.exists(self.PdWebCompilerPath + "/libpd"):
            myprint("" + "Downloading libpd...", color='yellow')
            os.mkdir(self.PdWebCompilerPath + "/libpd")
            os.system("git clone https://github.com/charlesneimog/libpd.git " +
                      f"{self.PdWebCompilerPath}/libpd --recursive")
            os.system(f"cd {self.PdWebCompilerPath}/libpd && git switch emscripten-pd54 &&"
                      " git submodule init && git submodule update" +
                      " && cd pure-data && git submodule init && git submodule update && git switch emscripten-pd54")


    def getSupportedLibraries(self):
        ''' 
        It reads yaml file and get all supported libraries.
        '''
        global PD_LIBRARIES
        thisFile = os.path.dirname(os.path.realpath(__file__))
        externalFile = os.path.join(thisFile, "externals/Externals.yaml")
        PD_LIBRARIES = PD_SUPPORTED_EXTERNALS()
        self.DynamicLibraries = []

        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.downloadSources = supportedLibraries['DownloadSources']
            supportedLibraries = supportedLibraries['SupportedLibraries']
            for library in supportedLibraries:
                PdLib = PureDataExternals(library, self.PROJECT_ROOT)
                PD_LIBRARIES.add(PdLib)
    

    def searchForSpecialObject(self, line):
        '''
        There is some special objects that we need extra configs.
        This function will search for these objects and add the configs.
        '''
        if len(line.Tokens) < 5: # case it is array or float.
            return

        if line.Tokens[4].replace("\n", "") == "clone":
            self.extraConfig_CLONE(line)
            

    def extraConfig_CLONE(self, line):
        '''
        This function execute the extra config for clone object.
        '''
        for token in line.Tokens:
            if not "-" in token:
                absName = token.replace(",", "") + ".pd"
                for root, _, files in os.walk(self.PdWebCompilerPath):
                    for file in files:
                        if not file.endswith(".pd") and absName not in file:
                            continue
                        if absName != file:
                            continue
                        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data/" + absName):
                            myprint("" + "Copying " +
                                       absName + " to webpatch/data", color='yellow')
                            shutil.copy(
                                os.path.join(
                                    root, file), self.PROJECT_ROOT + "webpatch/data")

    def replaceVISArray(self):
        '''
        Visual arrays are not support by WebPd, this function will replace
        Visual Arrays by [array define] object.
        '''
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
            myprint("" + self.args.patch +
                       " has VIS array, it is not supported and was replaced by [array define]", color='yellow')
            self.replaceVISArray()

        # save self.PatchLines to file
        with open(self.args.patch, "w") as file:
            for line in self.PatchLines:
                file.write(line)


    def configForAbstraction(self, abstractionfile):
        '''
        This function copies all abstractions to webpatch/data folder.
        '''
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")
        shutil.copy(abstractionfile, self.PROJECT_ROOT + "webpatch/data")


    def copyAllDataFiles(self):
        '''
        This function copies all files from supported folder to webpatch/data folder.
        TODO: Add support to copy folders specified by the user.
        '''
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")
        for folderName in ["extra", "Extras", "Audios", "libs", "Abstractions"]:
            if not os.path.exists(folderName):
                continue
            if folderName != "Extras":
                shutil.copytree(folderName, self.PROJECT_ROOT + "webpatch/data/" + folderName)
            else:
                shutil.copytree(folderName, self.PROJECT_ROOT + "webpatch/Extras")


    def checkIfIsSupportedObject(self, patchLine):
        pdClass = patchLine[1]
        if pdClass == "array": # If something go wrong and the array is not replaced, we will replace it here
            myprint("Visual Arrays are not supported, "
                    + "use [array define] object", color='red')
            sys.exit(1)
        
        # here you add all other objects


    def findExternals(self):
        '''
        This function will find all externals objects in the patch.
        '''
        for line in enumerate(self.PatchLines):
            patchLine = PatchLine()
            patchLine.patchLineIndex = line[0]
            patchLine.patchLine = line[1]
            patchLine.isExternal = False
            patchLine.Tokens = patchLine.patchLine.split(" ")
            if len(patchLine.Tokens) < 5:
                continue
            objName = patchLine.Tokens[4].replace(
                "\n", "").replace(";", "").replace(",", "")
            self.checkIfIsSupportedObject(patchLine.Tokens)

            if (patchLine.Tokens[0] == "#X" and patchLine.Tokens[1] == "obj"
                    and "/" in patchLine.Tokens[4]) and objName != "/":
                patchLine.isExternal = True
                patchLine.library = patchLine.Tokens[4].split("/")[0]
                patchLine.name = objName.split("/")[-1]
                patchLine.objGenSym = 'class_new(gensym("' + objName + '")'
                absPath = self.PROJECT_ROOT + "/" + patchLine.library + "/" + patchLine.name + ".pd"
                if os.path.exists(absPath):
                    patchLine.isAbstraction = True
                    patchLine.objFound = True
                    patchLine.isExternal = False

            elif self.ObjIsLibrary(patchLine.Tokens):
                patchLine.isExternal = True
                patchLine.library =  objName
                patchLine.name = patchLine.library
                if os.path.exists(patchLine.library + ".pd"):
                    myprint("It is an abstraction", color='red')
                patchLine.objGenSym = 'gensym("' + patchLine.library + '")'
                patchLine.singleObject = True

            elif ("s" == patchLine.Tokens[4] or "send" == patchLine.Tokens[4]):
                receiverSymbol = patchLine.Tokens[5].replace(
                    "\n", "").replace(";", "").replace(",", "")
                if "ui_" in receiverSymbol:
                    patchLine.uiReceiver = True
                    patchLine.uiSymbol = receiverSymbol
                    self.uiReceiversSymbol.append(receiverSymbol)
                patchLine.name = objName

            else:
                patchLine.name = objName
            self.searchForSpecialObject(patchLine)
            patchLine.addToUsedObject(PD_LIBRARIES)
            self.PatchLinesExternalFound.append(patchLine)


    def ObjIsLibrary(self, patchLine):
        '''
        This function check if the object has the same name as the library.
        '''
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
                    myprint("Could not find " + lineInfo.library, color='red')
                if lineInfo.objFound and lineInfo.isAbstraction:
                    myprint("Found Abstraction: " +
                               lineInfo.name, color='green')
                elif lineInfo.objFound and not lineInfo.isAbstraction:
                    myprint("Found External: " +
                               lineInfo.name, color='green')
                else:
                    myprint("Could not find " +
                               lineInfo.name, color='red')


    def searchCFunction(self, lineInfo, root, file):
        '''
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        '''
        functionName = lineInfo.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e") # else use . as 0x2e
        self.regexSearch(lineInfo, functionName, os.path.join(root, file))
        if not lineInfo.objFound:
            functionName = lineInfo.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.regexSearch(lineInfo, functionName, os.path.join(root, file))


    def regexSearch(self, lineInfo, functionName, file):
        '''
        This search for the setup function using regex.  
        '''

        with open(file, "r") as C_file:
            file_contents = C_file.read()
            pattern = r'void\s*{}\s*\(\s*void\s*\)'.format(
                re.escape(functionName))
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


    def enumerateExternals(self, libraryFolder, libraryName):
        '''
        Recursively enumerate all external objects and save the JSON file.
        '''

        # Check if there is an "externals.json" file in the self.PdWebCompilerPath folder
        externalsJson = os.path.join(self.PdWebCompilerPath, "externals.json")
        if os.path.exists(externalsJson):
            with open(externalsJson, "r") as file:
                externalsDict = json.load(file)
        else:
            externalsDict = {}

        # Define a dictionary to store external object information
        externalsObjects = []

        for root, _, files in os.walk(libraryFolder):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    with open(os.path.join(root, file), "r") as c_file:
                        file_contents = c_file.read()
                        pattern = r'class_new\s*\(\s*gensym\s*\(\s*\"([^"]*)\"\s*\)'
                        matches = re.finditer(pattern, file_contents)
                        for match in matches:
                            objectName = match.group(1)
                            externalsObjects.append(objectName)
                            

        # Add the library information to the externals dictionary
        externalsDict[libraryName] = externalsObjects

        # Save the JSON file
        with open(externalsJson, "w") as file:
            json.dump(externalsDict, file, indent=4)
                                                                    

    def saveMainFile(self):
        with open(self.PROJECT_ROOT + "webpatch/main.c", "w") as file:
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
                    myprint("Could not find " +
                               libraryName, color='red')
                    sys.exit(-1)

                LibraryClass.PROJECT_ROOT = self.PROJECT_ROOT

                if os.path.exists(os.path.join(self.PdWebCompilerPath + "/.externals/" + libraryName)):
                    LibraryClass.folder = os.path.join(self.PdWebCompilerPath + "/.externals/" + libraryName)
                    return True

                GithutAPI = PD_LIBRARIES.getDownloadURL(LibraryClass, self.downloadSources)
                if GithutAPI is None:
                    myprint("LibURL is not a string or None", color='red')
                    sys.exit(-1)

                elif GithutAPI == False:  # means that is a direct link
                    response = requests.get(LibraryClass.directLink)

                elif isinstance(GithutAPI, str):  # is a GithubAPI link
                    response = requests.get(GithutAPI)
                    responseJson = response.json()
                    sourceCodeLink = responseJson[0]["zipball_url"]
                    response = requests.get(sourceCodeLink)

                else:
                    myprint("The link of the srcs of " +
                               libraryName + " is not valid", color='red')
                    sys.exit(-1)

                myprint("Downloading " +
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
                
                self.enumerateExternals(LibraryClass.folder, libraryName)
                return True

            except Exception as e:
                myprint("" + str(e), color='red')
                myprint("" + str(responseJson["message"]), color='red')
                myprint("" + str(e), color='red')
                sys.exit(1)
        else:
            return False


    def mkBackup(self):
        '''
        This function makes a backup of the patch file.
        '''
        if not os.path.exists(self.PROJECT_ROOT + "/.backup"):
            os.mkdir(self.PROJECT_ROOT + "/.backup")
        Hour = datetime.datetime.now().hour
        Minute = datetime.datetime.now().minute
        Day = datetime.datetime.now().day
        Month = datetime.datetime.now().month
        patchName = os.path.basename(self.args.patch)
        patchName = patchName.split(".pd")[0]
        backPatchName = patchName + "_" + \
            str(Day) + "_" + str(Month) + "_" + \
            str(Hour) + "_" + str(Minute) + ".pd"
        backPatchPath = self.PROJECT_ROOT + ".backup/" + backPatchName
        if platform.system() == "Windows":
            backPatchPath = backPatchPath.replace("/", "\\")
        try:
            shutil.copy(self.args.patch, backPatchPath)
        except Exception as e:
            myprint(str(e), color='red')
    

    def extraFunctions(self):
        '''
        This function try to execute extra functions for the library.
        In this function, there is headers configurations, extra flags and
        others.
        '''
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


    def removeLibraryPrefix(self, patchfile):
        '''
        This function remove the library prefix from the patch file: else/counter => counter.
        because after the compilation, all the externals become embedded objects.
        '''
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


    def savePdPatchModified(self):
        '''
        After remove all the prefix, and others things (like remove visual arrays),
        we save the patch file that will be used by the website.
        '''
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")
        if self.insideaddAbstractions:
            PatchFile = self.args.patch
        else:
            PatchFile = self.PROJECT_ROOT + 'webpatch/data/index.pd'
        with open(PatchFile, "w") as file:
            finalPatch = []
            thereIsAbstraction = False
            for obj in self.PatchLinesExternalFound:
                if obj.isExternal and not obj.singleObject and not obj.isAbstraction:
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


    def addAbstractions(self):
        before_files = os.listdir(self.PROJECT_ROOT + "webpatch/data")
        for dir, _, files in os.walk(self.PROJECT_ROOT + "webpatch/data"):
            for patchfile in files:
                if patchfile.endswith(".pd") and patchfile != "index.pd":
                    # check if patch is not in PROCESSED_ABSTRACTIONS
                    if patchfile not in self.PROCESSED_ABSTRACTIONS:
                        webpdPatch(sourcefile=self.PROJECT_ROOT + "webpatch/main.c",
                                   pdpatch=self.PROJECT_ROOT + "webpatch/data/" + patchfile,
                                   insideaddAbstractions=True,
                                   runMain=True, 
                                   parent=self)
                        self.removeLibraryPrefix(dir + "/" + patchfile)
                        self.PROCESSED_ABSTRACTIONS.append(patchfile)
        after_files = os.listdir(self.PROJECT_ROOT + "webpatch/data")
        if before_files == after_files:
            return
        self.addAbstractions()


    def getDynamicLibraries(self):
        '''
        Configures dynamic libraries for compilation.

        This method iterates through the list of used libraries in the `PD_LIBRARIES.UsedLibraries`
        and checks if they require dynamic libraries specified in Externals.yaml. If dynamic libraries are required, it attempts
        to locate and configure them using the supported libraries in `DYNAMIC_LIBRARIES`.

        This method provides a way to configure dynamic libraries necessary for building a project
        with external dependencies.
        '''
        for library in PD_LIBRARIES.UsedLibraries:
            requiredLibraries = library.requireDynamicLibraries
            if requiredLibraries != False:
                for dyn_library in requiredLibraries:
                    try:
                        function = DYNAMIC_LIBRARIES[dyn_library]
                        function(self)  # call the function
                    except Exception as e:
                        myprint("Could not find " + dyn_library, color='red')
                        myprint("" + str(e), color='red')
                        sys.exit(1)


    def emccCompile(self):
        '''
        This function create the emcc command, run it and if the user passed
        the --server-port flag, it will run the server.
        '''
        memory = self.memory
        if platform.system() == "Windows":
            self.target = self.PROJECT_ROOT + 'webpatch\\libpd.js'
            self.libpd_dir = self.PdWebCompilerPath + '\\libpd'
            self.src_files = self.PROJECT_ROOT + 'webpatch\\main.c'
            os.chdir(self.PROJECT_ROOT)
            emcc = self.emcc.emcc
            command = [emcc,
                       '-I ', self.PROJECT_ROOT + 'webpatch\\includes\\',
                       '-I ', self.libpd_dir + '\\pure-data\\src\\',
                       '-I ', self.libpd_dir + '\\libpd_wrapper\\',
                       '-L ', self.PdWebCompilerPath + '\\lib\\compiled\\',
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
            self.target = self.PROJECT_ROOT + 'webpatch/libpd.js'
            self.libpd_dir = self.PdWebCompilerPath + '/libpd'
            self.src_files = self.PROJECT_ROOT + 'webpatch/main.c'
            emcc = self.PdWebCompilerPath + '/emsdk/upstream/emscripten/emcc'
            command = [emcc,
                       '-I', self.PROJECT_ROOT + 'webpatch/includes/',
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
                       '--preload-file', self.PROJECT_ROOT + 'webpatch/data/',
                       ]

        indexFlag = 0
        for flag in self.extraFlags:
            # add Extra Flags in command after -O3, it must be added in orde, so it must be
            command.insert(10 + indexFlag, flag)
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

        print("")
        myprint("" + " ".join(command), color='blue')
        print("")

        if platform.system() == "Windows":
            command.insert(0, "/C")
            command.insert(0, "cmd")
            command = " ".join(command)          
        
        process = subprocess.Popen(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        _, stderr = process.communicate()           
        
        error = False
        if isinstance(stderr, str):
            stderrTOKENS = stderr.lower().split("\n") 
            for key in stderrTOKENS:
                if "warning:" in key:
                    print("")
                    myprint(key, color='yellow')
                elif "error" in key and isinstance(key, str):
                    error = True
                    print("")
                    myprint(key, color='red')
                else:
                    myprint(key)

            if error:
                myprint("There was an error compiling, READ the output", color='red')
                sys.exit(1)

            else:
                myprint(("=" * 10) +
                            " Compiled with success " + ("=" * 10) + "\n", color='green')

        process.wait()
        
        if isinstance(self.html, str):
            shutil.copy(self.html, self.PROJECT_ROOT + "webpatch")

        if self.args.server_port:
            myprint("Starting server on port " + str(self.args.server_port), color='green')
            emrun = self.PdWebCompilerPath + \
                f'/emsdk/upstream/emscripten/emrun --port {self.args.server_port} .'
            os.system(emrun)
        sys.exit(0)