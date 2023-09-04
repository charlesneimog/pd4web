import os
import shutil

def piro_extra(librarySelf):
    ''' 
    Properties of librarySelf:
        * extraFuncExecuted: Set to True if extraFunc just need to be executed once
        * folder: Get the folder of the downloaded library
        * getUsedObjs: Get list of used objects when functions just must be executed for especific objects
        * name: name of the library
        * repo: Get the repository of the library
        * repoAPI: Link to the API of the repository
        * singleObject: When the object has the same name as the library and just one library object.
        * extraFlags: It is possible to add extra flags to the compilation process
    '''
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "libs")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "libs"))
    
    # get fftw3 "https://api.github.com/repos/{}/{}/tags"

    


    

 
    




