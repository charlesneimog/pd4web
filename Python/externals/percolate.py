import os
import shutil
import sys

from ..helpers import myprint
from .ExternalClass import PureDataExternals


def percolate_extra(librarySelf: PureDataExternals):
    """
    This function copy some things that I already need to compile some externals in cyclone
    """
    # librarySelf.webpdPatch.PROJECT_ROOT
    if not os.path.exists(
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")
    ):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))
    if not os.path.exists(
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals")
    ):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"))

    shutil.copy(
        librarySelf.folder + "/percolate/stk.c",
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"),
    )

    shutil.copy(
        librarySelf.folder + "/percolate/stk_c.h",
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"),
    )
    shutil.copy(
        librarySelf.folder + "/percolate/sinewave.h",
        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"),
    )

    if "marimba~" in librarySelf.usedObjs:
        shutil.copy(
            librarySelf.folder + "/percolate/marmstk1.h",
            os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"),
        )
