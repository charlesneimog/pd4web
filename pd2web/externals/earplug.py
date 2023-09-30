import os
import shutil
from .ExternalClass import PureDataExternals
# from ..helpers import myprint

def earplug_extra(librarySelf: PureDataExternals):
    '''
    This function copy some things that I already need to compile some externals in cyclone
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"))

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "data")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "data"))

    folder = librarySelf.folder
    shutil.copy(os.path.join(folder, "earplug_data.txt"), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "data/earplug_data.txt"))











