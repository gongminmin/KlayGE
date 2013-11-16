#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from build_external import build_external_libs
from blib_util import *
from build_KFL import *
from build_glloader import *
from build_kfont import *
from build_MeshMLLib import *
from build_KlayGE import *

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	ci = compiler_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building external libs...")
	build_external_libs(ci)

	print("Building KFL...")
	build_KFL(ci)

	print("Building glloader...")
	build_glloader(ci)

	print("Building kfont...")
	build_kfont(ci)

	print("Building MeshMLLib...")
	build_MeshMLLib(ci)

	print("Building KlayGE...")
	build_KlayGE(ci)

	print("Building KlayGE Samples...")
	build_Samples(ci)

	print("Building KlayGE Tools...")
	build_Tools(ci)

	print("Building KlayGE Tutorials...")
	build_Tutorials(ci)

	print("Building KlayGE Exporters...")
	build_Exporters(ci)
