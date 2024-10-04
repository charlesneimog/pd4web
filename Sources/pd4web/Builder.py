import os
import re
import shutil
import subprocess
import pygit2

from .Helpers import pd4web_print
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
        self.cmakeFile.append('set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")')
        self.cmakeFile.append("include(${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake)")
        self.cmakeFile.append("include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src)")
        self.cmakeFile.append("")

        # Pd4web executable
        self.cmakeFile.append("# Pd4Web executable")

        self.cmakeFile.append("add_executable(pd4web Pd4Web/pd4web.cpp Pd4Web/externals.cpp)")

        includeDirectories = ["target_include_directories(pd4web PRIVATE Pd4Web/pure-data/src)"]

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
            "    -sPTHREAD_POOL_SIZE=4",
            "    -sWASM=1",
            "    -sWASM_WORKERS=1",
            "    -sAUDIO_WORKLET=1",
            # "    -sASYNCIFY",
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
            self.Pd4Web.PD4WEB_LIBRARIES + "/pd.cmake/pd.cmake",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/pd.cmake",
        )
        shutil.copy(
            self.Pd4Web.PD4WEB_LIBRARIES + "/libpd.cmake",
            self.Pd4Web.PROJECT_ROOT + "/Pd4Web/libpd.cmake",
        )

    def UpdateSetupFunction(self):
        for usedObjects in self.Pd4Web.usedObjects:
            if usedObjects["Lib"] != "pure-data":
                for patchLine in self.Patch.patchLinesProcessed:
                    sameLibrary = patchLine.library == usedObjects["Lib"]
                    sameObject = patchLine.name == usedObjects["Obj"]
                    if sameLibrary and sameObject:
                        usedObjects["SetupFunction"] = patchLine.functionName

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
            functionName = functionName.replace(".", "0x2e")  # else use . as 0x2e
        self.RegexSearch(patchLine, functionName, os.path.join(root, file))
        if not patchLine.objFound:
            functionName = patchLine.name
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.RegexSearch(patchLine, functionName, os.path.join(root, file))

    def GetObjectsSourceCode(self):
        for patchLine in self.Patch.patchLinesProcessed:
            if patchLine.isExternal:
                foundLibrary = self.Libraries.GetLibrarySourceCode(patchLine.library)

                if foundLibrary:
                    for root, _, files in os.walk(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + patchLine.library):
                        for file in files:
                            if file.endswith(".c") or file.endswith(".cpp"):
                                self.SearchCFunction(patchLine, root, file)
                else:
                    raise Exception(f"Error: Could not find {patchLine.library} in the supported libraries")

                if patchLine.objFound and patchLine.isAbstraction:
                    externalSpace = 20 - len(patchLine.name)
                    absName = patchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found Abstraction: {absName}  | Lib: {patchLine.library}",
                        color="green",
                        silence=self.Pd4Web.SILENCE,
                        pd4web=self.Pd4Web.PD_EXTERNAL,
                    )

                elif patchLine.objFound and not patchLine.isAbstraction:
                    externalSpace = 20 - len(patchLine.name)
                    objName = patchLine.name + (" " * externalSpace)
                    pd4web_print(
                        f"Found External: {objName}  | Lib: {patchLine.library}",
                        color="green",
                        silence=self.Pd4Web.SILENCE,
                        pd4web=self.Pd4Web.PD_EXTERNAL,
                    )
                else:
                    raise Exception("Could not find " + patchLine.name)
        return True

    def BuildExternalsObjects(self):
        """
        Configure the project to build the externals objects.
        """
        usedObjectsName = {}

        for usedObjects in self.Pd4Web.usedObjects:
            library = usedObjects["Lib"]
            if library not in usedObjectsName:
                usedObjectsName[library] = [usedObjects["Obj"]]
            else:
                usedObjectsName[library].append(usedObjects["Obj"])

        externalsTargets = []
        print()
        for library, objects in usedObjectsName.items():
            if library == "pure-data":
                # TODO: Need to check for extra objects
                continue
            libraryPath = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + library
            pd4web_print(
                f"Creating compilation process for {library}\n",
                color="green",
                silence=self.Pd4Web.SILENCE,
                pd4web=self.Pd4Web.PD_EXTERNAL,
            )
            for pdobject in objects:
                if pdobject not in self.Pd4Web.externalsLinkLibraries:
                    self.Pd4Web.externalsLinkLibraries.append(pdobject)
                if libraryPath + "/.build" not in self.Pd4Web.externalsLinkLibrariesFolders:
                    self.Pd4Web.externalsLinkLibrariesFolders.append(libraryPath + "/.build")
                target = pdobject.replace("~", "_tilde")

                externalsTargets.append(target)

            CMAKE_LIB_FILE = os.path.normpath(
                os.path.join(self.Pd4Web.PD4WEB_ROOT, "..", "Libraries", f"{library}.cmake")
            )
            shutil.copy(CMAKE_LIB_FILE, self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/")
            self.cmakeFile.append(f"include(Pd4Web/Externals/{library}.cmake)")

        if len(externalsTargets) == 0:
            return
        self.cmakeFile.append(f"\n# Project Externals Libraries")
        targetsString = " ".join(externalsTargets)
        self.cmakeFile.append(f"target_link_libraries(pd4web PRIVATE {targetsString})")

    def AddFilesToWebPatch(self):
        self.cmakeFile.append("\n# FileSystem for the Patch")
        string = 'set_target_properties(pd4web PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd")'
        self.cmakeFile.append(string)

        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Audios"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Audios@/Audios/")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/.tmp"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/.tmp@/Libs/")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Extras"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Extras@/Extras/")'
            )

    def CreateCppCallsExternalFile(self):
        audioConfig = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/config.h"

        # ╭──────────────────────────────────────╮
        # │             Config File              │
        # ╰──────────────────────────────────────╯
        with open(audioConfig, "w") as f:
            # Audio Config
            if self.Pd4Web.OUTCHS_COUNT == 0:
                self.Pd4Web.OUTCHS_COUNT = 2

            f.write("// This is automatically generated code from pd4web.py script\n\n")

            f.write(f"// Project Name\n")
            projectName = os.path.basename(self.Pd4Web.PROJECT_ROOT)
            f.write(f'#define PD4WEB_PROJECT_NAME "{projectName}"\n\n')

            f.write(f"// Audio Config\n")
            f.write(f"#define PD4WEB_CHS_IN {self.Pd4Web.INCHS_COUNT}\n")
            f.write(f"#define PD4WEB_CHS_OUT {self.Pd4Web.OUTCHS_COUNT}\n")
            f.write(f"#define PD4WEB_SR 48000\n\n")

            # Gui Interface
            f.write(f"// GUI Interface\n")
            if self.Pd4Web.GUI:
                f.write(f"#define PD4WEB_GUI true\n")
            else:
                f.write(f"#define PD4WEB_GUI false\n")
            f.write(f"#define PD4WEB_FPS {self.Pd4Web.FPS}\n")

            if self.Pd4Web.AUTO_THEME:
                f.write(f"#define PD4WEB_AUTO_THEME true\n\n")
            else:
                f.write(f"#define PD4WEB_AUTO_THEME false\n\n")

            f.write(f"// Midi\n")
            if self.Pd4Web.MIDI:
                f.write(f"#define PD4WEB_MIDI true\n")
            else:
                f.write(f"#define PD4WEB_MIDI false\n")

        # ╭──────────────────────────────────────╮
        # │            External File             │
        # ╰──────────────────────────────────────╯
        externals = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/externals.cpp"
        with open(externals, "w") as f:
            # Escrever o cabeçalho e a função Pd4WebInitExternals()
            f.write("// This is automatically generated code from pd4web.py script\n\n")

            for usedObjects in self.Pd4Web.usedObjects:
                if usedObjects["SetupFunction"]:
                    f.write(f"extern \"C\" void {usedObjects['SetupFunction']}();\n")

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
        cmake = self.Pd4Web.Compiler.CMAKE
        command = [
            emcmake,
            cmake,
            ".",
            "-B",
            "build",
            "-DPDCMAKE_DIR=Pd4Web/Externals/",
            "-DPD4WEB=ON",
            "-G",
            "Ninja",
        ]
        if self.Pd4Web.verbose:
            pd4web_print(" ".join(command), color="green", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL)
        result = subprocess.run(command, capture_output=not self.Pd4Web.verbose, text=True).returncode
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
            f"Compiling project... This may take a while\n",
            color="green",
            silence=self.Pd4Web.SILENCE,
            pd4web=self.Pd4Web.PD_EXTERNAL,
        )

        if self.Pd4Web.verbose:
            pd4web_print(" ".join(command), color="green", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL)

        result = subprocess.run(command, capture_output=not self.Pd4Web.verbose, text=True).returncode
        if result != 0:
            raise Exception("Error: Could not compile the project, run on verbose mode to see the error")
        os.chdir(cwd)

    def CopyExtraJsFiles(self):
        cwd = os.getcwd()

        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.gitignore",
            self.Pd4Web.PROJECT_ROOT + "/.gitignore",
        )

        os.chdir(self.Pd4Web.PROJECT_ROOT)
        # shutil.copy(self.Pd4Web.Patch, self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.pd")

        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../favicon.ico",
            self.Pd4Web.PROJECT_ROOT + "/",
        )

        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.threads.js",
            self.Pd4Web.PROJECT_ROOT + "/WebPatch/",
        )

        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.midi.js",
            self.Pd4Web.PROJECT_ROOT + "/WebPatch/",
        )

        shutil.copy(
            self.Pd4Web.PD4WEB_ROOT + "/../pd4web.style.css",
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
