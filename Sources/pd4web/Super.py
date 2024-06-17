import os


class Pd4Web():
    def __init__(self, patch, Recursive=False):
        from .GetCode import GetCode
        from .Libraries import ExternalLibraries
        from .Patch import Patch

        # Sobre a recursivade para patch, talvez não chamar o construtor de Pd4Web, 
        # mas some a mesma ordem para __init__.
        self.Patch = patch
        self.InitVariables()

        self.Libraries = ExternalLibraries(self)
        self.ProcessedPatch = Patch(self)
        self.CodeRetrieved = GetCode(self)

    def InitVariables(self):
        self.PROJECT_ROOT = os.path.dirname(os.path.realpath(__file__))
        self.PD4WEB_ROOT = os.path.dirname(os.path.realpath(__file__))

        # Used Objects
        self.UsedObjects = []

        # Compiler and Code Variables
        self.UiReceiversSymbol = []
        self.ExternalsSourceCode = []
        self.ExternalsExtraFlags = []
        self.ExternalsLinkLibraries = []

    
