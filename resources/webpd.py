import os
import sys
import argparse
import requests
import zipfile
import shutil
import datetime

## ================== EXTERNALS THINGS ================== ##
from lib.externals import PD_LIBRARIES

## ================== EXTERNALS THINGS ================== ##

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


class webpdPatch():
    def __init__(self, sourcefile="src/template.c", pdpatch=None, insideaddAbstractions=False) -> None:
        parser = argparse.ArgumentParser(description="Sample script to create personalised webPd patch")
        parser.add_argument('--patch', required=True, help='Patch argument')
        parser.add_argument('--confirm', required=False, help='Confirm if the object is an external')
        parser.add_argument('--clearTmpFiles', required=False, default=True, help='Remove all objects from the patch')
        args = parser.parse_args()
        self.FoundExternals = False
        self.source = sourcefile
        self.clearTmpFiles = args.clearTmpFiles
        self.insideaddAbstractions = insideaddAbstractions
        self.lastPrintedLine = ""
        
        # template code
        with open(self.source, "r") as file:
            self.templateCode = file.readlines()

        # check if webpatch/externals and webpatch/extras exists
        if not os.path.exists("externals"):
            os.mkdir("externals")

        if not os.path.exists("webpatch"):
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

        if pdpatch is not None:
            self.patch = pdpatch
        else:
            self.patch = args.patch

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
        
        if self.clearTmpFiles == True:
            if os.path.exists("externals"):
                shutil.rmtree("externals")

        # copy index.html to webpatch
        shutil.copy("src/index.html", "webpatch/index.html")
        shutil.copy("src/helpers.js", "webpatch/helpers.js")
        shutil.copy("src/enable-threads.js", "webpatch/enable-threads.js")

        print("")

    def printInfo(self, str):
        # clear the last line
        sys.stdout.write("\033[K")
        sys.stdout.write("\r" + str)
        sys.stdout.flush()
        self.lastPrintedLine = str

        
    def printError(self, str):
        # print in red
        print("")
        print("\033[91m" + str + "\033[0m")



    def configForAbstraction(self, abstractionfile):
        # copy the abstraction file to webpatch/libs
        # check if libs folder exists
        if not os.path.exists("webpatch/libs"):
            os.mkdir("webpatch/libs")
        shutil.copy(abstractionfile, "webpatch/libs")


    def findExternals(self):
        for line in enumerate(self.PatchLines):
            lineInfo = LineInfo()
            lineInfo.patchLineIndex = line[0]
            lineInfo.patchLine = line[1]
            lineInfo.isExternal = False
            lineArgs = lineInfo.patchLine.split(" ")
            if len(lineArgs) < 5:
                continue
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
                    for root, _, files in os.walk("externals/" + lineInfo.library):
                        for file in files:
                            if file.endswith(".c"):
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
                                                        functionTokens = content[functionline].split(" ") # [1].replace("\n", "")
                                                        for token in enumerate(functionTokens):
                                                            if token[1] == "void":
                                                                lineInfo.functionName = functionTokens[token[0] + 1]
                                                        lineInfo.functionName = lineInfo.functionName.replace("\n", "")
                                                        lineInfo.functionName = lineInfo.functionName.replace("{", "")
                                                        lineInfo.functionName = lineInfo.functionName + ";"
                                                        if "(void)" in lineInfo.functionName:
                                                            lineInfo.functionName = lineInfo.functionName.replace("(void)", "()")
                                                        self.printInfo("\033[92m" + "Found function: " + lineInfo.functionName + "\033[0m")
                                                        print("")
                                                        break
                            elif file.endswith(".pd"):
                                if lineInfo.name == file.split(".")[0]:
                                    lineInfo.isAbstraction = True
                                    self.configForAbstraction(os.path.join(root, file))
                                  
                else:
                    lineInfo.objFound = False
                    self.printError("\033[91m" + "Could not find " + lineInfo.library + "\033[0m")
                    
            if lineInfo.isExternal and not lineInfo.objFound and not lineInfo.isAbstraction:
                self.printError("\033[91m" + "Could not find " + lineInfo.name + " in " + lineInfo.library + "\033[0m")


    def addObjSetup(self):
        '''
        This function will add the obj_setup() inside the main.c file
        '''
        for lineInfo in self.PatchLinesExternalFound:
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
                        alreadyAdded = False
                        for line in self.templateCode:
                            if functionName in line:
                                alreadyAdded = True
                                break
                        if not alreadyAdded:
                            self.templateCode.insert(start_index + 1, functionName)
                        break
        return True


    def saveMainFile(self):
        with open("webpatch/main.c", "w") as file:
            for line in self.templateCode:
                file.write(line)


    def createHeaderFile(self):
        '''
        In the same folder where self.source is, create one externals.h file.
        '''
        folder = os.path.dirname(self.source)
        if os.path.exists(os.path.join(folder, "externals.h")):
            os.remove(os.path.join(folder, "externals.h"))
        with open(os.path.join(folder, "externals.h"), "w") as file:
            for object in self.PatchLinesExternalFound:
                if object.isExternal:
                    file.write("void " + object.functionName + "\n")


    def usedLibraries(self, libraryName):
        '''
        It adds the used libraries for the patch, it can be accessed by the extra functions.
        '''
        if libraryName not in PD_LIBRARIES.UsedLibrariesNames:
            PD_LIBRARIES.UsedLibrariesNames.append(libraryName)
            PD_LIBRARIES.UsedLibraries.append(PD_LIBRARIES.get(libraryName))


    def downloadExternalLibrarySrc(self, libraryName):
        if libraryName in PD_LIBRARIES.LibraryNames:
            self.usedLibraries(libraryName)
            LibraryClass = PD_LIBRARIES.get(libraryName)
            libURL = PD_LIBRARIES.getDownloadURL(libraryName)
            if not isinstance(libURL, str):
                print("LibURL is not a string or None" + str(type(libURL)))
                return None

            # check if os.path.join(os.getcwd(), "externals/" + libraryName) exists
            if os.path.exists(os.path.join(os.getcwd(), "externals/" + libraryName)):
                LibraryClass.folder = os.path.join(os.getcwd(), "externals/" + libraryName)
                return True

            response = requests.get(libURL)
            responseJson = response.json()
            sourceCodeLink = responseJson[0]["zipball_url"]
            response = requests.get(sourceCodeLink)
            if not os.path.exists("externals"):
                os.mkdir("externals")
            with open("externals/" + libraryName + ".zip", "wb") as file:
                file.write(response.content)

            with zipfile.ZipFile("externals/" + libraryName + ".zip", 'r') as zip_ref:
                zip_ref.extractall("externals")
                extractFolderName = zip_ref.namelist()[0]
                os.rename("externals/" + extractFolderName, "externals/" + libraryName)

            LibraryClass.folder = os.path.join(os.getcwd(), "externals/" + libraryName)
            self.librariesFolder.append(os.path.join(os.getcwd(), "externals/" + libraryName))
            os.remove("externals/" + libraryName + ".zip")
            return True
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
        shutil.copy(self.patch, ".backup/" + backPatchName)


    def savePdPatchModified(self):
        with open('webpatch/index.pd', "w") as file:
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
                    file.write("#X declare -path libs;\n")
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
        # list all files in webpatch/libs
        for dir, _, files in os.walk("webpatch/libs"):
            for patchfile in files:
                if patchfile.endswith(".pd"):
                    print("")
                    self.printInfo("\033[92m" + "Found Abstraction: " + patchfile + "\033[0m")
                    webpdPatch(sourcefile="webpatch/main.c", pdpatch="webpatch/libs/" + patchfile, insideaddAbstractions=True)
                    self.removeLibraryPrefix(dir + "/" + patchfile)


if __name__ == "__main__":
    webpdPatch()
