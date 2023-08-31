import os
import shutil

def earplug_extra(librarySelf):
    ''' 
    This function copy some things that I already need to compile some externals in cyclone
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.ROOT, "webpatch", "externals")):
        os.makedirs(os.path.join(librarySelf.ROOT, "webpatch", "externals"))

    if not os.path.exists(os.path.join(librarySelf.ROOT, "webpatch", "data")):
        os.makedirs(os.path.join(librarySelf.ROOT, "webpatch", "data"))

    folder = librarySelf.folder
    shutil.copy(os.path.join(folder, "earplug_data.txt"), os.path.join(librarySelf.ROOT, "webpatch", "data/earplug_data.txt"))
    
            



    

 
    




