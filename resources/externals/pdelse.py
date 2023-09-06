import os
import shutil


def else_extra(librarySelf):
    '''
    This function copy some things that I already need to compile some externals in cyclone
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))
    folder = os.path.join(librarySelf.folder, "Code_source", "shared")

    for file in os.listdir(folder):
        if file.endswith(".h"):
            shutil.copy(os.path.join(folder, file),
                        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    for file in os.listdir(folder):
        if file.endswith(".c"):
            shutil.copy(os.path.join(folder, file),
                        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"))

    os.remove(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals", "s_elseutf8.c"))
    librarySelf.extraFuncExecuted = True
