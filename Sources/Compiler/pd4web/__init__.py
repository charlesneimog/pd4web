import sys
import os
from pd4web.pypd4web import run


def main():
    folder = os.path.abspath(os.path.dirname(__file__))
    args = sys.argv

    # add pd4web folder
    args += ["--pd4web-folder", os.path.join(folder, "Pd4Web")]
    run(args)
