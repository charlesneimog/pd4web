import os
import shutil
from .ExternalClass import PureDataExternals
# from ..helpers import myprint

def pmpd_extra(librarySelf: PureDataExternals):
    '''
    This function copy some things that I already need to compile some externals in cyclone
    '''
    # print in orange, executing earplug function
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    folder = librarySelf.folder
    # search for pmpd.h and pmpd_version.h and copy to externals folder
    for root, _, files in os.walk(folder):
        for file in files:
            if file == "pmpd.h":
                shutil.copy(os.path.join(root, file), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "pmpd.h"))
            if file == "pmpd_version.h":
                shutil.copy(os.path.join(root, file), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "pmpd_version.h"))
            if file == "pmpd2d.h":
                shutil.copy(os.path.join(root, file), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "pmpd2d.h"))
            if file == "pmpd3d.h":
                shutil.copy(os.path.join(root, file), os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "pmpd3d.h"))

    if librarySelf.webpdPatch == None:
        raise Exception("PdWebCompilerPath is not set")
        

    emCmake = librarySelf.webpdPatch.PdWebCompilerPath + '/emsdk/upstream/emscripten/emcmake cmake '
    # make the build folder
    if not os.path.exists(os.path.join(folder, "build")):
        os.makedirs(os.path.join(folder, "build"))

    # check if pmpd_export.h already exists, if not, run cmake
    if not os.path.exists(os.path.join(folder, "build", "pmpd_export.h")):
        os.system(f"{emCmake} -B {os.path.join(folder, 'build')} {folder} -Wno-dev")

    shutil.copy(os.path.join(folder, "build", "pmpd_export.h"), 
                os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "pmpd_export.h"))

    shutil.copy(os.path.join(folder, "build", "pmpd_version.c"),
                os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals", "pmpd_version.c"))

    if "pmpd" in librarySelf.usedObjs:
        for file in ["pmpd_core.c", "pmpd_set.c", "pmpd_get.c", "pmpd_list.c", "pmpd_tab.c", "pmpd_test.c", "pmpd_stat.c", "pmpd_various.c", "pmpd_deprecated.c"]:
            shutil.copy(os.path.join(folder, file),
                    os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", file))



