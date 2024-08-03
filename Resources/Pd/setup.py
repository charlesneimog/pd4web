from setuptools import setup

APP = ['pd-pd4web.py']
OPTIONS = {
    'argv_emulation': True,
    'packages': ['cmake', 'pd4web', "pygit2"],  # List required packages
}

setup(
    app=APP,
    options={'py2app': OPTIONS},
    setup_requires=['py2app'],
)
