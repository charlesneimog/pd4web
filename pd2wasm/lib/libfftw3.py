import requests
import tarfile
import os
import sys
import shutil
from ..helpers import myprint
from ..helpers import emccPaths


def downloadAndBuild_FFTW3(webpdPatchSelf): # defined in PdWebCompiler.py
    from ..pd2wasm import webpdPatch
    webpdPatchClass: webpdPatch = webpdPatchSelf # for better autocompletion

    projectRoot = webpdPatchClass.PdWebCompilerPath
    if not os.path.exists(projectRoot + "/.lib"):
        os.mkdir(projectRoot + "/.lib")

    if not os.path.exists(projectRoot + "/.lib/fftw-3.3.10"):
        # print in orange
        print("\n")
        myprint("Downloading FFTW3...", color="orange")
        response = requests.get('https://www.fftw.org/fftw-3.3.10.tar.gz')
        with open(projectRoot + '/.lib/fftw-3.3.10.tar.gz', 'wb') as f:
            f.write(response.content)
        with tarfile.open(projectRoot + '/.lib/fftw-3.3.10.tar.gz', 'r:gz') as tar:
            tar.extractall(projectRoot + '/.lib')
        os.remove(projectRoot + '/.lib/fftw-3.3.10.tar.gz')


    # check if file projectRoot + "/.lib/fftw-3.3.10/.libs/libfftw3f.a" exists
    if os.path.exists(projectRoot + "/.lib/fftw-3.3.10/.libs/libfftw3f.a"):
        webpdPatchClass.extraFlags.append("-I" + projectRoot + "/.lib/fftw-3.3.10/api")
        webpdPatchClass.extraFlags.append("-L" + projectRoot + "/.lib/fftw-3.3.10/.libs")
        webpdPatchClass.extraFlags.append("-lfftw3f")
        return True

    # go to the fftw folder
    print("\n")
    print("\033[33m" + "    Building fftw3..." + "\033[0m")
    print("\n")
    
    compilers = emccPaths()


    command = "cd '" + projectRoot + "/.lib/fftw-3.3.10'"
    command += f" && {compilers.configure} ./configure --enable-float --disable-fortran"
    command += f" && {compilers.make}"
    os.system(command)

    webpdPatchClass.extraFlags.append("-I" + projectRoot + "/.lib/fftw-3.3.10/api")
    webpdPatchClass.extraFlags.append("-L" + projectRoot + "/.lib/fftw-3.3.10/.libs")
    webpdPatchClass.extraFlags.append("-lfftw3f")
    return True

