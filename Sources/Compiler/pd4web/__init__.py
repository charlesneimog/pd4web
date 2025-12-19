import sys
import os
from pd4web.pypd4web import run


def main():
    folder = os.path.abspath(os.path.dirname(__file__))
    args = sys.argv
    args += ["--pd4web-root", folder]
    run(args)
