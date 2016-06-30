#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from build_external import build_external_libs
from blib_util import *
from build_KFL import *
from build_glloader import *
from build_DXBC2GLSL import *
from build_kfont import *
from build_MeshMLLib import *
from build_KlayGE import *

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
	build_KFL(bi)
	build_glloader(bi)
	build_DXBC2GLSL(bi)
	build_kfont(bi)
	build_MeshMLLib(bi)
	build_KlayGE(bi)
