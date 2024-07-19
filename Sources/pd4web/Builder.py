import os
import re
import shutil
import subprocess
import sys
import zipfile

import requests
import platform

from .Helpers import DownloadZipFile, pd4web_print
from .Patch import PatchLine
from .Pd4Web import Pd4Web


class GetAndBuildExternals:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web

        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.InitVariables()

        self.Patch = Pd4Web.ProcessedPatch
        self.Libraries = Pd4Web.Libraries
        OK = self.GetObjectsSourceCode()
        if not OK:
            raise Exception("Error: Could not get the externals source code")

        # update setup funciton for
        self.UpdateSetupFunction()
        self.CreateCppCallsExternalFile()

        # ╭──────────────────────────────────────╮
        # │   Here we have the source, now we    │
        # │  need to compile it using CMakeList  │
        # ╰──────────────────────────────────────╯
        self.BuildExternalsObjects()

        # Create filysystem
        self.AddFilesToWebPatch()

        # Create CMakeList
        self.CreateCMakeLists()

        # Compile
        self.ConfigureProject()
        self.CompileProject()
        self.CopyExtraJsFiles()

    def InitVariables(self):
        self.cmakeFile = []
        self.CopyCppFilesToProject()
        self.InitCMakeLists()
        pass

    def InitCMakeLists(self):
        # get name of the project using the project folder name
        self.ProjectName = os.path.basename(self.Pd4Web.PROJECT_ROOT)
        self.cmakeFile.append("cmake_minimum_required(VERSION 3.25)")
        self.cmakeFile.append(f"project({self.ProjectName})\n")

        # Pd Sources
        self.cmakeFile.append("# Pd sources")
        self.cmakeFile.append(
            "include(${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake)")
        self.cmakeFile.append("include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src)")
        self.cmakeFile.append("")


        # Pd4web executable
        self.cmakeFile.append("# Pd4Web executable")
        self.cmakeFile.append(
            "add_executable(pd4web Pd4Web/pd4web.cpp Pd4Web/externals.cpp)")

        includeDirectories = [
            "target_include_directories(pd4web PRIVATE Pd4Web/pure-data/src)"]

        targetLibraries = [
            "target_link_libraries(pd4web PRIVATE",
            "    embind",
            "    libpd",
            ")",
        ]

        targetProperties = [
            "set_target_properties(pd4web PROPERTIES",
            "    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch",
            ")",
        ]
        linkOptions = [
            "target_link_options(pd4web PRIVATE",
            "    -sMODULARIZE=1",
            "    -sEXPORT_NAME='Pd4WebModule'",
            f"    -sINITIAL_MEMORY={self.Pd4Web.MEMORY_SIZE}MB",
            "    -sUSE_PTHREADS=1",
            "    -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency",
            "    -sWASM=1",
            "    -sWASM_WORKERS=1",
            "    -sAUDIO_WORKLET=1",
            ")",
        ]

        # Extend cmakeFile with each section
        self.cmakeFile.extend(includeDirectories)
        self.cmakeFile.extend(targetLibraries)
        self.cmakeFile.extend(targetProperties)
        self.cmakeFile.extend(linkOptions)

    def CopyCppFilesToProject(self):
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web"):
            os.mkdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web")
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
            os.mkdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")
        else:
            if not os.path.isdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                raise Exception("Error: Pd4Web/Externals is not a folder")
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.cpp",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/",
        )
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.hpp",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/",
        )
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../cmake/pd.cmake/pd.cmake",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/pd.cmake",
        )
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../cmake/libpd.cmake",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/libpd.cmake",
        )

    def UpdateSetupFunction(self):
        for usedObjects in self.Pd4Web.usedObjects:
            if usedObjects["Lib"] != "puredata":
                for patchLine in self.Patch.patchLinesProcessed:
                    sameLibrary = patchLine.Library == usedObjects["Lib"]
                    sameObject = patchLine.name == usedObjects["Obj"]
                    if sameLibrary and sameObject:
                        usedObjects["SetupFunction"] = patchLine.functionName

    def GetLibraryData(self, PatchLine: PatchLine):
        libraryClass = PatchLine.GetLibraryData()
        return libraryClass

    def GetLibrarySourceCode(self, PatchLine: PatchLine):
        libraryName = PatchLine.Library
        if self.Libraries.isSupportedLibrary(libraryName):

            if os.path.exists(os.path.join(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libraryName)):
                return True
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                os.makedirs(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")
            libData = self.GetLibraryData(PatchLine)

            # Download Library
            if libData.version != "main" or libData.version != "master":
                if not os.path.exists(self.Pd4Web.APPDATA + "/Externals"):
                    os.mkdir(self.Pd4Web.APPDATA + "/Externals")

                libPath = self.Pd4Web.APPDATA + f"/Externals/{libData.name}/"
                if not os.path.exists(libPath):
                    os.mkdir(libPath)

                if not os.path.exists(self.Pd4Web.APPDATA + f"/Externals/{libData.name}/{libData.version}"):
                    pd4web_print(
                        f"Downloading {libData.name} {libData.version}...",
                        color="green",
                    )
                    libSource = (
                        f"https://github.com/{libData.dev}/{libData.repo}/archive/refs/tags/{libData.version}.zip"
                    )
                    libZip = f"{self.Pd4Web.APPDATA}/Externals/{libData.name}/{libData.version}.zip"
                    DownloadZipFile(libSource, libZip)
                else:
                    libZip = f"{self.Pd4Web.APPDATA}/Externals/{libData.name}/{libData.version}.zip"

                if not os.path.exists(self.PROJECT_ROOT + f"/Pd4Web/Externals/{libData.name}"):
                    with zipfile.ZipFile(libZip, "r") as zip_ref:
                        zip_ref.extractall(
                            self.PROJECT_ROOT + "/Pd4Web/Externals/")
                        extractFolderName = zip_ref.namelist()[0]
                        os.rename(
                            self.PROJECT_ROOT + "/Pd4Web/Externals/" + extractFolderName,
                            self.PROJECT_ROOT + "/Pd4Web/Externals/" + libData.name,
                        )

            else:
                raise Exception(
                    "Error: the use of main or master are not implemented yet")

            libFolder = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libraryName
            PatchLine.GetLibraryExternals(libFolder, libraryName)
            return True

    def RegexSearch(self, PatchLine: PatchLine, functionName, file):
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
            except:
                raise Exception(f"Could not read file: {file} using utf-8")

            patterns = [r"void\s*{}\s*\(\s*void\s*\)", r"void\s+{}\s*\(\s*\)"]
            for pattern in patterns:
                pattern = pattern.format(re.escape(functionName))
                matches = re.finditer(pattern, file_contents, re.DOTALL)
                listMatches = list(matches)
                if len(listMatches) > 0:
                    PatchLine.objFound = True
                    PatchLine.functionName = functionName
                    PatchLine.setupFunction = functionName

    def SearchCFunction(self, patchLine: PatchLine, root, file):
        """
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        """
        functionName = patchLine.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        if "." in functionName:
            functionName = functionName.replace(
                ".", "0x2e")  # else use . as 0x2e
        self.RegexSearch(patchLine, functionName, os.path.join(root, file))
        if not patchLine.objFound:
            functionName = patchLine.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.RegexSearch(patchLine, functionName, os.path.join(root, file))

    def CopyLibAbstraction(self, abstractionfile):
        """
        This function copies the abstractions to webpatch/data folder.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")

        if os.path.exists(self.PROJECT_ROOT + "webpatch/data/" + os.path.basename(abstractionfile)):
            return

        shutil.copy(abstractionfile, self.PROJECT_ROOT + "webpatch/data")

    def GetObjectsSourceCode(self):
        """ """
        for patchLine in self.Patch.patchLinesProcessed:
            if patchLine.isExternal:
                foundLibrary = self.GetLibrarySourceCode(patchLine)
                if foundLibrary:
                    for root, _, files in os.walk(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + patchLine.Library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.SearchCFunction(patchLine, root, file)
                            elif file.endswith(".pd"):
                                if patchLine.name == file.split(".pd")[0]:
                                    patchLine.isAbstraction = True
                                    patchLine.objFound = True
                                    self.CopyLibAbstraction(
                                        os.path.join(root, file))
                else:
                    raise Exception(
                        f"Error: Could not find {patchLine.Library} in the supported libraries")

                if patchLine.objFound and patchLine.isAbstraction:
                    externalSpace = 7 - len(patchLine.name)
                    absName = patchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found Abstraction: {absName}  | Lib: {patchLine.Library}",
                        color="green",
                    )

                elif patchLine.objFound and not patchLine.isAbstraction:
                    externalSpace = 10 - len(patchLine.name)
                    objName = patchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found External: {objName}  | Lib: {patchLine.Library}",
                        color="green",
                    )
                else:
                    raise Exception("Could not find " + patchLine.name)
        return True

    def BuildExternalsObjects(self):
        """
        Configure the project to build the externals objects.
        """
        print()
        usedObjectsName = {}

        for usedObjects in self.Pd4Web.usedObjects:
            library = usedObjects["Lib"]
            if library not in usedObjectsName:
                usedObjectsName[library] = [usedObjects["Obj"]]
            else:
                usedObjectsName[library].append(usedObjects["Obj"])

        externalsTargets = []
        for library, objects in usedObjectsName.items():
            if library == "puredata":
                # TODO: Need to check for extra objects
                continue
            libraryPath = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + library
            pd4web_print(
                f"Create compilation process for {library}\n", color="green")
            for pdobject in objects:
                if pdobject not in self.Pd4Web.externalsLinkLibraries:
                    self.Pd4Web.externalsLinkLibraries.append(pdobject)
                if libraryPath + "/.build" not in self.Pd4Web.externalsLinkLibrariesFolders:
                    self.Pd4Web.externalsLinkLibrariesFolders.append(
                        libraryPath + "/.build")
                target = pdobject.replace("~", "_tilde")
                externalsTargets.append(target)

            CMAKE_LIB_FILE = os.path.normpath(os.path.join(
                self.Pd4Web.PD4WEB_ROOT, "..", "cmake", f"{library}.cmake"))
            shutil.copy(CMAKE_LIB_FILE, self.Pd4Web.PROJECT_ROOT +
                        "/Pd4Web/Externals/")
            self.cmakeFile.append(f"include(Pd4Web/Externals/{library}.cmake)")

        if len(externalsTargets) == 0:
            return
        self.cmakeFile.append(f"\n# Project Externals Libraries")
        targetsString = " ".join(externalsTargets)
        self.cmakeFile.append(
            f"target_link_libraries(pd4web PRIVATE {targetsString})")

    def AddFilesToWebPatch(self):
        self.cmakeFile.append("\n# FileSystem for the Patch")
        string = 'set_target_properties(pd4web PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd")'
        self.cmakeFile.append(string)

        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Audios"):
            self.cmakeFile.append(
                "get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/Audios@/Audios/")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Libs"):
            self.cmakeFile.append(
                "get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/.tmp/@/Libs/")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Extras"):
            self.cmakeFile.append(
                "get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/Extras@/Extras/")'
            )

    def CreateCppCallsExternalFile(self):
        audioConfig = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/config.h"
        with open(audioConfig, "w") as f:
            f.write(
                "// This is automatically generated code from pd4web.py script\n\n")
            # print the PD4WEB_CHS_IN

            f.write(f"#define PD4WEB_CHS_IN {self.Pd4Web.INCHS_COUNT}\n")
            f.write(f"#define PD4WEB_CHS_OUT {self.Pd4Web.OUTCHS_COUNT}\n")
            if self.Pd4Web.GUI:
                f.write(f"#define PD4WEB_GUI true\n")
            else:
                f.write(f"#define PD4WEB_GUI false\n")

        externals = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/externals.cpp"
        with open(externals, "w") as f:
            # Escrever o cabeçalho e a função Pd4WebInitExternals()
            f.write(
                "// This is automatically generated code from pd4web.py script\n\n")

            for usedObjects in self.Pd4Web.usedObjects:
                if usedObjects["SetupFunction"]:
                    f.write(
                        f"extern \"C\" void {usedObjects['SetupFunction']}();\n")

            f.write("\n")
            f.write("void Pd4WebInitExternals() {\n")

            for usedObjects in self.Pd4Web.usedObjects:
                if usedObjects["SetupFunction"]:
                    f.write(f"    {usedObjects['SetupFunction']}();\n")

            f.write("    return;\n")
            f.write("};\n")

    def CreateCMakeLists(self):
        with open(self.Pd4Web.PROJECT_ROOT + "/CMakeLists.txt", "w") as cmakeFile:
            for line in self.cmakeFile:
                cmakeFile.write(line + "\n")

    def ConfigureProject(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        emcmake = self.Pd4Web.Compiler.EMCMAKE
        if platform.system() == "Windows":
            cmake = self.Pd4Web.Compiler.CMAKE
        else:
            cmake = "cmake"
        command = [
            emcmake,
            f'"{cmake}"',
            ".",
            "-B",
            "build",
            "-DPDCMAKE_DIR=Pd4Web/Externals/",
            "-DPD4WEB=ON",
        ]
        if not self.Pd4Web.verbose:
            command.append("--no-warn-unused-cli")
            command.append("-Wno-dev")
        result = subprocess.run(
            command, capture_output=False, text=True).returncode
        if result != 0:
            raise Exception("Error: Could not configure the project")
        os.chdir(cwd)

    def CompileProject(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        cpu_count = os.cpu_count()
        command = [self.Pd4Web.Compiler.CMAKE, "--build", "build"]
        command.append(f"-j{cpu_count}")
        command.append("--target")
        command.append("pd4web")
        pd4web_print(
            f"Compiling project... This may take a while\n", color="green")
        if self.Pd4Web.verbose:
            result = subprocess.run(
                command, capture_output=False, text=True).returncode
        else:
            result = subprocess.run(
                command, capture_output=False, text=True).returncode
        if result != 0:
            raise Exception(
                "Error: Could not compile the project, run on verbose mode to see the error")
        os.chdir(cwd)

    def CopyExtraJsFiles(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        shutil.copy(self.Pd4Web.Patch, self.Pd4Web.PROJECT_ROOT +
                    "/WebPatch/index.pd")
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.threads.js",
            self.Pd4Web.PROJECT_ROOT + "/WebPatch/",
        )
        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.gui.js",
            self.Pd4Web.PROJECT_ROOT + "/WebPatch/",
        )
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.html"):
            shutil.copy(
                self.Pd4Web.PD4WEB_ROOT + "/../index.html",
                self.Pd4Web.PROJECT_ROOT + "/WebPatch/",
            )

        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/index.html"):
            with open(self.Pd4Web.PROJECT_ROOT + "/index.html", "w") as f:
                f.write("<!doctype html>\n")
                f.write('<html lang="en-us">\n')
                f.write("  <body>\n")
                f.write("    <script>\n")
                f.write('      window.location.href = "WebPatch/index.html";\n')
                f.write("    </script>\n")
                f.write("  </body>\n")
                f.write("</html>\n")

        os.chdir(cwd)

    def __repr__(self) -> str:
        return f"< GetCode Object >"

    def __str__(self) -> str:
        return self.__repr__()
