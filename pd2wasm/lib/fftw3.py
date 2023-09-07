import requests
import tarfile
import os
import sys
import shutil

def downloadAndBuild_FFTW3(webpdPatchClass): # defined in PdWebCompiler.py

    projectRoot = webpdPatchClass.PROJECT_ROOT

    if not os.path.exists(projectRoot + "/.lib"):
        os.mkdir(projectRoot + "/.lib")

    if not os.path.exists(projectRoot + "/.lib/fftw-3.3.10"):
        # print in orange
        print("\n")
        print("\033[33m" + "    Downloading FFTW3..." + "\033[0m")
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

    # check if emconfigure and emmake are in the PATH
    if shutil.which("emconfigure") is None or shutil.which("emmake") is None:
        webpdPatchClass.printError("\033[91m" + "    emconfigure or emmake are not in the PATH. Please install Emscripten." + "\033[0m")
        print("")
        sys.exit(-1)

    # go to the fftw folder
    print("\n")
    print("\033[33m" + "    Building fftw3..." + "\033[0m")
    print("\n")

    command1 = "cd '" + projectRoot + "/.lib/fftw-3.3.10'"
    command1 += " && emconfigure ./configure --enable-float --disable-fortran"
    command1 += " && emmake make"
    os.system(command1)

    webpdPatchClass.extraFlags.append("-I" + projectRoot + "/.lib/fftw-3.3.10/api")
    webpdPatchClass.extraFlags.append("-L" + projectRoot + "/.lib/fftw-3.3.10/.libs")
    webpdPatchClass.extraFlags.append("-lfftw3f")
    return True

