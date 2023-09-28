import requests
import tarfile
import os
from ..helpers import myprint, fixPaths
from ..helpers import emccPaths
import platform


def downloadAndBuild_FFTW3(webpdPatchSelf): # defined in PdWebCompiler.py
    from ..pd2wasm import webpdPatch
    webpdPatchClass: webpdPatch = webpdPatchSelf # for better autocompletion
    PackagePatch = webpdPatchClass.PdWebCompilerPath
    
    if not os.path.exists(PackagePatch + "/.lib/fftw-3.3.10"):
        myprint("Downloading FFTW3...", color="orange")
        response = requests.get('https://www.fftw.org/fftw-3.3.10.tar.gz')
        outputFile = fixPaths(PackagePatch + '/.lib/fftw-3.3.10.tar.gz')
        with open(outputFile, 'wb') as f:
            f.write(response.content)
        with tarfile.open(fixPaths(PackagePatch + '/.lib/fftw-3.3.10.tar.gz'), 'r:gz') as tar:
            tar.extractall(PackagePatch + '/.lib')
        os.remove(PackagePatch + '/.lib/fftw-3.3.10.tar.gz')

    if os.path.exists(fixPaths(PackagePatch + "/.lib/fftw-3.3.10/.libs/libfftw3f.a")):
        webpdPatchClass.extraFlags.append(fixPaths("-I" + PackagePatch + "/.lib/fftw-3.3.10/api"))
        webpdPatchClass.extraFlags.append(fixPaths("-L" + PackagePatch + "/.lib/fftw-3.3.10/.libs"))
        webpdPatchClass.extraFlags.append("-lfftw3f")
        return True

    if platform.system() == "Windows" or platform.system() == "Darwin":
        myprint("On Windows and Mac we use the compiled version of libfftw3", color="red")
        webpdPatchClass.extraFlags.append(fixPaths("-I" + PackagePatch + "\\.lib\\fftw-3.3.10\\api"))
        webpdPatchClass.extraFlags.append(fixPaths('-L' +  PackagePatch + '\\lib\\compiled\\'))
        webpdPatchClass.extraFlags.append("-lfftw3f")
        return True

    myprint("Building fftw3...", color="orange")
    compilers = emccPaths()
    command = ("cd '" + fixPaths(PackagePatch + "/.lib/fftw-3.3.10'"))
    command += f" && {compilers.configure} ./configure --enable-float --disable-fortran" # for now, weaudio just use float
    command += f" && {compilers.make}"
    
    os.system(command)
    webpdPatchClass.extraFlags.append(fixPaths("-I" + PackagePatch + "/.lib/fftw-3.3.10/api"))
    webpdPatchClass.extraFlags.append(fixPaths("-L" + PackagePatch + "/.lib/fftw-3.3.10/.libs"))
    webpdPatchClass.extraFlags.append("-lfftw3f")
    return True

