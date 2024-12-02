import argparse
import os
import sys
import subprocess
import pygit2

import shutil
import importlib.metadata as importlib_metadata

import certifi

os.environ["SSL_CERT_FILE"] = certifi.where()


class Pd4Web:
    # Paths
    PD4WEB_LIBRARIES: str = ""
    PD_EXTERNAL: bool = False

    # User
    TEMPLATE: int = 0

    # Dev
    BYPASS_UNSUPPORTED: bool = False
    SILENCE: bool = False
    PD_VERSION: str = "0.55-0"
    EMSDK_VERSION: str = "3.1.68"
    DEBUG: bool = False

    # Compiler
    MEMORY_SIZE: int = 256

    # Audio
    OUTCHS_COUNT: int = 0
    INCHS_COUNT: int = 0

    # Gui
    FPS: int = 60
    GUI: bool = True
    AUTO_THEME: bool = True
    PATCH_ZOOM: int = 1

    # Midi
    MIDI: bool = False

    def __init__(self, Patch=""):
        self.Patch = Patch
        self.verbose = False

    def argParse(self):
        print()
        parser = argparse.ArgumentParser(
            description="Compile Pure Data externals for the web.",
            usage="pd4web.py <PureData Patch>",
        )
        parser.add_argument("patch_file", type=str, help="The patch file to be compiled", nargs="?")
        self.action_flags(parser)
        self.options_flags(parser)
        self.dev_flags(parser)

        parser = parser.parse_args()

        self.getMainPaths()
        self.do_actions(parser)

        self.Parser = parser
        self.Patch = os.path.abspath(self.Parser.patch_file)

        if not os.path.isfile(self.Patch):
            self.exception("\n\nError: Patch file not found")

        # default values
        self.verbose = self.Parser.verbose
        self.MEMORY_SIZE = self.Parser.initial_memory
        self.PATCH_ZOOM = self.Parser.patch_zoom
        self.GUI = not self.Parser.nogui
        self.PD_VERSION = self.Parser.pd_version
        self.PD_EXTERNAL = self.Parser.pd_external
        self.BYPASS_UNSUPPORTED = self.Parser.bypass_unsupported
        self.TEMPLATE = self.Parser.template
        self.DEBUG = self.Parser.debug

        self.Execute()

    def InitVariables(self):
        from .Objects import Objects
        from .Libraries import ExternalLibraries

        self.cpuCores = os.cpu_count()
        self.usedObjects = []
        self.patchLinesProcessed = []
        self.uiReceiversSymbol = []
        self.externalsSourceCode = []
        self.externalsLinkLibraries = []
        self.externalsLinkLibrariesFolders = []
        self.externalsSetupFunctions = []

        self.declaredLocalAbs = []
        self.declaredLibsObjs = []
        self.declaredPaths = []
        self.processedAbs = []

        # Versions
        self.Version = {}
        self.Version["python"] = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
        self.Version["pure-data"] = self.PD_VERSION
        self.Version["emsdk"] = self.EMSDK_VERSION
        self.Version["pd4web"] = importlib_metadata.version("pd4web")
        self.Version["externals"] = {}

        self.Libraries: ExternalLibraries = ExternalLibraries(self)
        self.Objects: Objects = Objects(self)

        self.env = os.environ.copy()
        python_dir = os.path.dirname(sys.executable)
        if "PATH" in self.env:
            self.env["PATH"] = f"{python_dir};{self.env['PATH']}"
        else:
            self.env["PATH"] = python_dir
        self.env["PYTHON"] = sys.executable
        self.env["EMSDK_QUIET"] = "1"
        # add /bin and /usr/bin to PATH on macOS
        if sys.platform == "darwin":
            self.env["PATH"] += ":/bin:/usr/bin"

    def Execute(self):
        from .Builder import GetAndBuildExternals
        from .Compilers import ExternalsCompiler
        from .Patch import Patch

        if self.Patch == "":
            self.exception("You must set a patch file")

        self.getMainPaths()
        self.InitVariables()

        # ╭──────────────────────────────────────╮
        # │    NOTE: Sobre a recursivade para    │
        # │      patch, talvez não chamar o      │
        # │        construtor de Pd4Web,         │
        # │mas some a mesma ordem para __init__. │
        # ╰──────────────────────────────────────╯

        # ───────────── Init Classes ─────────────
        self.GetPdSourceCode()
        self.Compiler = ExternalsCompiler(self)

        # ──────────── Process Patch ──────────
        self.ProcessedPatch: Patch = Patch(self)  # Recursively in case of Abstraction

        # ──────────── Build Externals ──────────
        self.ExternalsBuilder = GetAndBuildExternals(self)

        # try:
        #     self.PROJECT_GIT = pygit2.Repository(self.PROJECT_ROOT)
        # except pygit2.GitError:
        #     self.PROJECT_GIT = pygit2.init_repository(self.PROJECT_ROOT, bare=False)

    def RunBrowser(self):
        self.CWD = os.getcwd()
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        
        if sys.platform == "win32":
            program_data_path = os.getenv("PROGRAMDATA")
            app_data_subfolder = os.path.join(program_data_path, "pd4web")
            if not os.path.exists(app_data_subfolder):
                os.makedirs(app_data_subfolder)
            self.APPDATA = app_data_subfolder
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(os.path.expanduser("~/.local/share"), "pd4web")
        else:
            self.exception("Unsupported platform")

        emccRun = self.APPDATA + "/emsdk/upstream/emscripten/emrun"
        if not os.path.exists(emccRun):
            self.exception("emrun not found. Please install Emscripten")
        projectRoot = os.path.realpath(self.Patch)
        os.chdir(projectRoot)
        subprocess.run(f"{emccRun} {projectRoot}/index.html", shell=True, env=self.env)

    def getMainPaths(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        if self.PD4WEB_LIBRARIES == "":
            self.PD4WEB_LIBRARIES = os.path.join(self.PD4WEB_ROOT, "..", "Libraries")
            self.PD4WEB_LIBRARIES = os.path.abspath(self.PD4WEB_LIBRARIES)

        self.CWD = os.getcwd()
        if not os.path.exists(os.path.join(self.PD4WEB_ROOT, "data")):
            os.makedirs(os.path.join(self.PD4WEB_ROOT, "data"))
        if sys.platform == "win32":
            program_data_path = os.getenv("PROGRAMDATA")
            app_data_subfolder = os.path.join(program_data_path, "pd4web")
            if not os.path.exists(app_data_subfolder):
                os.makedirs(app_data_subfolder)
            self.APPDATA = app_data_subfolder
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(os.path.expanduser("~/.local/share"), "pd4web")
        else:
            self.exception("Unsupported platform")
        if not os.path.exists(self.APPDATA):
            os.makedirs(self.APPDATA)

    def GetPdSourceCode(self):
        if not os.path.exists(self.APPDATA + "/Pd"):
            self.print("Cloning Pd", color="yellow", silence=self.SILENCE, pd4web=self.PD_EXTERNAL)
            pd_path = self.APPDATA + "/Pd"
            pd_git = "https://github.com/pure-data/pure-data"
            ok = pygit2.clone_repository(pd_git, pd_path)
            if not ok:
                self.exception("Failed to clone emsdk")

            libRepo: pygit2.Repository = pygit2.Repository(pd_path)
            tag_name = Pd4Web.PD_VERSION

            # commit
            tag_ref = libRepo.references.get(f"refs/tags/{tag_name}")
            if tag_ref is None:
                self.exception("Tag ref is None")
            if isinstance(tag_ref.peel(), pygit2.Tag):
                commit = tag_ref.peel().target
            else:
                commit = tag_ref.peel()
            libRepo.set_head(commit.id)
            libRepo.checkout_tree(commit)
            libRepo.reset(commit.id, pygit2.GIT_RESET_HARD)

        if not os.path.exists(self.PROJECT_ROOT + "/Pd4Web/pure-data"):
            shutil.copytree(self.APPDATA + "/Pd/src", self.PROJECT_ROOT + "/Pd4Web/pure-data/src")
            # copy README and LICENSE
            shutil.copy(self.APPDATA + "/Pd/README.txt", self.PROJECT_ROOT + "/Pd4Web/pure-data/README.txt")
            shutil.copy(self.APPDATA + "/Pd/LICENSE.txt", self.PROJECT_ROOT + "/Pd4Web/pure-data/LICENSE.txt")

    def Silence(self):
        self.SILENCE = True

    def do_actions(self, parser):
        if parser.run_browser:
            self.RunBrowser()
            exit()
        if parser.clear:
            shutil.rmtree(os.path.join(self.PROJECT_ROOT, "build"), ignore_errors=True)
            shutil.rmtree(os.path.join(self.PROJECT_ROOT, "Pd4Web"), ignore_errors=True)
            # shutil.rmtree(os.path.join(self.PROJECT_ROOT, "WebPatch"), ignore_errors=True)

        if parser.add_lib_cmake:
            newLibCmake = parser.add_lib_cmake
            if not os.path.exists(newLibCmake):
                self.exception("Library not found")
            if not newLibCmake.endswith(".cmake"):
                self.exception("Library must end with .cmake")

            libName = os.path.basename(newLibCmake)
            self.print(f"{libName} added to pd4web supported libraries", color="green")
            shutil.copy(newLibCmake, self.PD4WEB_LIBRARIES)
            exit()

        if parser.version:
            pd4web_version = importlib_metadata.version("pd4web")
            print(f"pd4web version {pd4web_version}")
            exit()

    def action_flags(self, parser):
        parser.add_argument(
            "--run-browser",
            required=False,
            action="store_true",
            default=False,
            help="Use Emscripten to run the Browser",
        )

        parser.add_argument(
            "--add-lib-cmake",
            required=False,
            type=str,
            help="Add new library to pd4web supported libraries",
        )
        parser.add_argument(
            "--version",
            required=False,
            action="store_true",
            help="Get the version of pd4web",
        )
        parser.add_argument(
            "--clear",
            required=False,
            action="store_true",
            default=False,
            help="Clear the cache before running",
        )

        parser.add_argument(
            "--install-emcc",
            required=False,
            action="store_true",
            default=False,
            help="Install emcc",
        )

    def options_flags(self, parser):
        parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode")
        parser.add_argument(
            "-m",
            "--initial-memory",
            required=False,
            default=128,
            type=int,
            help="Initial memory size in MB",
        )

        parser.add_argument(
            "--patch-file",
            required=False,
            default="",
            type=str,
            help="Patch file to compile",
        )

        # GUI
        parser.add_argument(
            "-nogui",
            "--nogui",
            required=False,
            action="store_true",
            default=False,
            help="Do not use GUI interface",
        )
        parser.add_argument(
            "-z",
            "--patch-zoom",
            required=False,
            type=float,
            default=1,
            help="Zoom level for the patch (must be a number)",
        )

        # Debug
        parser.add_argument(
            "--debug",
            required=False,
            action="store_true",
            default=False,
            help="Enable debug mode for the build",
        )

        # Pure Data
        parser.add_argument(
            "--pd-version",
            required=False,
            default="0.55-0",
            type=str,
            help="Pure Data version to use",
        )

        # Pd Object
        parser.add_argument(
            "--pd-external",
            required=False,
            default=False,
            action="store_true",
            help="If is it pd4web external",
        )

        parser.add_argument(
            "--pd-external-version",
            required=False,
            default=importlib_metadata.version("pd4web"),
            type=str,
            help="Version of the pd4web external being used",
        )

        ## User
        parser.add_argument(
            "--template",
            required=False,
            default=0,
            type=int,
            help="Number of the template to use, check https://charlesneimog.github.io/pd4web/en/patch/templates",
        )

    # Parse
    def dev_flags(self, parser):
        parser.add_argument(
            "--bypass-unsupported",
            required=False,
            default=False,
            action="store_true",
            help="Bypass unsupported objects in libraries",
        )

    def print(self, text, color=None, bright=False, silence=False, pd4web=False):
        tab = " " * 4
        if pd4web:
            if color == "red":
                print("ERROR: " + text)
                sys.stdout.flush()
            elif color == "yellow":
                print("WARNING: " + text)
                sys.stdout.flush()
            else:
                print(text)
                sys.stdout.flush()
            return
        if silence:
            return
        try:
            if color is None:
                color_code = ""
            else:
                color_code = {
                    "red": "\033[91;1m" if bright else "\033[91m",
                    "green": "\033[92;1m" if bright else "\033[92m",
                    "yellow": "\033[93;1m" if bright else "\033[93m",
                    "blue": "\033[94;1m" if bright else "\033[94m",
                    "magenta": "\033[95;1m" if bright else "\033[95m",
                    "cyan": "\033[96;1m" if bright else "\033[96m",
                    "lightgray": "\033[97;1m" if bright else "\033[97m",
                    "darkgray": "\033[90;1m" if bright else "\033[90m",
                    "lightred": "\033[91;1m" if bright else "\033[91m",
                    "lightgreen": "\033[92;1m" if bright else "\033[92m",
                    "lightyellow": "\033[93;1m" if bright else "\033[93m",
                    "lightblue": "\033[94;1m" if bright else "\033[94m",
                    "lightmagenta": "\033[95;1m" if bright else "\033[95m",
                    "lightcyan": "\033[96;1m" if bright else "\033[96m",
                    "white": "\033[97;1m" if bright else "\033[97m",
                    "blackbold": "\033[1m",
                    "blackunderline": "\033[4m",
                    "dark_grey": "\033[90m",
                }.get(color.lower(), "")
            reset_code = "\033[0m"

            if color == "red":
                print(tab + color_code + "🔴️ ERROR: " + text + reset_code)
            elif color == "yellow":
                print(tab + color_code + "🟡️ WARNING: " + text + reset_code)
            elif color == "green":
                print(tab + color_code + "🟢️ " + text + reset_code)
            elif color == "blue":
                print(tab + color_code + "🔵️ " + text + reset_code)
            elif color == "magenta":
                print(tab + color_code + "🟣️ " + text + reset_code)
            else:
                print(tab + color_code + text + reset_code)
            sys.stdout.flush()  # Ensure immediate output
        except:
            print(text)
            sys.stdout.flush()  # Ensure immediate output

    def exception(self, text):
        if self.PD_EXTERNAL:
            self.print(text, color="red", silence=self.SILENCE, pd4web=self.PD_EXTERNAL)
            exit(-1)
        else:
            raise Exception(text)
