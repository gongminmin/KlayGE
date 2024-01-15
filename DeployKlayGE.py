#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys
from Build import BuildInfo

def CopyToDst(src_name, dst_dir):
	print(f"Copying {src_name} to {dst_dir}...")
	import shutil
	shutil.copy2(src_name, dst_dir)

def DeployKlayGE(target_dir, build_info, compiler_info, cfg):
	import glob

	bin_src_dir = f"KlayGE/bin/{build_info.target_platform}_{compiler_info.arch}/"
	bin_dst_dir = f"{target_dir}/bin/{build_info.target_platform}_{compiler_info.arch}/"
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
	output_suffix = f"_{build_info.compiler_name}{build_info.compiler_version}"
	if cfg == "Debug":
		debug_suffix = "_d"
	else:
		debug_suffix = ""
	lib_suffix = output_suffix + debug_suffix + dll_suffix

	if build_info.is_windows and ((build_info.compiler_name == "vc") or (build_info.compiler_name == "clangcl")):
		lib_prefix = ""
	else:
		lib_prefix = "lib"

	if not os.path.exists(f"{target_dir}/bin"):
		os.mkdir(f"{target_dir}/bin");
	if not os.path.exists(bin_dst_dir):
		os.mkdir(bin_dst_dir);
	if not os.path.exists(f"{bin_dst_dir}Audio/"):
		os.mkdir(f"{bin_dst_dir}Audio/");
	if not os.path.exists(f"{bin_dst_dir}Input/"):
		os.mkdir(f"{bin_dst_dir}Input/");
	if not os.path.exists(f"{bin_dst_dir}Render/"):
		os.mkdir(f"{bin_dst_dir}Render/");
	if not os.path.exists(f"{bin_dst_dir}Scene/"):
		os.mkdir(f"{bin_dst_dir}Scene/");
	if not os.path.exists(f"{bin_dst_dir}Script/"):
		os.mkdir(f"{bin_dst_dir}Script/");
	if not os.path.exists(f"{bin_dst_dir}Show/"):
		os.mkdir(f"{bin_dst_dir}Show/");

	CopyToDst("KlayGE/bin/KlayGE.cfg", f"{target_dir}/bin/");

	print("\nDeploying 7z...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}7zxa{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}LZMA{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying DXSDK...")
	for fname in glob.iglob(f"{bin_src_dir}d3dcompiler_47.dll"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying OpenAL...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}OpenAL{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying assimp...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}assimp{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying FreeImage...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}FreeImage{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying zlib...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}zlib{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying glloader...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}glloader{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying kfont...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}kfont{lib_suffix}"):
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
			CopyToDst(f"{system32}/msvcp140{vc_debug_suffix}.dll", bin_dst_dir)
			CopyToDst(f"{system32}/vcruntime140{vc_debug_suffix}.dll", bin_dst_dir)
			if (build_info.compiler_version >= 142):
				CopyToDst(f"{system32}/vcruntime140_1{vc_debug_suffix}.dll", bin_dst_dir)
		elif build_info.compiler_name == "mgw":
			CopyToDst(f"{compiler_info.compiler_root}/libwinpthread-1.dll", bin_dst_dir)
			CopyToDst(f"{compiler_info.compiler_root}/libgcc_s_seh-1.dll", bin_dst_dir)
			CopyToDst(f"{compiler_info.compiler_root}/libstdc++-6.dll", bin_dst_dir)

	print("\nDeploying KlayGE...")
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}KlayGE_Core{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}KlayGE_DevHelper*{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob(f"{bin_src_dir}Audio/{lib_prefix}KlayGE_Audio*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Audio/");
	for fname in glob.iglob(f"{bin_src_dir}Input/{lib_prefix}KlayGE_Input*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Input/");
	for fname in glob.iglob(f"{bin_src_dir}Render/{lib_prefix}KlayGE_Render*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Render/");
	for fname in glob.iglob(f"{bin_src_dir}Scene/{lib_prefix}KlayGE_Scene*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Scene/");
	for fname in glob.iglob(f"{bin_src_dir}Script/{lib_prefix}KlayGE_Script*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Script/");
	for fname in glob.iglob(f"{bin_src_dir}Show/{lib_prefix}KlayGE_Show*{lib_suffix}"):
		CopyToDst(fname, f"{bin_dst_dir}Show/");
	for fname in glob.iglob(f"{bin_src_dir}Cooker{debug_suffix}{exe_suffix}"):
		CopyToDst(fname, bin_dst_dir);
	for fname in glob.iglob(f"{bin_src_dir}{lib_prefix}ToolCommon{lib_suffix}"):
		CopyToDst(fname, bin_dst_dir);

	print("\nDeploying media files...")

	if not os.path.exists(f"{target_dir}/media"):
		os.mkdir(f"{target_dir}/media");
	if not os.path.exists(f"{target_dir}/media/Fonts"):
		os.mkdir(f"{target_dir}/media/Fonts");
	if not os.path.exists(f"{target_dir}/media/Models"):
		os.mkdir(f"{target_dir}/media/Models");
	if not os.path.exists(f"{target_dir}/media/PlatConf"):
		os.mkdir(f"{target_dir}/media/PlatConf");
	if not os.path.exists(f"{target_dir}/media/PostProcessors"):
		os.mkdir(f"{target_dir}/media/PostProcessors");
	if not os.path.exists(f"{target_dir}/media/RenderFX"):
		os.mkdir(f"{target_dir}/media/RenderFX");
	if not os.path.exists(f"{target_dir}/media/Textures"):
		os.mkdir(f"{target_dir}/media/Textures");
	if not os.path.exists(f"{target_dir}/media/Textures/2D"):
		os.mkdir(f"{target_dir}/media/Textures/2D");
	if not os.path.exists(f"{target_dir}/media/Textures/3D"):
		os.mkdir(f"{target_dir}/media/Textures/3D");
	if not os.path.exists(f"{target_dir}/media/Textures/Cube"):
		os.mkdir(f"{target_dir}/media/Textures/Cube");
	if not os.path.exists(f"{target_dir}/media/Textures/Juda"):
		os.mkdir(f"{target_dir}/media/Textures/Juda");

	CopyToDst("KlayGE/media/Fonts/gkai00mp.kfont", f"{target_dir}/media/Fonts/");
	CopyToDst("KlayGE/media/Models/AmbientLightProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/CameraProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/DirectionalLightProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/IndirectLightProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/PointLightProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/SpotLightProxy.glb", f"{target_dir}/media/Models/");
	CopyToDst("KlayGE/media/Models/TubeLightProxy.glb", f"{target_dir}/media/Models/");
	for fname in glob.iglob("KlayGE/media/PlatConf/*.plat"):
		CopyToDst(fname, f"{target_dir}/media/PlatConf");
	for fname in glob.iglob("KlayGE/media/PostProcessors/*.ppml"):
		CopyToDst(fname, f"{target_dir}/media/PostProcessors");
	for fname in glob.iglob("KlayGE/media/RenderFX/*.fxml"):
		CopyToDst(fname, f"{target_dir}/media/RenderFX");
	for fname in glob.iglob("KlayGE/media/Textures/2D/*.dds"):
		CopyToDst(fname, f"{target_dir}/media/Textures/2D/");
	CopyToDst("KlayGE/media/Textures/3D/color_grading.dds", f"{target_dir}/media/Textures/3D/");
	for fname in glob.iglob("KlayGE/media/Textures/Cube/Lake_CraterLake03_*.dds"):
		CopyToDst(fname, f"{target_dir}/media/Textures/Cube/");
	for fname in glob.iglob("KlayGE/media/Textures/Cube/rnl_cross_*.dds"):
		CopyToDst(fname, f"{target_dir}/media/Textures/Cube/");
	for fname in glob.iglob("KlayGE/media/Textures/Cube/uffizi_cross_*.dds"):
		CopyToDst(fname, f"{target_dir}/media/Textures/Cube/");
	for fname in glob.iglob("KlayGE/media/Textures/Juda/*.jdt"):
		CopyToDst(fname, f"{target_dir}/media/Textures/Juda/");

if __name__ == "__main__":
	if len(sys.argv) > 1:
		target_dir = sys.argv[1]
		build_info = BuildInfo.FromArgv(sys.argv, 1)
		for cfg in build_info.cfg:
			for compiler_info in build_info.compilers:
				DeployKlayGE(target_dir, build_info, compiler_info, cfg)
	else:
		print("Usage: DeployKlayGE.py target_dir [project] [compiler] [arch] [config]")
