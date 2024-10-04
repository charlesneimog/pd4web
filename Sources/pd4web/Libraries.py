import os
import zipfile

from pygit2.repository import Repository
import requests
import yaml
import pygit2
import shutil

from .Pd4Web import Pd4Web
from .Helpers import DownloadZipFile, pd4web_print

# ╭──────────────────────────────────────╮
# │    In this file we have all code     │
# │ related to get the Libraries Source  │
# │          Code, Download it.          │
# ╰──────────────────────────────────────╯


class ExternalLibraries:
    def __init__(self, Pd4Web: Pd4Web) -> None:
        self.Pd4Web = Pd4Web
        self.PROJECT_ROOT = Pd4Web.PROJECT_ROOT
        self.GetSupportedLibraries()
        return

    def InitVariables(self):
        self.UsedObjects = []
        self.UsedLibraries = []
        self.totalOfLibraries = 0
        self.LibraryNames = []
        self.DownloadSources = {}
        self.SupportedLibraries = {}
        self.LibraryScriptDir = ""

    def GetSupportedLibraries(self):
        """
        It reads yaml file and get all supported libraries.
        """
        libFolder = self.Pd4Web.PD4WEB_LIBRARIES
        externalFile = os.path.join(libFolder, "Libraries.yaml")
        # check if file exists
        if not os.path.exists(externalFile):
            pass

        self.DynamicLibraries = []
        with open(externalFile) as file:
            supportedLibraries = yaml.load(file, Loader=yaml.FullLoader)
            self.DownloadSources = supportedLibraries["Sources"]
            self.SupportedLibraries = supportedLibraries["Libraries"]
            self.LibraryNames = [lib["Name"] for lib in self.SupportedLibraries]
            self.totalOfLibraries = len(supportedLibraries)

    class LibraryClass:
        def __init__(self, LibraryData, DownloadSources) -> None:
            try:
                self.name = LibraryData["Name"]
            except:
                self.valid = False
                return

            self.dev = LibraryData["Developer"]
            self.repo = LibraryData["Repository"]
            self.folder = ""
            self.downloadSources = DownloadSources
            self.downloadLink = ""

            if "Source" in LibraryData:
                self.source = LibraryData["Source"]
            else:
                self.source = False
                if "DirectLink" in LibraryData:
                    self.directLink = LibraryData["DirectLink"]
                else:
                    raise Exception("Error: {self.name} doesn't have a download source")

            self.GetLinkForDownload()

            if "Dependencies" in LibraryData:
                self.dynamicLibraries = LibraryData["Dependencies"]
            else:
                self.dynamicLibraries = []

            if "Unsupported" in LibraryData:
                self.unsupported = LibraryData["Unsupported"]
            else:
                self.unsupported = []

            if "Version" in LibraryData:
                self.version = LibraryData["Version"]
            else:
                self.version = None

            self.usedObjs = []
            self.usedSourceFiles = []
            self.extraFuncExecuted = False
            self.unsupportedObjects = {}
            self.extraFlags = []
            self.valid = True

        def __str__(self):
            return f"< Library: {self.name} >"

        def __repr__(self):
            return self.__str__()

        def GetLinkForDownload(self):
            if self.source:
                if self.source in self.downloadSources:
                    self.downloadLink = self.downloadSources[self.source]
                    return self.downloadLink.format(self.dev, self.repo)
                else:
                    raise Exception(f"Error: {self.name} doesn't have a download source")
            else:
                return self.directLink

    def GetLibraryData(self, libName) -> LibraryClass:
        lib = next((lib for lib in self.SupportedLibraries if lib["Name"] == libName), None)
        lib = self.LibraryClass(lib, self.DownloadSources)
        return lib

    def isSupportedLibrary(self, name):
        if name in self.LibraryNames:
            return True
        else:
            return False

    def CheckLibraryLink(self, url: str):
        try:
            response = requests.head(url, allow_redirects=True)
            if response.status_code == 200:
                return True
            else:
                return False
        except:
            return False

    def getLibCommitVersion(self, libRepo, tag_name) -> pygit2.Commit:
        """ """
        try:
            tag_ref = libRepo.references.get(f"refs/tags/{tag_name}")
            if isinstance(tag_ref.peel(), pygit2.Tag):
                commit = tag_ref.peel().target
            else:
                commit = tag_ref.peel()
        except:
            pd4web_print(f"Tag not found, trying to get commit by hash", color="yellow")
            commit = libRepo.head.peel()
        return commit

    def CloneLibrary(self, libsPath, libData):
        """
        Initializes submodules in the specified repository path.

        Args:
            path (str): The path to the repository containing submodules.
        """
        libPath = self.Pd4Web.APPDATA + "/Externals/" + libData.name
        libRepo: pygit2.Repository = pygit2.Repository(libPath)
        if os.path.exists(libPath):
            curr_commit: pygit2.Commit = libRepo.head.peel()
            lib_commit: pygit2.Commit = self.getLibCommitVersion(libRepo, libData.version)
            if curr_commit.id == lib_commit.id:
                return  # library up-to-date
            libRepo.set_head(lib_commit.id)
            libRepo.checkout_tree(lib_commit)
            libRepo.reset(lib_commit.id, pygit2.GIT_RESET_HARD)
            return

        libLink = f"https://github.com/{libData.dev}/{libData.repo}"
        try:
            pd4web_print(f"Cloning library {libData.repo}... This will take some time!", color="green")
            pygit2.clone_repository(libLink, libPath)
        except Exception as e:
            raise Exception(f"Failed to clone repository: {str(e)}")

        tag_name = libData.version  # Assuming libData.version contains either a tag or a commit hash
        commit = self.getLibCommitVersion(libRepo, tag_name)
        libRepo.set_head(commit.id)
        libRepo.checkout_tree(commit)
        libRepo.reset(commit.id, pygit2.GIT_RESET_HARD)
        pd4web_print(f"Library {libData.repo} cloned successfully!", color="green")
        pd4web_print(f"Using commit {commit.id}", color="green")
        try:
            pd4web_print(f"Initializing submodules of {libData.repo}...", color="green")
            submodule_collection = pygit2.submodules.SubmoduleCollection(pygit2.Repository(libPath))
            submodule_collection.init()
            submodule_collection.update()
            libName = libData.name
            libFolder = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName
            self.Pd4Web.Objects.GetLibraryObjects(libFolder, libName)
        except:
            raise Exception("Failed to initialize submodules.")

        # TODO: Try to merge commits from submodules
        try:
            main_repo = pygit2.Repository(self.Pd4Web.PROJECT_ROOT)
            submodule_repo = pygit2.Repository(libPath)

        except Exception as e:
            raise Exception(f"Failed to merge submodule commits: {str(e)}")

    def GetLibrarySourceCode(
        self,
        libName: str,
    ):
        if not self.isSupportedLibrary(libName):
            return False

        if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
            os.makedirs(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")

        # Library Download
        libData = self.GetLibraryData(libName)
        if not os.path.exists(self.Pd4Web.APPDATA + "/Externals"):
            os.mkdir(self.Pd4Web.APPDATA + "/Externals")

        libFolder = self.Pd4Web.APPDATA + f"/Externals/{libData.name}"
        self.CloneLibrary(libFolder, libData)
        if not os.path.exists(self.PROJECT_ROOT + f"/Pd4Web/Externals/{libData.name}"):
            shutil.copytree(libFolder, self.PROJECT_ROOT + "/Pd4Web/Externals/" + libData.name, symlinks=False)
        return True

    def GetLibrarySourceCodeOld(
        self,
        libName: str,
    ):
        if self.isSupportedLibrary(libName):
            if os.path.exists(os.path.join(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName)):
                return True
            if not os.path.exists(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals"):
                os.makedirs(self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals")

            # Library Download
            libData = self.GetLibraryData(libName)
            if not os.path.exists(self.Pd4Web.APPDATA + "/Externals"):
                os.mkdir(self.Pd4Web.APPDATA + "/Externals")
            libPath = self.Pd4Web.APPDATA + f"/Externals/{libData.name}/"
            if not os.path.exists(libPath):
                os.mkdir(libPath)

            libZip = f"{self.Pd4Web.APPDATA}/Externals/{libData.name}/{libData.version}.zip"
            if not os.path.exists(libZip):
                pd4web_print(
                    f"Downloading {libData.name} {libData.version}...\n",
                    color="green",
                    silence=self.Pd4Web.SILENCE,
                )
                libSource = f"https://github.com/{libData.dev}/{libData.repo}/archive/refs/tags/{libData.version}.zip"
                if not self.CheckLibraryLink(libSource):
                    libSource = f"https://github.com/{libData.dev}/{libData.repo}/archive/{libData.version}.zip"
                    if not self.CheckLibraryLink(libSource):
                        raise Exception(
                            f"Error: Could not find good download link for {libData.name} version {libData.version}."
                        )
                DownloadZipFile(libSource, libZip)  # <== Download Library

            if not os.path.exists(self.PROJECT_ROOT + f"/Pd4Web/Externals/{libData.name}"):
                with zipfile.ZipFile(libZip, "r") as zip_ref:
                    zip_ref.extractall(self.PROJECT_ROOT + "/Pd4Web/Externals/")
                    extractFolderName = zip_ref.namelist()[0]
                    os.rename(
                        self.PROJECT_ROOT + "/Pd4Web/Externals/" + extractFolderName,
                        self.PROJECT_ROOT + "/Pd4Web/Externals/" + libData.name,
                    )

            libFolder = self.Pd4Web.PROJECT_ROOT + "/Pd4Web/Externals/" + libName
            self.Pd4Web.Objects.GetLibraryObjects(libFolder, libName)
            return True

    def AddUsedLibraries(self, LibraryName):
        self.UsedLibraries.append(LibraryName)

    def addToUsed(self, objName):
        self.UsedObjects.append(objName)

    def getUsedObjs(self):
        return self.UsedObjects

    def __repr__(self) -> str:
        return f"< Externals Libraries: {self.totalOfLibraries} >"

    def __str__(self) -> str:
        return self.__repr__()
