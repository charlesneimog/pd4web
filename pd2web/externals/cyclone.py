import os
import shutil
from .ExternalClass import PureDataExternals
from ..helpers import myprint
from ..pd2wasm import webpdPatch


def cyclone_extra(librarySelf: PureDataExternals):
    '''
    This function copy some things that I already need to compile some externals in cyclone
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    folder = librarySelf.folder
    folders = os.listdir(folder)
    for i in folders:
        if i == "shared": # in case porres change something
            sharedFiles = os.listdir(os.path.join(folder, i))
            for k in sharedFiles: # k is yet another folder
                theFile = os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", k)
                if not os.path.exists(theFile):
                    shutil.copytree(os.path.join(folder, i, k), theFile)

    if 'coll' in librarySelf.usedObjs:
       
        shutil.copy(librarySelf.PROJECT_ROOT + "webpatch/includes/common/file.c", 
                    "webpatch/externals/file.c")

    librarySelf.extraFuncExecuted = True


