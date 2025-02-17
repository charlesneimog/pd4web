import argparse

from .pd4web import Pd4Web


def main():
    Pd4WebInstance = Pd4Web()
    Pd4WebInstance.arg_parse()


if __name__ == "__main__":
    main()
