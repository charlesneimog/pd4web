import os
import shutil

def cyclone_extra(librarySelf):
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
                shutil.copytree(os.path.join(folder, i, k), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", k))

    librarySelf.extraFuncExecuted = True


