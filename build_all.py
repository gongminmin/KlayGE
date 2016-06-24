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

	print("Building external libs...")
	build_external_libs(bi)

	print("Building KFL...")
	build_KFL(bi)

	print("Building glloader...")
	build_glloader(bi)

	print("Building DXBC2GLSL...")
	build_DXBC2GLSL(bi)

	print("Building kfont...")
	build_kfont(bi)

	print("Building MeshMLLib...")
	build_MeshMLLib(bi)

	print("Building KlayGE...")
	build_KlayGE(bi)
