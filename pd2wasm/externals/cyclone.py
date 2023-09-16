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
                theFile = os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", k)
                if not os.path.exists(theFile):
                    shutil.copytree(os.path.join(folder, i, k), theFile)

    if 'coll' in librarySelf.usedObjs:
        librarySelf.webpdPatch.print("Coll object is not supported yet", color="red")
        shutil.copy("webpatch/includes/common/file.c", "webpatch/externals/file.c")

    elif 'sfz~' in librarySelf.usedObjs:
        librarySelf.webpdPatch.print("sfz~ object is not supported yet", color="red")

    elif 'sfont~' in librarySelf.usedObjs:
        librarySelf.webpdPatch.print("sfont~ object is not supported yet", color="red")

    elif 'plaits~' in librarySelf.usedObjs:
        librarySelf.webpdPatch.print("plaits~ object is not supported yet", color="red")



    librarySelf.extraFuncExecuted = True


