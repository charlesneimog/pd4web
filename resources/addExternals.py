from .external.Class import *
from .external.cyclone import *
from .external.earplug import *
from .external.pdelse import * # else can't be used as a name, using pdelse instead
from .external.convolve import *

GITHUB = "https://api.github.com/repos/{}/{}/releases"
GITHUB_TAGS = "https://api.github.com/repos/{}/{}/tags"


PD_LIBRARIES = PD_EXTERNALS()

PD_LIBRARIES.add(PureDataExternals(GITHUB, "porres", "pd-cyclone", "cyclone", cyclone_extra))
PD_LIBRARIES.add(PureDataExternals(GITHUB, "porres", "pd-else", "else", else_extra))
PD_LIBRARIES.add(PureDataExternals(GITHUB_TAGS, "pd-externals", "earplug", "earplug~", earplug_extra, single=True))
PD_LIBRARIES.add(PureDataExternals(GITHUB_TAGS, "wbrent", "convolve_tilde", "convolve", convolve_extra, single=True))


