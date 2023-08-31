from lib.cyclone import *
from lib.earplug import *


GITHUB = "https://api.github.com/repos/{}/{}/releases"
GITHUB_TAGS = "https://api.github.com/repos/{}/{}/tags"

class PureDataExternals:
    def __init__(self, repoAPI, user, repo, name, extraFunc=None, single=False) -> None:
        # when object is a single object, like earplug~, py4pd, and others, we need to use single=True
        self.repoAPI = repoAPI
        self.username = user
        self.repo = repo
        self.name = name
        self.folder = ''
        self.singleObject = single
        self.extraFunc = extraFunc
        self.extraFuncExecuted = False
        self.ROOT = os.getcwd()
        
    def __repr__(self) -> str:
        return f"<Dev: {self.username} | User: {self.repo}>"

    def __str__(self) -> str:
        return f"<Dev: {self.username} | User: {self.repo}>"


class PD_EXTERNALS:
    def __init__(self) -> None:
        self.PureDataExternals = []
        self.LibraryNames = []
        self.UsedLibraries = []
        self.UsedLibrariesNames = []


    def add(self, PureDataExternals):
        self.PureDataExternals.append(PureDataExternals)
        self.LibraryNames.append(PureDataExternals.name)

    def get(self, name):
        for i in self.PureDataExternals:
            if i.name == name:
                return i
        return None

    def isUsed(self, name):
        for i in self.UsedLibraries:
            if i.name == name:
                return i
        return False

    def getDownloadURL(self, name):
        for i in self.PureDataExternals:
            if i.name == name:
                return i.repoAPI.format(i.username, i.repo)
        return None

    def executeExtraFunction(self):
        for i in self.PureDataExternals:
            if i.extraFunc != None and i in self.UsedLibraries:
                libraryClass = self.isUsed(i.name)
                i.extraFunc(libraryClass)


    def __repr__(self) -> str:
        return f"<PD_EXTERNALS>"

    def __str__(self) -> str:
        return f"<PD_EXTERNALS>"



PD_LIBRARIES = PD_EXTERNALS()
PD_LIBRARIES.add(PureDataExternals(GITHUB, "porres", "pd-cyclone", "cyclone", cyclone_extra))
PD_LIBRARIES.add(PureDataExternals(GITHUB, "porres", "pd-else", "else"))
PD_LIBRARIES.add(PureDataExternals(GITHUB_TAGS, "pd-externals", "earplug", "earplug~", earplug_extra, single=True))



