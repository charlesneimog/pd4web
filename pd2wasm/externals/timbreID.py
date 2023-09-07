import os
import shutil

def timbreID_extra(librarySelf):
    ''' 
    This function copy some things that I already need to compile some externals in cyclone
    '''
    # print in orange, executing earplug function
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    includeFolder = os.path.join(librarySelf.folder, "include")
    files = os.listdir(includeFolder)
    for i in files:
        if i == "tIDLib.h":
            shutil.copy(os.path.join(includeFolder, i), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    # copy the 
    srcFolder = os.path.join(librarySelf.folder, "src") 
    shutil.copy(srcFolder + "/tIDLib.c", os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"))

    

 
    




