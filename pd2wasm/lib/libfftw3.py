import requests
import tarfile
import os
import sys
import shutil
import platform
from ..helpers import myprint, fixPaths
from ..helpers import emccPaths


def downloadAndBuild_FFTW3(webpdPatchSelf): # defined in PdWebCompiler.py
    from ..pd2wasm import webpdPatch
    webpdPatchClass: webpdPatch = webpdPatchSelf # for better autocompletion

    PackagePatch = webpdPatchClass.PdWebCompilerPath

        

    if not os.path.exists(PackagePatch + "/.lib/fftw-3.3.10"):
        # print in orange
        print("\n")
        myprint("Downloading FFTW3...", color="orange")
        response = requests.get('https://www.fftw.org/fftw-3.3.10.tar.gz')
        with open(fixPaths(PackagePatch + '/.lib/fftw-3.3.10.tar.gz'), 'wb') as f:
            f.write(response.content)
        with tarfile.open(fixPaths(PackagePatch + '/.lib/fftw-3.3.10.tar.gz'), 'r:gz') as tar:
            tar.extractall(PackagePatch + '/.lib')
        os.remove(PackagePatch + '/.lib/fftw-3.3.10.tar.gz')


    # check if file projectRoot + "/.lib/fftw-3.3.10/.libs/libfftw3f.a" exists
    if os.path.exists(fixPaths(PackagePatch + "/.lib/fftw-3.3.10/.libs/libfftw3f.a")):
        webpdPatchClass.extraFlags.append(fixPaths("-I" + PackagePatch + "/.lib/fftw-3.3.10/api"))
        webpdPatchClass.extraFlags.append(fixPaths("-L" + PackagePatch + "/.lib/fftw-3.3.10/.libs"))
        webpdPatchClass.extraFlags.append("-lfftw3f")
        return True

    # go to the fftw folder
    print("\n")
    print("\033[33m" + "    Building fftw3..." + "\033[0m")
    print("\n")
    
    compilers = emccPaths()


    command = fixPaths("cd '" + PackagePatch + "/.lib/fftw-3.3.10'")
    command += f" && {compilers.configure} ./configure --enable-float --disable-fortran"
    command += f" && {compilers.make}"
    os.system(command)

    webpdPatchClass.extraFlags.append(fixPaths("-I" + PackagePatch + "/.lib/fftw-3.3.10/api"))
    webpdPatchClass.extraFlags.append(fixPaths("-L" + PackagePatch + "/.lib/fftw-3.3.10/.libs"))
    webpdPatchClass.extraFlags.append("-lfftw3f")
    return True

