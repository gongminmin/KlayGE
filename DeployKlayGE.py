#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from Build import BuildInfo

def CopyToDst(src_name, dst_dir):
	print("Copying %s to %s..." % (src_name, dst_dir))
	import shutil
	shutil.copy2(src_name, dst_dir)

def DeployKlayGE(target_dir, build_info, compiler_info, cfg):
	import glob

	bin_src_dir = "KlayGE/bin/%s_%s/" % (build_info.target_platform, compiler_info.arch)
	bin_dst_dir = "%s/bin/%s_%s/" % (target_dir, build_info.target_platform, compiler_info.arch)
	if build_info.is_windows:
		bat_suffix = "bat"
		dll_suffix = ".dll"
		exe_suffix = ".exe"
	elif build_info.is_darwin:
		bat_suffix = "sh"
		dll_suffix = ".dylib"
		exe_suffix = ""
	else:
		bat_suffix = "sh"
		dll_suffix = ".so"
		exe_suffix = ""
	output_suffix = "_%s%d" % (build_info.compiler_name, build_info.compiler_version)
	if cfg == "Debug":
		debug_suffix = "_d"
	else:
		debug_suffix = ""
	lib_suffix = output_suffix + debug_suffix + dll_suffix

	if build_info.is_windows and ((build_info.compiler_name == "vc") or (build_info.compiler_name == "clangcl")):
		lib_prefix = ""
	else:
		lib_prefix = "lib"

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
	if not os.path.exists("%sScript/" % bin_dst_dir):
		os.mkdir("%sScript/" % bin_dst_dir);
	if not os.path.exists("%sShow/" % bin_dst_dir):
		os.mkdir("%sShow/" % bin_dst_dir);

	CopyToDst("KlayGE/bin/KlayGE.cfg", "%s/bin/" % target_dir);
	
	print("\nDeploying 7z...")
	for fname in glob.iglob("%s%s7zxa%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob("%s%sLZMA%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying DXSDK...")
	for fname in glob.iglob("%sd3dcompiler_47.dll" % bin_src_dir):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying OpenAL...")
	for fname in glob.iglob("%s%sOpenAL%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying assimp...")
	for fname in glob.iglob("%s%sassimp%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying FreeImage...")
	for fname in glob.iglob("%s%sFreeImage%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying zlib...")
	for fname in glob.iglob("%s%szlib%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying glloader...")
	for fname in glob.iglob("%s%sglloader%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying kfont...")
	for fname in glob.iglob("%s%skfont%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	if build_info.is_windows:
		if build_info.compiler_name == "vc":
			print("\nDeploying vcredist...")
			system32 = os.environ["SystemRoot"] + "/"
			if cfg == "Debug":
				vc_debug_suffix = "d"
			else:
				vc_debug_suffix = ""
			import platform
			if (platform.architecture()[0] == '32bit') and ('ProgramFiles(x86)' in os.environ):
				system32 += "SysNative"
			else:
				system32 += "System32"
			CopyToDst("%s/msvcp140%s.dll" % (system32, vc_debug_suffix), bin_dst_dir)
			CopyToDst("%s/vcruntime140%s.dll" % (system32, vc_debug_suffix), bin_dst_dir)
			if (build_info.compiler_version >= 142):
				CopyToDst("%s/vcruntime140_1%s.dll" % (system32, vc_debug_suffix), bin_dst_dir)
		elif build_info.compiler_name == "mgw":
			CopyToDst("%s/libwinpthread-1.dll" % compiler_info.compiler_root, bin_dst_dir)
			CopyToDst("%s/libgcc_s_seh-1.dll" % compiler_info.compiler_root, bin_dst_dir)
			CopyToDst("%s/libstdc++-6.dll" % compiler_info.compiler_root, bin_dst_dir)

	print("\nDeploying KlayGE...")
	for fname in glob.iglob("%s%sKlayGE_Core%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob("%s%sKlayGE_DevHelper*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob("%sAudio/%sKlayGE_Audio*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sAudio/" % bin_dst_dir);
	for fname in glob.iglob("%sInput/%sKlayGE_Input*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sInput/" % bin_dst_dir);
	for fname in glob.iglob("%sRender/%sKlayGE_Render*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sRender/" % bin_dst_dir);
	for fname in glob.iglob("%sScene/%sKlayGE_Scene*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sScene/" % bin_dst_dir);
	for fname in glob.iglob("%sScript/%sKlayGE_Script*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sScript/" % bin_dst_dir);
	for fname in glob.iglob("%sShow/%sKlayGE_Show*%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, "%sShow/" % bin_dst_dir);
	for fname in glob.iglob("%sPlatformDeployer%s%s" % (bin_src_dir, debug_suffix, exe_suffix)):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob("%s%sToolCommon%s" % (bin_src_dir, lib_prefix, lib_suffix)):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying media files...")

	if not os.path.exists("%s/media" % target_dir):
		os.mkdir("%s/media" % target_dir);
	if not os.path.exists("%s/media/Fonts" % target_dir):
		os.mkdir("%s/media/Fonts" % target_dir);
	if not os.path.exists("%s/media/Models" % target_dir):
		os.mkdir("%s/media/Models" % target_dir);
	if not os.path.exists("%s/media/PlatConf" % target_dir):
		os.mkdir("%s/media/PlatConf" % target_dir);
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

	CopyToDst("KlayGE/media/Fonts/gkai00mp.kfont", "%s/media/Fonts/" % target_dir);
	CopyToDst("KlayGE/media/Models/AmbientLightProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/CameraProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/DirectionalLightProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/IndirectLightProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/PointLightProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/SpotLightProxy.glb", "%s/media/Models/" % target_dir);
	CopyToDst("KlayGE/media/Models/TubeLightProxy.glb", "%s/media/Models/" % target_dir);
	for fname in glob.iglob("KlayGE/media/PlatConf/*.plat"):
		CopyToDst(fname, "%s/media/PlatConf" % target_dir);
	for fname in glob.iglob("KlayGE/media/PostProcessors/*.ppml"):
		CopyToDst(fname, "%s/media/PostProcessors" % target_dir);
	for fname in glob.iglob("KlayGE/media/RenderFX/*.fxml"):
		CopyToDst(fname, "%s/media/RenderFX" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/2D/*.dds"):
		CopyToDst(fname, "%s/media/Textures/2D/" % target_dir);
	CopyToDst("KlayGE/media/Textures/3D/color_grading.dds", "%s/media/Textures/3D/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/Lake_CraterLake03_*.dds"):
		CopyToDst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/rnl_cross_*.dds"):
		CopyToDst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Cube/uffizi_cross_*.dds"):
		CopyToDst(fname, "%s/media/Textures/Cube/" % target_dir);
	for fname in glob.iglob("KlayGE/media/Textures/Juda/*.jdt"):
		CopyToDst(fname, "%s/media/Textures/Juda/" % target_dir);

if __name__ == "__main__":
	if len(sys.argv) > 1:
		target_dir = sys.argv[1]
		build_info = BuildInfo.FromArgv(sys.argv, 1)
		for cfg in build_info.cfg:
			for compiler_info in build_info.compilers:
				DeployKlayGE(target_dir, build_info, compiler_info, cfg)
	else:
		print("Usage: DeployKlayGE.py target_dir [project] [compiler] [arch] [config]")
