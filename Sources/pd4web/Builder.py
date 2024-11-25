import os
import re
import shutil
import subprocess
import importlib.metadata as importlib_metadata
import yaml

# from .Patch import PatchLine
from .Pd4Web import Pd4Web


class GetAndBuildExternals:
    def __init__(self, Pd4Web: Pd4Web):
        self.Pd4Web = Pd4Web

        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.InitVariables()

        self.Patch = Pd4Web.ProcessedPatch
        self.Libraries = Pd4Web.Libraries

        OK = self.getObjectsSourceCode()  # I need to know what is the _setup function name
        if not OK:
            self.Pd4Web.exception("Error: Could not get the externals source code")

        # update setup funciton for
        # self.UpdateSetupFunction()
        self.CreateCppCallsExternalFile()

        # ╭──────────────────────────────────────╮
        # │   Here we have the source, now we    │
        # │  need to compile it using CMakeList  │
        # ╰──────────────────────────────────────╯
        self.buildExternalsObjects()

        # Create filysystem
        self.AddFilesToWebPatch()

        # Create CMakeList
        self.CreateCMakeLists()

        # Compile
        self.ConfigureProject()
        self.CompileProject()
        self.CopyExtraJsFiles()
        
        # Save the project versions
        self.SaveProjectVersions()

    def InitVariables(self):
        self.cmakeFile = []
        self.CopyCppFilesToProject()
        self.InitCMakeLists()
        pass

    def InitCMakeLists(self):
        # get name of the project using the project folder name
        if "#" in self.Pd4Web.PROJECT_ROOT:
            self.ProjectName = f'"{os.path.basename(self.Pd4Web.PROJECT_ROOT)}"'
        else:
            self.ProjectName = os.path.basename(self.Pd4Web.PROJECT_ROOT)
        self.cmakeFile.append("cmake_minimum_required(VERSION 3.25)")
        self.cmakeFile.append(f'project("{self.ProjectName}")\n')

        # Pd Sources
        self.cmakeFile.append("# Pd sources")
        self.cmakeFile.append('set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")')
        self.cmakeFile.append('include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake")')
        self.cmakeFile.append('include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pd.cmake")')
        self.cmakeFile.append('set(PDCMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" CACHE STRING "" FORCE)')
        self.cmakeFile.append(
            'set(PD4WEB_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/" CACHE STRING "" FORCE)'
        )
        self.cmakeFile.append('include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src")')
        self.cmakeFile.append("add_definitions(-DPDTHREADS)")
        self.cmakeFile.append("")

        # Debug option
        self.cmakeFile.append('if(CMAKE_BUILD_TYPE STREQUAL "Debug")')
        self.cmakeFile.append('    message(WARNING "Building in Debug mode")')
        self.cmakeFile.append('    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")')
        self.cmakeFile.append('    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")')
        self.cmakeFile.append("else()")
        self.cmakeFile.append(
            '    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 -flto -pthread -matomics -mbulk-memory -msimd128")'
        )
        self.cmakeFile.append(
            '    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128")'
        )
        self.cmakeFile.append("endif()")
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
            '    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/WebPatch"',
            ")",
        ]
        linkOptions = [
            "target_link_options(pd4web PRIVATE",
            "    -sMODULARIZE=1",
            "    -sEXPORT_NAME='Pd4WebModule'",
            f"    -sINITIAL_MEMORY={self.Pd4Web.MEMORY_SIZE}MB",
            "    -sUSE_PTHREADS=1",
            "    -sPTHREAD_POOL_SIZE=4",
            "    -sWASMFS=1",
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
                self.Pd4Web.exception("Error: Pd4Web/Externals is not a folder")
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

    def regexSearch(self, obj: dict, functionName, file):
        """
        This search for the setup function using regex.
        """
        with open(
            file,
            "r",
            encoding="utf-8",
            errors="ignore",
        ) as c_file:
            try:
                file_contents = c_file.read()
            except:
                self.Pd4Web.exception(f"Could not read file: {file} using utf-8")

            patterns = [r"void\s*{}\s*\(\s*void\s*\)", r"void\s+{}\s*\(\s*\)"]
            for pattern in patterns:
                pattern = pattern.format(re.escape(functionName))
                matches = re.finditer(pattern, file_contents, re.DOTALL)
                listMatches = list(matches)
                if len(listMatches) > 0:
                    obj["SetupFunction"] = functionName
                    self.Pd4Web.print(
                        f"Found setup function: {functionName}",
                        color="green",
                        silence=self.Pd4Web.SILENCE,
                        pd4web=self.Pd4Web.PD_EXTERNAL,
                    )

    def searchCFunction(self, obj: dict, root, file):
        """
        This function search for the setup function in the C file using different
        ways. Here you can add more ways to search for the setup function.
        """
        functionName = obj["Obj"]
        functionName = functionName.replace("~", "_tilde")
        functionName += "_setup"
        if "." in functionName:
            functionName = functionName.replace(".", "0x2e")  # else use . as 0x2e
        self.regexSearch(obj, functionName, os.path.join(root, file))
        if obj["SetupFunction"] == "":
            functionName = obj["Obj"]
            functionName = functionName.replace("~", "_tilde")
            functionName = "setup_" + functionName
            if "." in functionName:
                functionName = functionName.replace(".", "0x2e")
            self.regexSearch(obj, functionName, os.path.join(root, file))

    def getObjectsSourceCode(self):
        print()
        for obj in self.Pd4Web.usedObjects:
            libName = obj["Lib"]
            foundLibrary = self.Libraries.GetLibrarySourceCode(libName)
            if foundLibrary:
                for root, _, files in os.walk(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName):
                    for file in files:
                        if file.endswith(".c") or file.endswith(".cpp") or file.endswith(".C"):
                            self.searchCFunction(obj, root, file)
            else:
                self.Pd4Web.exception(f"Error: Could not find {obj.library} in the supported libraries")
        return True

    def buildExternalsObjects(self):
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
            for pdobject in objects:

                if pdobject not in self.Pd4Web.externalsLinkLibraries:
                    self.Pd4Web.externalsLinkLibraries.append(pdobject)
                if libraryPath + "/build" not in self.Pd4Web.externalsLinkLibrariesFolders:
                    self.Pd4Web.externalsLinkLibrariesFolders.append(libraryPath + "/build")
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
        #string = 'set_target_properties(pd4web PROPERTIES LINK_FLAGS "--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd\"")'
        string = 'set_target_properties(pd4web PROPERTIES LINK_FLAGS "--preload-file \\"${CMAKE_CURRENT_SOURCE_DIR}/WebPatch/index.pd@/index.pd\\"")'
        self.cmakeFile.append(string)
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Audios"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file \\"${CMAKE_CURRENT_SOURCE_DIR}/Audios@/Audios/\\"")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/.tmp"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file \\"${CMAKE_CURRENT_SOURCE_DIR}/.tmp@/Libs/\\"")'
            )
        if os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Extras"):
            self.cmakeFile.append("get_target_property(EMCC_LINK_FLAGS pd4web LINK_FLAGS)")
            self.cmakeFile.append(
                'set_target_properties(pd4web PROPERTIES LINK_FLAGS "${EMCC_LINK_FLAGS} --preload-file \\"${CMAKE_CURRENT_SOURCE_DIR}/Extras@/Extras/\\"")'
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

            pd4web_version = importlib_metadata.version("pd4web")
            numbers = pd4web_version.split(".")
            major = numbers[0]
            minor = numbers[1]
            patch = numbers[2]

            f.write(f"// Pd4Web Version\n")
            f.write(f'#define PD4WEB_VERSION_MAJOR "{major}"\n')
            f.write(f'#define PD4WEB_VERSION_MINOR "{minor}"\n')
            f.write(f'#define PD4WEB_VERSION_PATCH "{patch}"\n\n')

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
            f.write(f"#define PD4WEB_PATCH_ZOOM {self.Pd4Web.PATCH_ZOOM}\n")
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
        ninja = self.Pd4Web.Compiler.NINJA
        releaseType = "Release"
        if self.Pd4Web.DEBUG:
            releaseType = "Debug"

        command = [
            emcmake,
            cmake,
            ".",
            "-B",
            "build",
            "-DPDCMAKE_DIR=Pd4Web/Externals/",
            "-DCMAKE_BUILD_TYPE=" + releaseType,
            "-G",
            "Ninja",
            f"-DCMAKE_MAKE_PROGRAM={ninja}",
        ]
        if self.Pd4Web.verbose:
            self.Pd4Web.print(
                " ".join(command), color="green", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL
            )
        result = subprocess.run(
            command, env=self.Pd4Web.env, capture_output=not self.Pd4Web.verbose, text=True
        ).returncode
        if result != 0:
            self.Pd4Web.exception("Error: Could not configure the project")
        os.chdir(cwd)

    def CompileProject(self):
        cwd = os.getcwd()
        os.chdir(self.Pd4Web.PROJECT_ROOT)
        cpu_count = os.cpu_count()
        command = [self.Pd4Web.Compiler.CMAKE, "--build", "build"]
        command.append(f"-j{cpu_count}")
        command.append("--target")
        command.append("pd4web")
        self.Pd4Web.print(
            f"Compiling project... This may take a while\n",
            color="green",
            silence=self.Pd4Web.SILENCE,
            pd4web=self.Pd4Web.PD_EXTERNAL,
        )

        if self.Pd4Web.verbose:
            self.Pd4Web.print(
                " ".join(command), color="green", silence=self.Pd4Web.SILENCE, pd4web=self.Pd4Web.PD_EXTERNAL
            )

        result = subprocess.run(
            command, env=self.Pd4Web.env, capture_output=not self.Pd4Web.verbose, text=True
        ).returncode
        if result != 0:
            self.Pd4Web.exception("Error: Could not compile the project, run on verbose mode to see the error")
        os.chdir(cwd)

    def CopyExtraJsFiles(self):
        cwd = os.getcwd()

        # just copy files if they don't exist

        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/.gitignore"):
            shutil.copy(
                self.Pd4Web.PD4WEB_ROOT + "/../pd4web.gitignore",
                self.Pd4Web.PROJECT_ROOT + "/.gitignore",
            )

        os.chdir(self.Pd4Web.PROJECT_ROOT)

        # Define source and destination paths
        favicon_src = self.Pd4Web.PD4WEB_ROOT + "/../favicon.ico"
        favicon_dest = self.Pd4Web.PROJECT_ROOT + "/favicon.ico"

        threads_src = self.Pd4Web.PD4WEB_ROOT + "/../pd4web.threads.js"
        threads_dest = self.Pd4Web.PROJECT_ROOT + "/WebPatch/pd4web.threads.js"

        midi_src = self.Pd4Web.PD4WEB_ROOT + "/../pd4web.midi.js"
        midi_dest = self.Pd4Web.PROJECT_ROOT + "/WebPatch/pd4web.midi.js"

        css_src = self.Pd4Web.PD4WEB_ROOT + "/../pd4web.style.css"
        css_dest = self.Pd4Web.PROJECT_ROOT + "/WebPatch/pd4web.style.css"

        js_src = self.Pd4Web.PD4WEB_ROOT + "/../pd4web.gui.js"
        js_dest = self.Pd4Web.PROJECT_ROOT + "/WebPatch/pd4web.gui.js"

        # Copy favicon if it does not exist at the destination
        if not os.path.exists(favicon_dest):
            shutil.copy(favicon_src, favicon_dest)

        # Copy threads.js if it does not exist at the destination
        if not os.path.exists(threads_dest):
            shutil.copy(threads_src, threads_dest)

        # Copy midi.js if it does not exist at the destination
        if not os.path.exists(midi_dest):
            shutil.copy(midi_src, midi_dest)

        # Copy CSS file if it does not exist at the destination
        if not os.path.exists(css_dest):
            shutil.copy(css_src, css_dest)

        # Copy JS file if it does not exist at the destination
        if not os.path.exists(js_dest):
            shutil.copy(js_src, js_dest)

        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.html") and self.Pd4Web.TEMPLATE == 0:
            index_page = "index.html"
            if not self.Pd4Web.GUI:
                index_page = "nogui.html"
            shutil.copy(
                self.Pd4Web.PD4WEB_ROOT + f"/../{index_page}",
                self.Pd4Web.PROJECT_ROOT + "/WebPatch/index.html",
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

        if self.Pd4Web.TEMPLATE != 0:
            root = os.path.join(self.Pd4Web.PD4WEB_ROOT, "..", f"Templates/{self.Pd4Web.TEMPLATE}")
            if not os.path.exists(root):
                self.Pd4Web.exception(f"Error: Could not find template {self.Pd4Web.TEMPLATE}")
            files = os.listdir(root)
            for file in files:
                if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/WebPatch/" + file):
                    shutil.copy(os.path.join(root, file), self.Pd4Web.PROJECT_ROOT + "/WebPatch/")
        os.chdir(cwd)
        
    def SaveProjectVersions(self):
        # check if file exists
        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/versions.yaml"):
            with open(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/versions.yaml", "w") as f:
                yaml.dump(self.Pd4Web.Version, f)
                return       

    def __repr__(self) -> str:
        return f"< GetCode Object >"

    def __str__(self) -> str:
        return self.__repr__()
