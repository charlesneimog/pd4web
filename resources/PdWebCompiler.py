import os
import sys
import subprocess
import argparse
import requests
import zipfile
import shutil
import re
import datetime

## ================== EXTERNALS THINGS ================== ##
from lib.externals import PD_LIBRARIES

## ================== EXTERNALS THINGS ================== ##

PROCESSED_ABSTRACTIONS = []

class File:
    def __init__(self, patchPath):
        with open(patchPath, "r") as file:
            self.patchLines = file.readlines()
        

class LineInfo:
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


    def addToUsedObject(self):
        if self.isExternal:
            LibraryClass = PD_LIBRARIES.get(self.library)
            LibraryClass.addToUsed(self.name)
            

class webpdPatch():
    def __init__(self, sourcefile="src/template.c", pdpatch=None, insideaddAbstractions=False) -> None:
        parser = argparse.ArgumentParser(description="Sample script to create personalised webPd patch")
        parser.add_argument('--patch', required=True, help='Patch argument')
        parser.add_argument('--confirm', required=False, help='Confirm if the object is an external')
        parser.add_argument('--clearTmpFiles', required=False, default=False, help='Remove all objects from the patch')
        args = parser.parse_args()
        self.FoundExternals = False
        self.source = sourcefile
        self.clearTmpFiles = args.clearTmpFiles
        self.insideaddAbstractions = insideaddAbstractions
        self.lastPrintedLine = ""
        self.PdWebCompilearPath = os.path.dirname(os.path.realpath(__file__))
        self.PdWebCompilearPath = os.path.dirname(self.PdWebCompilearPath)
        self.PROJECT_ROOT = os.getcwd()
        self.processedAbstractions = []

        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = args.patch

        if not os.path.isabs(self.patch) and not insideaddAbstractions:
            absolutePath = os.path.dirname(os.path.abspath(os.path.join(os.getcwd(), self.patch)))
            self.patch = os.getcwd() + "/" + self.patch
            self.source = os.getcwd() + "/" + self.source
            self.PROJECT_ROOT = absolutePath
            os.chdir(absolutePath)

        # template code
        if not insideaddAbstractions:
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

            if not os.path.exists("webpatch/extra"):
                os.mkdir("webpatch/extra")
            else:
                shutil.rmtree("webpatch/extra")
                os.mkdir("webpatch/extra")

        self.librariesFolder = []
        self.confirm = args.confirm
        self.getPatchPath()
        self.mkBackup()
        self.PatchLines = File(self.patch).patchLines # REMOVE
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

        if not insideaddAbstractions:
            self.emccCompile()
            
        if self.clearTmpFiles == True and not insideaddAbstractions:
            if os.path.exists(".externals"):
                shutil.rmtree(".externals")
        
        if not insideaddAbstractions:
            print("")
        


    def printInfo(self, str):
        # clear the last line
        sys.stdout.write("\033[K")
        sys.stdout.write("\r" + "    " + str)
        sys.stdout.flush()
        self.lastPrintedLine = str

        
    def printError(self, str):
        print("\033[91m" + str + "\033[0m")


    def configForAbstraction(self, abstractionfile):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")
        shutil.copy(abstractionfile, "webpatch/data")


    def copyAllDataFiles(self):
        if not os.path.exists("webpatch/data"):
            os.mkdir("webpatch/data")

        for root, _, files in os.walk("extra"):
            for file in files:
                shutil.copy(os.path.join(root, file), "webpatch/data")


    def checkIfIsSupportedObject(self, patchLine):
        pdClass = patchLine[1]
        if pdClass == "array":
                self.printError("\033[91m" + "    " +
                                      "Visual Arrays are not supported, use [array define] object" + "\033[0m")
                print("")
                sys.exit()


    def findExternals(self):
        for line in enumerate(self.PatchLines):
            lineInfo = LineInfo()
            lineInfo.patchLineIndex = line[0]
            lineInfo.patchLine = line[1]
            lineInfo.isExternal = False
            lineArgs = lineInfo.patchLine.split(" ")
            if len(lineArgs) < 5:
                continue

            self.checkIfIsSupportedObject(lineArgs)
            if (lineArgs[0] == "#X" and lineArgs[1] == "obj" and "/" in lineArgs[4]):
                lineInfo.isExternal = True
                lineInfo.library = lineArgs[4].split("/")[0]
                lineInfo.name = lineArgs[4].split("/")[1].replace("\n", "").replace(";", "").replace(",", "")
                lineInfo.objGenSym = 'class_new(gensym("' + lineArgs[4].split("/")[1].replace("\n", "").replace(";", "") + '")'
                self.printInfo("\033[92m" + "Found External: " + lineInfo.name + "\033[0m")

            elif self.checkIsObjIsSingle(lineArgs):
                lineInfo.isExternal = True
                lineInfo.library = lineArgs[4].replace(";", "").replace("\n", "").replace(",", "")
                lineInfo.name = lineInfo.library
                lineInfo.objGenSym = 'gensym("' + lineInfo.library + '")'
                lineInfo.singleObject = True
                self.printInfo("\033[92m" + "Found External: " + lineInfo.name + "\033[0m")

            else:
                lineInfo.name = lineArgs[4].replace(";", "").replace("\n", "")

            lineInfo.addToUsedObject()
            self.PatchLinesExternalFound.append(lineInfo)


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
                                if lineInfo.name == file.split(".")[0]:
                                    lineInfo.isAbstraction = True
                                    self.configForAbstraction(os.path.join(root, file))
                                  
                else:
                    lineInfo.objFound = False
                    self.printError("    " + "\033[91m" + "Could not find " + lineInfo.library + "\033[0m")
                    
            if lineInfo.isExternal and not lineInfo.objFound and not lineInfo.isAbstraction:
                self.printError("    " + "\033[91m" + "Could not find " + lineInfo.name + " in " + lineInfo.library + "\033[0m")


    def searchCFunction(self, lineInfo, root, file):
        with open(os.path.join(root, file), "r") as file: 
            lineNumber = 0
            for line in file:
                lineNumber += 1
                if lineInfo.objGenSym in line:
                    shutil.copy(file.name, "webpatch/externals")
                    lineInfo.objFound = True
                    lineInfo.genSymIndex = lineNumber
                    with open(file.name, "r") as file2checkFunc:
                        content = file2checkFunc.readlines()
                        for functionline in range(lineNumber, 0, -1):
                            if "void" in content[functionline]:
                                functionTokens = content[functionline].split(" ") 
                                for token in enumerate(functionTokens):
                                    if token[1] == "void":
                                        lineInfo.functionName = functionTokens[token[0] + 1]
                                lineInfo.functionName = lineInfo.functionName.replace("\n", "")
                                lineInfo.functionName = lineInfo.functionName.replace("{", "")
                                lineInfo.functionName = lineInfo.functionName + ";"
                                if "(void)" in lineInfo.functionName:
                                    lineInfo.functionName = lineInfo.functionName.replace("(void)", "()")
                                self.printInfo("\033[92m" + "External Function Found: " + lineInfo.functionName + "\033[0m")
                                print("")
                                break


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
                            functionName = "void " + lineInfo.functionName + "\n"
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
                            functionName = "    " + functionName + "\n"
                            self.templateCode.insert(start_index + 1, functionName)
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
                LibraryClass.PROJECT_ROOT = self.PROJECT_ROOT
                libURL = PD_LIBRARIES.getDownloadURL(libraryName)
                if not isinstance(libURL, str) or LibraryClass is None:
                    print("LibURL is not a string or None" + str(type(libURL)))
                    return None

                if os.path.exists(os.path.join(os.getcwd(), ".externals/" + libraryName)):
                    LibraryClass.folder = os.path.join(os.getcwd(), ".externals/" + libraryName)
                    return True

                response = requests.get(libURL)
                responseJson = response.json()
                sourceCodeLink = responseJson[0]["zipball_url"]
                response = requests.get(sourceCodeLink)
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
            except:
                print("")
                print("")
                self.printError("\033[91m" + str(responseJson["message"]) + "\033[0m")
                print("")
                print("")
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
            self.printError("\033[91m" + str(e) + "\033[0m")
            


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
        PD_LIBRARIES.executeExtraFunction()

    
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



    def emccCompile(self):
        '''
        This is where the code is compiled.
        '''
        # check if emcc is installed and in the path
        if shutil.which("emcc") is None:
            self.printError("\033[91m" + "emcc is not installed or in the path" + "\033[0m")
            print("")
            sys.exit(-1)
        
        self.target = 'webpatch/libpd.js'
        self.libpd_dir = self.PdWebCompilearPath + '/libpd'
        self.src_files = 'webpatch/main.c'
        self.CFLAGS = '-I webpatch/extra/ -I "' + self.libpd_dir + '/pure-data/src" -I "' + self.libpd_dir + '/libpd_wrapper" '
        self.CFLAGS += '-L "' + self.libpd_dir + '/build/libs" -lpd '
        self.LDFLAGS = '-O3  '
        self.LDFLAGS += '-s AUDIO_WORKLET=1 -s WASM_WORKERS=1 -s WASM=1 -s USE_PTHREADS=1 '

        command = ['emcc',
                    "-I", "webpatch/extra/",
                    "-I", '' + self.libpd_dir + '/pure-data/src/',
                    "-I", '' + self.libpd_dir + '/libpd_wrapper/',
                    "-L", '' + self.libpd_dir + '/build/libs/',
                    "-lpd",
                    "-O3",
                    # "-s", "MODULARIZE=1",
                    # "-sEXPORT_NAME=LibPd",
                    "-s", "AUDIO_WORKLET=1",
                    "-s", "WASM_WORKERS=1",
                    "-s", "WASM=1",
                    "-s", "USE_PTHREADS=1",
                    "--preload-file", "webpatch/data/",
                   ]

        command.append(self.src_files)
        command.append("-o")
        command.append(self.target)

        for root, _, files in os.walk("webpatch/externals"):
            for file in files:
                if file.endswith(".c") or file.endswith(".cpp"):
                    command.append(os.path.join(root, file))

            


        print("")

        # print command in dark blue
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
                    # print in dark green ok
                    print("\033[92m" + ("=" * 10) + " Compiled with success " + ("=" * 10) +  "\033[0m")

                                    
                

        process.wait()

        print("")



if __name__ == "__main__":
    webpdPatch()
