import os
import re
import shutil
import subprocess
import sys
import zipfile

import requests

from .Helpers import getPrintValue, pd4web_print
from .Patch import PatchLine
from .Pd4Web import Pd4Web


class GetAndBuildExternals():
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web
 
        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.InitVariables()

        self.Patch = Pd4Web.ProcessedPatch
        self.Libraries = Pd4Web.Libraries
        OK = self.GetExternalsSourceCode()
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
        self.AddPd4WebRequirements()
        pass

    def InitCMakeLists(self):
        # get name of the project using the project folder name
        self.ProjectName = os.path.basename(self.Pd4Web.PROJECT_ROOT)
        self.cmakeFile.append("cmake_minimum_required(VERSION 3.25)")
        self.cmakeFile.append(f"project({self.ProjectName})\n")

        self.cmakeFile.append("# LibPd and PureData")
        self.cmakeFile.append("cmake_policy(SET CMP0077 NEW)")
        self.cmakeFile.append("set(PD_EXTRA off)")
        self.cmakeFile.append("add_subdirectory(Pd4Web/libpd)")
        self.cmakeFile.append("target_compile_options(libpd_static PUBLIC -w)\n")

        # Pd4web executable
        self.cmakeFile.append("# Pd4Web executable")
        self.cmakeFile.append("add_executable(pd4web Pd4Web/pd4web.cpp Pd4Web/externals.cpp)")

        includeDirectories = [
            "target_include_directories(pd4web PRIVATE",
            "    Pd4Web/libpd/libpd_wrapper",
            "    Pd4Web/libpd/pure-data/src",
            ")"
        ]

        targetLibraries = [
            "target_link_libraries(pd4web PRIVATE",
            "    embind",
            "    libpd_static",
            ")"
        ]

        targetProperties = [
            "set_target_properties(pd4web PROPERTIES",
            "    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch",
            ")"
        ]
        linkOptions = [
            "target_link_options(pd4web PRIVATE",
            "    -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency",
            "    -sMODULARIZE=1",
            "    -sEXPORT_NAME='Pd4WebModule'",
            "    -sAUDIO_WORKLET=1",
            "    -sUSE_PTHREADS=1",
            "    -sWASM_WORKERS=1",
            "    -sWASM=1",
            ")"
        ]

        # Extend cmakeFile with each section
        self.cmakeFile.extend(includeDirectories)
        self.cmakeFile.extend(targetLibraries)
        self.cmakeFile.extend(targetProperties)
        self.cmakeFile.append("target_compile_options(pd4web PRIVATE -sUSE_PTHREADS=1 -sWASM_WORKERS=1)")
        self.cmakeFile.extend(linkOptions)



    def CopyCppFilesToProject(self):
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web"):
            os.mkdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web")
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
            os.mkdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")
        else:
            if not os.path.isdir(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                raise Exception("Error: Pd4Web/Externals is not a folder")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../pd4web.cpp", self.Pd4Web.PROJECT_ROOT + "/Pd4Web/")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../pd4web.hpp", self.Pd4Web.PROJECT_ROOT + "/Pd4Web/")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../cmake/pd.cmake/pd.cmake", self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/pd.cmake")

    def AddPd4WebRequirements(self):
        current_dir = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        # TODO: Check if there is a tag specified
        os.system("git submodule add https://github.com/charlesneimog/libpd.git Pd4Web/libpd")
        os.system("git submodule update --init --recursive --depth 1")
        os.chdir(current_dir)

    def UpdateSetupFunction(self):
        for usedObjects in self.Pd4Web.UsedObjects:
            if usedObjects["Lib"] != "puredata":
                for patchLine in self.Patch.PatchLinesProcessed:
                    sameLibrary = patchLine.Library == usedObjects["Lib"]
                    sameObject = patchLine.name == usedObjects["Obj"]
                    if sameLibrary and sameObject:
                        usedObjects["SetupFunction"] = patchLine.functionName


    def GetLibraryData(self, PatchLine: PatchLine):
        libraryClass = PatchLine.GetLibraryData()
        return libraryClass

    def DownloadLibrarySourceCode(self, PatchLine: PatchLine):
        libraryName = PatchLine.Library
        if self.Libraries.isSupportedLibrary(libraryName):

            if os.path.exists(os.path.join(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libraryName)):
                return True
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                os.makedirs(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")
            libraryData = self.GetLibraryData(PatchLine)
            if libraryData.Source:
                link = libraryData.GetLinkForDownload()

                folder = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libraryName 
                if not os.path.exists(folder):
                    print(f"Creating folder {folder}")
                    os.makedirs(folder)

                os.system("git submodule add " + link + " " + folder)
                os.system(f"git clone {link} {folder}")
                os.chdir(folder)
                tagName = libraryData.Version
                os.system(f"git checkout {tagName}")
                os.system('git submodule update --init --recursive --depth 1')
            else:
                LibraryDirectLink = libraryData.DirectLink
                response = requests.get(LibraryDirectLink, stream=True)
                if response.status_code != 200:
                    raise Exception(f"Error: {response.status_code}")
                total_size = int(response.headers.get('content-length', 0))
                chunk_size = 1024
                num_bars = 40
                pd4web_print(f"Downloading {libraryName}...", color="yellow")
                with open(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libraryName + ".zip", 'wb') as file: 
                        downloaded_size = 0
                        for data in response.iter_content(chunk_size):
                            file.write(data)
                            downloaded_size += len(data)
                            if total_size:
                                progress = downloaded_size / total_size
                                num_hashes = int(progress * num_bars)
                                progress_bar = '#' * num_hashes + '-' * (num_bars - num_hashes)
                                sys.stdout.write(f'\r    🟡 |{progress_bar}| {progress:.2%}')
                            else:
                                num_hashes = int(downloaded_size / chunk_size) % num_bars
                                progress_bar = '#' * num_hashes + '-' * (num_bars - num_hashes)
                                sys.stdout.write(f'\r    🟡 |{progress_bar}| {downloaded_size} bytes')
                            sys.stdout.flush()


                print()
                with zipfile.ZipFile(self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals/" + libraryName + ".zip", "r") as zip_ref:
                    zip_ref.extractall(self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(
                        self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals/" + extractFolderName,
                        self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals/" + libraryName,
                    )
                os.remove(self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals/" + libraryName + ".zip")

            LibraryFolder = self.Pd4Web.PD4WEB_ROOT + "/Pd4Web/Externals/" + libraryName
            PatchLine.GetLibraryExternals(LibraryFolder, libraryName)
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

    def SearchCFunction(self, PatchLine : PatchLine, root, file):
        """
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        """
        functionName = PatchLine.name
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        # NOTE: Maybe there is another cases like this
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e")  # else use . as 0x2e
        self.RegexSearch(PatchLine, functionName, os.path.join(root, file))
        if not PatchLine.objFound:
            functionName = PatchLine.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.RegexSearch(PatchLine, functionName, os.path.join(root, file))

    def CopyLibAbstraction(self, abstractionfile):
        """
        This function copies the abstractions to webpatch/data folder.
        """
        if not os.path.exists(self.PROJECT_ROOT + "webpatch/data"):
            os.mkdir(self.PROJECT_ROOT + "webpatch/data")

        if os.path.exists(self.PROJECT_ROOT + "webpatch/data/" + os.path.basename(abstractionfile)):
            return

        shutil.copy(abstractionfile, self.PROJECT_ROOT + "webpatch/data")


    def GetExternalsSourceCode(self):
        for patchLine in self.Patch.PatchLinesProcessed:
            if patchLine.isExternal:
                foundLibrary = self.DownloadLibrarySourceCode(patchLine)
                if foundLibrary:
                    for root, _, files in os.walk(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + patchLine.Library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.SearchCFunction(patchLine, root, file)
                            elif file.endswith(".pd"):
                                if patchLine.name == file.split(".pd")[0]:
                                    patchLine.isAbstraction = True
                                    patchLine.objFound = True
                                    self.CopyLibAbstraction(os.path.join(root, file))
                else:
                    raise Exception(f"Error: Could not find {patchLine.Library} in the supported libraries")

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
        print()
        usedObjectsName = {}
        
        for usedObjects in self.Pd4Web.UsedObjects:
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
            pd4web_print(f"Create compilation process for {library}\n", color="green")
            for pdobject in objects:
                if pdobject not in self.Pd4Web.ExternalsLinkLibraries:
                    self.Pd4Web.ExternalsLinkLibraries.append(pdobject)
                if libraryPath + "/.build" not in self.Pd4Web.ExternalsLinkLibrariesFolders:
                    self.Pd4Web.ExternalsLinkLibrariesFolders.append(libraryPath + "/.build")
                target = pdobject.replace("~", "_tilde")
                externalsTargets.append(target)

            CMAKE_LIB_FILE = os.path.normpath(os.path.join(self.Pd4Web.PD4WEB_ROOT, "..", "cmake", f"{library}.cmake"))
            shutil.copy(CMAKE_LIB_FILE, self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/")
            self.cmakeFile.append(f"include(Pd4Web/Externals/{library}.cmake)")

        if len(externalsTargets) == 0:
            return
        self.cmakeFile.append(f"\n# Project Externals Libraries")
        targetsString = " ".join(externalsTargets)
        self.cmakeFile.append(f"target_link_libraries(pd4web PRIVATE {targetsString})")

    def AddFilesToWebPatch(self):
        self.cmakeFile.append('\n# FileSystem for the Patch')
        patchName = self.Pd4Web.PROJECT_PATCH
        string = 'set_target_properties(pd4web PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/'
        string += patchName + '@/index.pd")'
        self.cmakeFile.append(string)

        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Audios"):
            self.cmakeFile.append('get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)')
            self.cmakeFile.append('set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/Audios@/Audios/")')
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Libs"):
            self.cmakeFile.append('get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)')
            self.cmakeFile.append('set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/Libs@/Libs/")')
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Libs"):
            self.cmakeFile.append('get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)')
            self.cmakeFile.append('set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/Extras@/Extras/")')


    def CreateCppCallsExternalFile(self):
        path = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/externals.cpp"
        with open(path, 'w') as f:
            # Escrever o cabeçalho e a função Pd4WebInitExternals()
            f.write("// This is automatically generated code from pd4web.py script\n\n")
            for usedObjects in self.Pd4Web.UsedObjects:
                if usedObjects["SetupFunction"]:
                    f.write(f"extern \"C\" void {usedObjects['SetupFunction']}();\n")

            f.write("\n\n")
            f.write("void Pd4WebInitExternals() {\n")
            f.write("    // Call the _setup functions for all externals\n")

            for usedObjects in self.Pd4Web.UsedObjects:
                if usedObjects["SetupFunction"]:
                    f.write(f"    {usedObjects['SetupFunction']}();\n")
            
            f.write("    return;\n")
            f.write("};\n")

    def CreateCMakeLists(self):
        print(f"{self.Pd4Web.PROJECT_ROOT}/CMakeLists.txt")
        with open(self.Pd4Web.PROJECT_ROOT + "/CMakeLists.txt", "w") as cmakeFile:
            for line in self.cmakeFile:
                cmakeFile.write(line + "\n")


    def ConfigureProject(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        emcmake = self.Pd4Web.Compiler.EMCMAKE
        command = [emcmake, "cmake", ".", "-B", "build", "-DPDCMAKE_DIR=Pd4Web/Externals/", "-DPD4WEB=ON"]
        # command.append("-DCMAKE_VERBOSE_MAKEFILE=ON")
        command.append("--no-warn-unused-cli")
        if not self.Pd4Web.Verbose:
            command.append("-Wno-dev")
        result = os.system(" ".join(command))
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
        result = os.system(" ".join(command))
        if not self.Pd4Web.Verbose:
            command.append("-Wno-dev")
            command.append("-W")
        if result != 0:
            raise Exception("Error: Could not compile the project")
        os.chdir(cwd)

    def CopyExtraJsFiles(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        shutil.copy(self.Pd4Web.Patch, self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.pd")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../pd4web.threads.js", self.Pd4Web.PROJECT_ROOT + "/WebPatch/")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../pd4web.gui.js", self.Pd4Web.PROJECT_ROOT + "/WebPatch/")
        shutil.copy(self.Pd4Web.PD4WEB_ROOT + "/../index.html", self.Pd4Web.PROJECT_ROOT + "/WebPatch/")
        with open(self.Pd4Web.PROJECT_ROOT + "/index.html", "w") as f:
            f.write("<!doctype html>\n")
            f.write("<html lang=\"en-us\">\n")
            f.write("  <body>\n")
            f.write("    <script>\n")
            f.write("      window.location.href = \"WebPatch/index.html\";\n")
            f.write("    </script>\n")
            f.write("  </body>\n")
            f.write("</html>\n")
        os.chdir(cwd)

    def __repr__(self) -> str:
        return f"< GetCode Object >"

    def __str__(self) -> str:
        return self.__repr__()

