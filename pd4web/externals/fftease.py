import os
import shutil

from .ExternalClass import PureDataExternals

# from ..helpers import myprint


def fftease_extra(librarySelf: PureDataExternals):
    """
    This function copy some things that I already need to compile some externals in cyclone
    """
    # print in orange, executing earplug function
    if librarySelf.extraFuncExecuted:
        return

    if not os.path.exists(
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")
    ):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    folder = librarySelf.folder
    for root, _, files in os.walk(folder):
        for file in files:
            for required in [
                "fftease.h",
                "fftease_utilities.c",
                "fold.c",
                "fft4.c",
                "overlapadd.c",
                "makewindows.c",
            ]:
                if file == required:
                    shutil.copy(
                        os.path.join(root, file),
                        os.path.join(
                            librarySelf.PROJECT_ROOT, "webpatch", "externals", file
                        ),
                    )
