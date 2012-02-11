#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os
import sys
from build_external import build_external_libs
from build_external import copy_to_dst

if __name__ == "__main__":
	if len(sys.argv) > 1:
		cfg = sys.argv[1]
	else:
		cfg = ""

	env = os.environ

	if "" == cfg:
		if "VS100COMNTOOLS" in env:
			cfg = "vc10"
		elif "VS90COMNTOOLS" in env:
			cfg = "vc9"
		elif "VS80COMNTOOLS" in env:
			cfg = "vc8"
		elif os.path.exists("C:\MinGW\bin\gcc.exe"):
			cfg = "mingw"

	if "vc10" == cfg:
		compiler_name = "vc"
		compiler_version = 10
		ide_name = "VS"
		ide_version = 2010
	elif "vc9" == cfg:
		compiler_name = "vc"
		compiler_version = 9
		ide_name = "VS"
		ide_version = 2008
	else:
		print("Wrong configuration\n")
		sys.exit(1)

	dst_dir_x86 = "KlayGE/bin/win_x86/"
	dst_dir_x64 = "KlayGE/bin/win_x64/"


	print("Building external libs...")
	build_external_libs(cfg)

	print("Building glloader...")
	os.chdir("glloader/build/%s-%d_0" % (compiler_name, compiler_version))
	os.system("build_all.bat -q")
	os.chdir("../../../")

	copy_to_dst("glloader/lib/Win32/glloader_x86.dll", dst_dir_x86)
	copy_to_dst("glloader/lib/Win32/glloader_x86_d.dll", dst_dir_x86)
	copy_to_dst("glloader/lib/Win32/glloader_es_x86.dll", dst_dir_x86)
	copy_to_dst("glloader/lib/Win32/glloader_es_x86_d.dll", dst_dir_x86)
	copy_to_dst("glloader/lib/x64/glloader_x64.dll", dst_dir_x64)
	copy_to_dst("glloader/lib/x64/glloader_x64_d.dll", dst_dir_x64)
	copy_to_dst("glloader/lib/x64/glloader_es_x64.dll", dst_dir_x64)
	copy_to_dst("glloader/lib/x64/glloader_es_x64_d.dll", dst_dir_x64)
	
	print("Building kfont...")
	os.chdir("kfont/build/%s-%d_0" % (compiler_name, compiler_version))
	os.system("build_all.bat -q")
	os.chdir("../../../")

	copy_to_dst("kfont/lib/Win32/kfont_x86.dll", dst_dir_x86)
	copy_to_dst("kfont/lib/Win32/kfont_x86_d.dll", dst_dir_x86)
	copy_to_dst("kfont/lib/x64/kfont_x64.dll", dst_dir_x64)
	copy_to_dst("kfont/lib/x64/kfont_x64_d.dll", dst_dir_x64)

	print("Building KlayGE...")
	os.chdir("KlayGE/Build/%s-%d_0" % (compiler_name, compiler_version))
	os.system("build_all.bat -q")
	os.chdir("../../../")

	print("Building KlayGE Samples...")
	os.chdir("KlayGE/Samples/build/%s-%d_0" % (compiler_name, compiler_version))
	os.system("build_all.bat -q")
	os.chdir("../../../../")

	print("Building KlayGE Tools...")
	os.chdir("KlayGE/Tools/build/%s-%d_0" % (compiler_name, compiler_version))
	os.system("build_all.bat -q")
	os.chdir("../../../../")
