import os
import sys

import requests

from .Helpers import getPrintValue, pd4web_print
from .Libraries import ExternalLibraries
from .Patch import Patch, PatchLine


class GetCode():
    def __init__(self, ProcessedPatch: Patch, Libraries: ExternalLibraries):
        self.PROJECT_ROOT = ProcessedPatch.PROJECT_ROOT
        self.InitVariables()

        self.Libraries = Libraries
        for i in ProcessedPatch.PatchLinesProcessed:
            if i.isExternal:
                self.DownloadLibrarySourceCode(i)
            else:
                pass

    def InitVariables(self):
        self.LibraryScriptDir = os.path.dirname(os.path.realpath(__file__))

    def DownloadLink(self, PatchLine: PatchLine):
        LibraryClass = PatchLine.GetLibraryData()
        if LibraryClass.repoAPI == False:
            return False

        print("Downloading " + PatchLine.Library)


    def DownloadLibrarySourceCode(self, PatchLine: PatchLine):
        ResponseJson = {"message": "Unknown error"}
        LibraryName = PatchLine.Library
        if self.Libraries.isSupportedLibrary(LibraryName):
            if os.path.exists(os.path.join(self.LibraryScriptDir + "/Libraries/" + LibraryName)):
                return True

            GithutAPI = self.DownloadLink(PatchLine)
            return;

                # pd4web_print("Downloading " + LibraryName, color="yellow")
                #
                # if GithutAPI is None:
                #     raise Exception("LibURL is not a string or None")
                # elif GithutAPI == False:  # means that is a direct link
                #     response = requests.get(LibraryClass.directLink)
                # elif isinstance(GithutAPI, str):  # is a GithubAPI link
                #     response = requests.get(GithutAPI)
                #     responseJson = response.json()
                #     sourceCodeLink = responseJson[0]["zipball_url"]
                #     response = requests.get(sourceCodeLink)
                # else:
                #     raise Exception("The link of the srcs of " + LibraryName + " is not valid")
                #
                # if not os.path.exists(self.LibraryScriptDir + "/Libraries"):
                #     os.mkdir(self.LibraryScriptDir + "/Libraries")
                #
                # with open(self.LibraryScriptDir + "/Libraries/" + LibraryName + ".zip", "wb") as file:
                #     file.write(response.content)
                #
                # with zipfile.ZipFile(self.LibraryScriptDir + "/Libraries/" + LibraryName + ".zip", "r") as zip_ref:
                #     zip_ref.extractall(self.LibraryScriptDir + "/Libraries")
                #     extractFolderName = zip_ref.namelist()[0]
                #     os.rename(
                #         self.LibraryScriptDir + "/Libraries/" + extractFolderName,
                #         self.LibraryScriptDir + "/Libraries/" + LibraryName,
                #     )
                #
                # LibraryClass.folder = os.path.join(
                #     os.getcwd(), self.LibraryScriptDir + "/Libraries/" + LibraryName
                # )
                # os.remove(
                #     self.LibraryScriptDir + "/Libraries/" + LibraryName + ".zip"
                # )
                #
                # self.enumerateExternals(LibraryClass.folder, libraryName)
                # self.enumeratePureDataObjs()
                # self.getAllSupportedObjects()
                # return True

            # except Exception as e:
            #     pd4web_print("" + str(e), color="red")
            #     pd4web_print("" + str(ResponseJson["message"]), color="red")
            #     pd4web_print("" + str(e), color="red")
            #     raise Exception("Error downloading " + LibraryName)
            # else:
            #     raise Exception(getPrintValue("red") + LibraryName + " is not a supported library" + getPrintValue("reset"))
            return False

