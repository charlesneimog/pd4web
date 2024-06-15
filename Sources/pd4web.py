import os

from pd4web2 import ExternalLibraries, GetCode, Patch


class Pd4Web():
    def __init__(self, patch):
        self.Patch = patch
        self.GetPatchRoot()

        Libraries = ExternalLibraries(self.PROJECT_ROOT)
        ProcessedPatch = Patch(self.Patch, Libraries)
        CodeRetrieved = GetCode(ProcessedPatch, Libraries)

    def GetPatchRoot(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(__file__))









if __name__ == '__main__':
    # just for development
    WebPatch = Pd4Web("./Resources/Patches/pd4web.pd")
    
