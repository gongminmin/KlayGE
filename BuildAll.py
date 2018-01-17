#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *
from BuildExternal import BuildExternalLibs
from BuildKFL import *
from BuildGlloader import *
from BuildDXBC2GLSL import *
from BuildKFont import *
from BuildMeshMLLib import *
from BuildKlayGE import *

if __name__ == "__main__":
	bi = BuildInfo.FromArgv(sys.argv)

	BuildExternalLibs(bi)
	BuildKFL(bi)
	BuildGlloader(bi)
	BuildDXBC2GLSL(bi)
	BuildKFont(bi)
	BuildMeshMLLib(bi)
	BuildKlayGE(bi)
