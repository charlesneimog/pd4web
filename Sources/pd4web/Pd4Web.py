import argparse
import os
import sys
import subprocess
import pygit2
from pygit2 import Repository, GIT_RESET_HARD
import requests
import shutil
import importlib.metadata as importlib_metadata
import certifi
import platform
import yaml

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
    PD_VERSION: str = "0.55-2"
    EMSDK_VERSION: str = "4.0.3"
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

    def arg_parse(self):
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

        self.get_paths()
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

        self.execute()

    def init_vars(self):
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
        if sys.platform == "darwin":
            self.env["PATH"] += ":/bin:/usr/bin"

    def execute(self):
        from .Builder import GetAndBuildExternals
        from .Compilers import ExternalsCompiler
        from .Patch import Patch

        if self.Patch == "":
            self.exception("You must set a patch file")

        self.get_paths()
        self.init_vars()

        if os.path.exists(os.path.join(self.PROJECT_ROOT, "Pd4Web/versions.yaml")):
            yaml_file = yaml.safe_load(open(os.path.join(self.PROJECT_ROOT, "Pd4Web/versions.yaml"), "r"))
            self.EMSDK_VERSION = yaml_file["emsdk"]
            self.PD_VERSION = yaml_file["pure-data"]
            print(self.EMSDK_VERSION, self.PD_VERSION)

        # ╭──────────────────────────────────────╮
        # │    NOTE: Sobre a recursivade para    │
        # │      patch, talvez não chamar o      │
        # │        construtor de Pd4Web,         │
        # │mas some a mesma ordem para __init__. │
        # ╰──────────────────────────────────────╯

        # ───────────── Init Classes ─────────────
        self.get_emsdk_sourcecode()
        self.get_pd_sourcecode()
        self.Compiler = ExternalsCompiler(self)

        # ──────────── Process Patch ──────────
        self.ProcessedPatch: Patch = Patch(self)  # Recursively in case of Abstraction

        # ──────────── Build Externals ──────────
        self.ExternalsBuilder = GetAndBuildExternals(self)

    def run_browser(self):
        self.CWD = os.getcwd()
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        self.get_publicPath()

        emccRun = self.APPDATA + "/emsdk/upstream/emscripten/emrun"
        if not os.path.exists(emccRun):
            self.exception("emrun not found. Please install Emscripten")
        projectRoot = os.path.realpath(self.Patch)
        os.chdir(projectRoot)
        subprocess.run(f"{emccRun} {projectRoot}/index.html", shell=True, env=self.env)

    def get_paths(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(self.Patch))
        self.PROJECT_PATCH = os.path.basename(self.Patch)
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))
        if self.PD4WEB_LIBRARIES == "":
            self.PD4WEB_LIBRARIES = os.path.join(self.PD4WEB_ROOT, "..", "Libraries")
            self.PD4WEB_LIBRARIES = os.path.abspath(self.PD4WEB_LIBRARIES)

        self.CWD = os.getcwd()
        if not os.path.exists(os.path.join(self.PD4WEB_ROOT, "data")):
            os.makedirs(os.path.join(self.PD4WEB_ROOT, "data"))
        self.get_publicPath()
        if not os.path.exists(self.APPDATA):
            os.makedirs(self.APPDATA)

    # pd_path = self.APPDATA + "/Pd"

    def get_sourcecode(self, url, path, tag_name):
        if not os.path.exists(path):
            self.print(f"Cloning {path}", color="yellow", silence=self.SILENCE, pd4web=self.PD_EXTERNAL)
            ok = pygit2.clone_repository(url, path)
            if not ok:
                self.exception(f"Failed to clone {path}")
            libRepo: Repository = Repository(path)
            tag_ref = libRepo.references.get(f"refs/tags/{tag_name}")
            if tag_ref is None:
                self.exception("Tag ref is None")
            if isinstance(tag_ref.peel(), pygit2.Tag):
                commit = tag_ref.peel().target
            else:
                commit = tag_ref.peel()
            libRepo.set_head(commit.id)
            libRepo.checkout_tree(commit)
            libRepo.reset(commit.id, GIT_RESET_HARD)

        # check if actual tag match the desired tag
        libRepo: Repository = Repository(path)
        libRepo.remotes["origin"].fetch()
        tag_ref = libRepo.references.get(f"refs/tags/{tag_name}")
        target_commit = tag_ref.peel()
        libRepo.reset(target_commit.id, pygit2.GIT_RESET_HARD)
        libRepo.checkout_tree(target_commit.tree, strategy=pygit2.GIT_CHECKOUT_FORCE)
        libRepo.set_head(target_commit.id)

    def get_emsdk_sourcecode(self) -> None:
        self.EMSDK = self.APPDATA + "/emsdk/emsdk"
        path = self.APPDATA + "/emsdk"
        emsdk = "https://github.com/emscripten-core/emsdk"

        if not os.path.exists(path):
            self.get_sourcecode(emsdk, path, Pd4Web.EMSDK_VERSION)

        repo = Repository(path)
        previous_tag = repo.head.peel().name
        print(previous_tag)

        if platform.system() == "Windows":
            self.EMSDK += ".bat"
        else:
            result = subprocess.run(
                ["chmod", "+x", self.EMSDK], env=self.env, capture_output=not self.verbose, text=True
            )
            if result.returncode != 0:
                self.exception("Failed to make emsdk executable")

        command = f"{self.EMSDK} install {self.EMSDK_VERSION}"
        result = subprocess.run(command, shell=True, env=self.env).returncode
        if result != 0:
            self.exception("Failed to activate emsdk")
        command = f"{self.EMSDK} activate {self.EMSDK_VERSION}"
        result = subprocess.run(command, shell=True, env=self.env).returncode
        if result != 0:
            self.exception("Failed to activate emsdk")

    def get_pd_sourcecode(self) -> None:
        pd = "https://github.com/pure-data/pure-data"
        self.get_sourcecode(pd, self.APPDATA + "/pure-data", Pd4Web.PD_VERSION)
        if not os.path.exists(self.PROJECT_ROOT + "/Pd4Web/pure-data"):
            shutil.copytree(self.APPDATA + "/pure-data/src", self.PROJECT_ROOT + "/Pd4Web/pure-data/src")
            shutil.copy(self.APPDATA + "/pure-data/README.txt", self.PROJECT_ROOT + "/Pd4Web/pure-data/README.md")
            shutil.copy(self.APPDATA + "/pure-data/LICENSE.txt", self.PROJECT_ROOT + "/Pd4Web/pure-data/LICENSE.txt")

    def silence(self):
        self.SILENCE = True

    def do_actions(self, parser):
        if parser.run_browser:
            self.run_browser()
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

            package = "pd4web"
            response = requests.get(f"https://pypi.org/pypi/{package}/json")
            data = response.json()
            latest_version = data["info"]["version"]
            pd4web_version = importlib_metadata.version("pd4web")
            # check if the latest version is different from the installed version
            this_major, this_minor, this_bug = (
                pd4web_version.split(".")[0],
                pd4web_version.split(".")[1],
                pd4web_version.split(".")[2],
            )
            pip_major, pip_minor, pip_bug = (
                latest_version.split(".")[0],
                latest_version.split(".")[1],
                latest_version.split(".")[2],
            )
            if this_major < pip_major or this_minor < pip_minor or this_bug < pip_bug:
                self.print(
                    f"pd4web version {pd4web_version} is outdated. The latest version is {latest_version}, please update",
                    color="yellow",
                )
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

    def get_publicPath(self):
        if sys.platform == "win32":
            public_path = os.getenv("PROGRAMDATA")
            public_path = os.path.join(public_path, "pd4web")
            if not os.path.exists(public_path):
                os.makedirs(public_path)
            self.APPDATA = public_path
        elif sys.platform == "darwin":
            self.APPDATA = os.path.join(os.path.expanduser("~/Library/"), "pd4web")
        elif sys.platform == "linux":
            self.APPDATA = os.path.join(os.path.expanduser("~/.local/share"), "pd4web")
        else:
            self.exception("Unsupported platform")

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
            # sys.tracebacklimit = 0
            raise Exception(text)
