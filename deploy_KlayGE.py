#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy(src_name, dst_dir)

def deploy_KlayGE(target_dir, compiler_name, compiler_version, compiler_arch, platform):
	import glob

	if "win32" == platform:
		bin_dst_dir = "%s/bin/win_%s/" % (target_dir, compiler_arch)
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == platform:
		bin_dst_dir = "%s/bin/linux_%s/" % (target_dir, compiler_arch)
		bat_suffix = "sh"
		dll_suffix = "so"
		
	if not os.path.exists("%s/bin" % target_dir):
		os.mkdir("%s/bin" % target_dir);
	if not os.path.exists(bin_dst_dir):
		os.mkdir(bin_dst_dir);
	if not os.path.exists("%sAudio/" % bin_dst_dir):
		os.mkdir("%sAudio/" % bin_dst_dir);
	if not os.path.exists("%sInput/" % bin_dst_dir):
		os.mkdir("%sInput/" % bin_dst_dir);
	if not os.path.exists("%sRender/" % bin_dst_dir):
		os.mkdir("%sRender/" % bin_dst_dir);
	if not os.path.exists("%sScene/" % bin_dst_dir):
		os.mkdir("%sScene/" % bin_dst_dir);
	if not os.path.exists("%sShow/" % bin_dst_dir):
		os.mkdir("%sShow/" % bin_dst_dir);

	copy_to_dst("KlayGE/bin/KlayGE.cfg", "%s/bin/" % target_dir);
	
	print("Deploying boost...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/boost_*.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);

	print("Deploying Python...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/python*.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);

	print("Deploying 7z...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/7zxa*.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/LZMA*.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);
	
	print("Deploying DXSDK...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/d3dcompiler_46.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);
	
	print("Deploying OpenAL...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/OpenAL32.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);

	print("Deploying Cg...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/cg.%s" % (compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);
		
	print("Deploying glloader...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/glloader_%s_%s.%s" % (compiler_arch, compiler_name, compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);

	print("Deploying kfont...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/kfont_%s_%s.%s" % (compiler_arch, compiler_name, compiler_arch, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);

	print("Deploying KlayGE...\n")
	for fname in glob.iglob("KlayGE/bin/win_%s/KlayGE_Core_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/Audio/KlayGE_Audio*_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, "%sAudio/" % bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/Input/KlayGE_Input*_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, "%sInput/" % bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/Render/KlayGE_Render*_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, "%sRender/" % bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/Scene/KlayGE_Scene*_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, "%sScene/" % bin_dst_dir);
	for fname in glob.iglob("KlayGE/bin/win_%s/Show/KlayGE_Show*_%s*.%s" % (compiler_arch, compiler_name, dll_suffix)):
		copy_to_dst(fname, "%sShow/" % bin_dst_dir);

	print("Deploying media files...\n")

	if not os.path.exists("%s/media" % target_dir):
		os.mkdir("%s/media" % target_dir);
	if not os.path.exists("%s/media/Fonts" % target_dir):
		os.mkdir("%s/media/Fonts" % target_dir);
	if not os.path.exists("%s/media/Models" % target_dir):
		os.mkdir("%s/media/Models" % target_dir);
	if not os.path.exists("%s/media/PostProcessors" % target_dir):
		os.mkdir("%s/media/PostProcessors" % target_dir);
	if not os.path.exists("%s/media/RenderFX" % target_dir):
		os.mkdir("%s/media/RenderFX" % target_dir);
	if not os.path.exists("%s/media/Textures" % target_dir):
		os.mkdir("%s/media/Textures" % target_dir);
	if not os.path.exists("%s/media/Textures/2D" % target_dir):
		os.mkdir("%s/media/Textures/2D" % target_dir);
	if not os.path.exists("%s/media/Textures/3D" % target_dir):
		os.mkdir("%s/media/Textures/3D" % target_dir);
	if not os.path.exists("%s/media/Textures/Cube" % target_dir):
		os.mkdir("%s/media/Textures/Cube" % target_dir);
	if not os.path.exists("%s/media/Textures/Juda" % target_dir):
		os.mkdir("%s/media/Textures/Juda" % target_dir);
	
	copy_to_dst("KlayGE/media/Fonts/gkai00mp.kfont", "%s/media/Fonts/" % target_dir);
	copy_to_dst("KlayGE/media/Models/ambient_light_proxy.meshml", "%s/media/Models/" % target_dir);
	copy_to_dst("KlayGE/media/Models/directional_light_proxy.meshml", "%s/media/Models/" % target_dir);
	copy_to_dst("KlayGE/media/Models/indirect_light_proxy.meshml", "%s/media/Models/" % target_dir);
	copy_to_dst("KlayGE/media/Models/point_light_proxy.meshml", "%s/media/Models/" % target_dir);
	copy_to_dst("KlayGE/media/Models/spot_light_proxy.meshml", "%s/media/Models/" % target_dir);
	for fname in glob.iglob("KlayGE/media/PostProcessors/*.ppml"):
		copy_to_dst(fname, "%s/media/PostProcessors" % target_dir);
	for fname in glob.iglob("KlayGE/media/RenderFX/*.fxml"):
		copy_to_dst(fname, "%s/media/RenderFX" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/2D/*.dds"):
		copy_to_dst(fname, "%s/media/Textures/2D/" % target_dir);
	copy_to_dst("KlayGE/media/Textures/3D/color_grading.dds", "%s/media/Textures/3D/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/Lake_CraterLake03_*.dds"):
		copy_to_dst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/rnl_cross_*.dds"):
		copy_to_dst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/uffizi_cross_*.dds"):
		copy_to_dst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Juda/*.jdt"):
		copy_to_dst(fname, "%s/media/Textures/Juda/" % target_dir);

if __name__ == "__main__":	
	if len(sys.argv) > 1:
		target_dir = sys.argv[1]
	else:
		target_dir = ""
	if len(sys.argv) > 2:
		compiler = sys.argv[2]
	else:
		compiler = ""
	if len(sys.argv) > 3:
		arch = (sys.argv[3], )
	else:
		arch = ""
	if len(sys.argv) > 4:
		cfg = sys.argv[4]
	else:
		cfg = ""

	compiler_info = get_compiler_info(compiler, arch, cfg)
	
	if 0 == len(compiler_info):
		print("No target folder specified\n")
		sys.exit(1)

	for arch in compiler_info[2]:
		deploy_KlayGE(target_dir, compiler_info[0], compiler_info[1], arch[0], compiler_info[4])
