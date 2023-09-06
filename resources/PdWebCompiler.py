import os
import sys
import subprocess
import argparse
import requests
import zipfile
import shutil
import datetime
import yaml
import re

from externals.ExternalClass import PD_SUPPORTED_EXTERNALS, PureDataExternals, PatchLine
from lib.main import DYNAMIC_LIBRARIES


## ================== EXTERNALS THINGS ================== ##

PROCESSED_ABSTRACTIONS = []


class webpdPatch():
    def __init__(self, sourcefile="src/template.c", pdpatch=None, insideaddAbstractions=False) -> None:
        parser = argparse.ArgumentParser(description="Sample script to create personalised webPd patch")
        parser.add_argument('--patch', required=True, help='Patch file to compile')
        parser.add_argument('--html', required=False, help='HTML file')
        parser.add_argument('--confirm', required=False, help='Confirm if the object is an external')
        parser.add_argument('--clearTmpFiles', required=False, default=False, help='Remove all objects from the patch')
        args = parser.parse_args()
        self.FoundExternals = False
        self.html = False
        self.source = sourcefile
        self.clearTmpFiles = args.clearTmpFiles
        self.uiReceiversSymbol = []
        self.insideaddAbstractions = insideaddAbstractions
        self.lastPrintedLine = ""
        self.extraFlags = []
        self.externalsDict = {}

        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = args.patch

        if not os.path.isabs(args.html) and not insideaddAbstractions:
            absolutePath = os.path.dirname(os.path.abspath(os.path.join(os.getcwd(), args.html)))
            self.html = os.getcwd() + "/" + args.html
            print(self.html)


        if not os.path.isabs(self.patch) and not insideaddAbstractions:
            absolutePath = os.path.dirname(os.path.abspath(os.path.join(os.getcwd(), self.patch)))
            self.patch = os.getcwd() + "/" + self.patch
            self.source = os.getcwd() + "/" + self.source
            self.PROJECT_ROOT = absolutePath
            os.chdir(absolutePath)

        with open(self.patch, "r") as file:
            self.PatchLines = file.readlines()

        # read externals
        self.getSupportedLibraries()
        # ==============

        self.PdWebCompilearPath = os.path.dirname(os.path.realpath(__file__))
        self.PdWebCompilearPath = os.path.dirname(self.PdWebCompilearPath)
        self.PROJECT_ROOT = os.getcwd()
        self.processedAbstractions = []

        # template code
        if not insideaddAbstractions:
            print("\n")
            with open(os.path.join(self.PdWebCompilearPath, "src/template.c"), "r") as file:
                self.templateCode = file.readlines()
        else:
            with open("webpatch/main.c", "r") as file:
                self.templateCode = file.readlines()


        if not insideaddAbstractions:
            if not os.path.exists(".externals"):
                os.mkdir(".externals")

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
        self.confirm = args.confirm
        self.getPatchPath()
        self.mkBackup()
        self.PatchLinesExternalFound = []
        self.findExternals()
        self.cfgExternals()
        self.addObjSetup()
        if not insideaddAbstractions:
            self.savePdPatchModified()

        self.saveMainFile()
        self.extraFunctions()
        if not insideaddAbstractions:
            self.addAbstractions()

        self.copyAllDataFiles()

        shutil.copy(self.PdWebCompilearPath + "/src/index.html", "webpatch/index.html")
        shutil.copy(self.PdWebCompilearPath + "/src/helpers.js", "webpatch/helpers.js")
        shutil.copy(self.PdWebCompilearPath + "/src/enable-threads.js", "webpatch/enable-threads.js")

        self.getDynamicLibraries()

        if not insideaddAbstractions:
            self.emccCompile()

        if self.clearTmpFiles == True and not insideaddAbstractions:
            if os.path.exists(".externals"):
                shutil.rmtree(".externals")

        if not insideaddAbstractions:
            print("")

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


    def configForAbstraction(self, abstractionfile):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")
        shutil.copy(abstractionfile, "webpatch/data")
        print("\033[92m" + "    Found Abstraction: " + abstractionfile.split("/")[-1] + "\033[0m")


    def copyAllDataFiles(self):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")

        for root, _, files in os.walk("extra"):
            for file in files:
                shutil.copy(os.path.join(root, file), "webpatch/data")


    def checkIfIsSupportedObject(self, patchLine):
        pdClass = patchLine[1]
        if pdClass == "array":
                print("\033[91m" + "    " +
                        "Visual Arrays are not supported, use [array define] object" + "\033[0m")


    def findExternals(self):
        for line in enumerate(self.PatchLines):
            patchLine = PatchLine()
            patchLine.patchLineIndex = line[0]
            patchLine.patchLine = line[1]
            patchLine.isExternal = False
            lineArgs = patchLine.patchLine.split(" ")
            if len(lineArgs) < 5:
                continue

            # here for especial objects (externals, not supported objects, abstractions)
            objName = lineArgs[4].replace("\n", "").replace(";", "").replace(",", "")

            self.checkIfIsSupportedObject(lineArgs)

            if (lineArgs[0] == "#X" and lineArgs[1] == "obj" and "/" in lineArgs[4]) and objName != "/":
                patchLine.isExternal = True
                patchLine.library = lineArgs[4].split("/")[0]
                patchLine.name = lineArgs[4].split("/")[1].replace("\n", "").replace(";", "").replace(",", "")
                patchLine.objGenSym = 'class_new(gensym("' + lineArgs[4].split("/")[1].replace("\n", "").replace(";", "") + '")'

            elif self.checkIsObjIsSingle(lineArgs):
                patchLine.isExternal = True
                patchLine.library = lineArgs[4].replace(";", "").replace("\n", "").replace(",", "")
                patchLine.name = patchLine.library
                patchLine.objGenSym = 'gensym("' + patchLine.library + '")'
                patchLine.singleObject = True

            elif ("s" == lineArgs[4] or "send" == lineArgs[4]):
                receiverSymbol = lineArgs[5].replace("\n", "").replace(";", "").replace(",", "")
                if ("ui_" in receiverSymbol):
                    patchLine.uiReceiver = True
                    patchLine.uiSymbol = receiverSymbol
                    self.uiReceiversSymbol.append(receiverSymbol)

                patchLine.name = lineArgs[4].replace(";", "").replace("\n", "")

            else:
                patchLine.name = lineArgs[4].replace(";", "").replace("\n", "")

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
                foundLibrary = self.downloadExternalLibrarySrc(lineInfo.library)
                if foundLibrary:
                    for root, _, files in os.walk(".externals/" + lineInfo.library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.searchCFunction(lineInfo, root, file)
                            elif file.endswith(".pd"):
                                if lineInfo.name == file.split(".pd")[0]:
                                    lineInfo.isAbstraction = True
                                    lineInfo.objFound = True
                                    self.configForAbstraction(os.path.join(root, file))

                else:
                    lineInfo.objFound = False
                    print("    " + "\033[91m" + "Could not find " + lineInfo.library + "\033[0m")

                if lineInfo.objFound and lineInfo.isAbstraction:
                    print("\033[92m" + "    Found Abstraction: " + lineInfo.name + "\033[0m")
                elif lineInfo.objFound and not lineInfo.isAbstraction:
                    print("\033[92m" + "    Found External: " + lineInfo.name + "\033[0m")
                else:
                    print("\033[91m" + "    Could not find " + lineInfo.name + "\033[0m")
                    print("")

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
            pattern = r'void\s*{}\s*\(\s*void\s*\)'.format(re.escape(functionName))
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
                            functionName = "void " + lineInfo.functionName + "(void);\n"
                            self.templateCode.insert(start_index + 1, functionName)
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
                            self.templateCode.insert(start_index + 1, functionName)
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
                self.templateCode[HTML_IDS_SIZE] = "int HTML_IDS_SIZE = " + str(lenUIReceiver) + ";\n"
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
                    print("\033[91m" + "Could not find " + libraryName + "\033[0m")
                    sys.exit(-1)

                LibraryClass.PROJECT_ROOT = self.PROJECT_ROOT

                if os.path.exists(os.path.join(os.getcwd(), ".externals/" + libraryName)):
                    LibraryClass.folder = os.path.join(os.getcwd(), ".externals/" + libraryName)
                    return True

                GithutAPI = PD_LIBRARIES.getDownloadURL(LibraryClass, self.downloadSources)
                if GithutAPI is None:
                    print("\033[91m" + "   LibURL is not a string or None" + "\033[0m")
                    sys.exit(-1)

                elif GithutAPI == False: # means that is a direct link
                    response = requests.get(LibraryClass.directLink)

                elif isinstance(GithutAPI, str): # is a GithubAPI link
                    response = requests.get(GithutAPI)
                    responseJson = response.json()
                    sourceCodeLink = responseJson[0]["zipball_url"]
                    response = requests.get(sourceCodeLink)

                else:
                    print("\033[91m" + "    The link of the srcs of " + libraryName + " is not valid" + "\033[0m")
                    sys.exit(-1)

                print("\033[92m" + "    Downloading " + libraryName + "\033[0m")


                if not os.path.exists(".externals"):
                    os.mkdir(".externals")

                with open(".externals/" + libraryName + ".zip", "wb") as file:
                    file.write(response.content)

                with zipfile.ZipFile(".externals/" + libraryName + ".zip", 'r') as zip_ref:
                    zip_ref.extractall(".externals")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(".externals/" + extractFolderName, ".externals/" + libraryName)

                LibraryClass.folder = os.path.join(os.getcwd(), ".externals/" + libraryName)
                self.librariesFolder.append(os.path.join(os.getcwd(), ".externals/" + libraryName))
                os.remove(".externals/" + libraryName + ".zip")
                return True

            except Exception as e:
                print("\033[91m" + str(responseJson["message"]) + "\033[0m")
                print("\033[91m" + str(e) + "\033[0m")
                return False
        else:
            return False


    def getPatchPath(self):
        if os.path.isabs(self.patch):
            self.patch =  self.patch
        else:
             self.patch = os.path.join(os.getcwd(),  self.patch)


    def mkBackup(self):
        if not os.path.exists(".backup"):
            os.mkdir(".backup")
        Hour = datetime.datetime.now().hour
        Minute = datetime.datetime.now().minute
        Day = datetime.datetime.now().day
        Month = datetime.datetime.now().month
        patchName = self.patch.split("/")[-1].split(".")[0]
        backPatchName = patchName + "_" + str(Day) + "_" + str(Month) + "_" + str(Hour) + "_" + str(Minute) + ".pd"
        try:
            shutil.copy(self.patch, ".backup/" + backPatchName)
        except Exception as e:
            print("\033[91m" + str(e) + "\033[0m")



    def savePdPatchModified(self):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")

        with open('webpatch/data/index.pd', "w") as file:
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
            if usedLibrary.name in self.externalsDict:
                usedLibrary.UsedSourceFiles = self.externalsDict[usedLibrary.name]

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
                    webpdPatch(sourcefile="webpatch/main.c", pdpatch="webpatch/data/" + patchfile, insideaddAbstractions=True)
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
                        function(self) # call the function
                    except:
                        print("\033[91m" + "Could not find " + dyn_library + "\033[0m")


    def emccCompile(self):
        '''
        This is where the code is compiled.
        '''
        # check if emcc is installed and in the path
        if shutil.which("emcc") is None:
            print("\033[91m" + "emcc is not installed or in the path" + "\033[0m")
            print("")
            sys.exit(-1)

        self.target = 'webpatch/libpd.js'
        self.libpd_dir = self.PdWebCompilearPath + '/libpd'
        self.src_files = 'webpatch/main.c'

        command = ['emcc',
                    "-I", "webpatch/includes/",
                    "-I", '' + self.libpd_dir + '/pure-data/src/',
                    "-I", '' + self.libpd_dir + '/libpd_wrapper/',
                    "-L", '' + self.libpd_dir + '/build/libs/',
                    "-lpd",
                    "-O3",
                    "-s", "AUDIO_WORKLET=1",
                    "-s", "WASM_WORKERS=1",
                    "-s", "WASM=1",
                    "-s", "USE_PTHREADS=1",
                    "--preload-file", "webpatch/data/",
                   ]

        indexFlag = 0
        for flag in self.extraFlags:
            # add in command after -O3, it must be added in orde, so it must be after -O3
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
        print("\033[94m" + " ".join(command) + "\033[0m")
        print("")
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        _, stderr = process.communicate()
        if stderr:
                if "warning" in stderr:
                    # split by new line
                    stderr = stderr.split("\n")
                    for line in stderr:
                        if "warning:" in line:
                            print("")
                            print("     " + "\033[93m" + line + "\033[0m")
                            print("")
                        else:
                            print("     " + line)
                if "error" in stderr:
                    stderr = stderr.split("\n")
                    for line in stderr:
                        if "error:" in line:
                            print("")
                            print("\033[91m" + line + "\033[0m")
                            print("")
                            sys.exit(-1)
                        else:
                            print(line)
                else:
                    print("\033[92m" + ("=" * 10) + " Compiled with success " + ("=" * 10) +  "\033[0m")

        process.wait()
        if isinstance(self.html, str):
            shutil.copy(self.html, "webpatch")
            print("\033[92m" + "    " + "Copied to " + self.html + "\033[0m")



if __name__ == "__main__":
    webpdPatch()
