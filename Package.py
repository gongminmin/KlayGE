#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, shutil, sys
from Build import BuildInfo
from DeployKlayGE import DeployKlayGE, CopyToDst

def PackageSamples(tareget_dir, build_info, compiler_info, cfg):
	if not os.path.exists(tareget_dir):
		os.mkdir(tareget_dir)
	dst_sample_dir = f"{tareget_dir}/KlayGE_Samples_{build_info.compiler_name}{build_info.compiler_version}_{build_info.target_platform}_{compiler_info.arch}/"
	if os.path.exists(dst_sample_dir):
		shutil.rmtree(dst_sample_dir)
	os.mkdir(dst_sample_dir)
	DeployKlayGE(dst_sample_dir, build_info, compiler_info, cfg)

	output_suffix = f"_{build_info.compiler_name}{build_info.compiler_version}"
	if cfg == "Debug":
		debug_suffix = "_d"
	else:
		debug_suffix = ""
	if build_info.is_windows:
		exe_suffix = ".exe"
		dll_suffix = ".dll"
	elif build_info.is_darwin:
		exe_suffix = ""
		dll_suffix = ".dylib"
	else:
		exe_suffix = ""
		dll_suffix = ".so"
	if build_info.is_windows and ((build_info.compiler_name == "vc") or (build_info.compiler_name == "clangcl")):
		lib_prefix = ""
	else:
		lib_prefix = "lib"

	src_bin_dir = f"KlayGE/bin/{build_info.target_platform}_{compiler_info.arch}/"
	dst_bin_dir = f"{dst_sample_dir}bin/{build_info.target_platform}_{compiler_info.arch}/"

	exe_list = (
		"AreaLighting",
		"AtmosphericScattering",
		"CascadedShadowMap",
		"CausticsMap",
		"DeepGBuffers",
		"DeferredRendering",
		"DetailedSurface",
		"DetailedSurfaceDR",
		"EnvLighting",
		"Foliage",
		"GPUParticleSystem",
		"JudaTexViewer",
		"Metalness",
		"MotionBlurDoF",
		"Ocean",
		"OIT",
		"ParticleEditor",
		"PostProcessing",
		"ProceduralTex",
		"Reflection",
		"ScenePlayer",
		"ShadowCubeMap",
		"SkinnedMesh",
		"Sound",
		"SSSSS",
		"SubSurface",
		"Text",
		"VDMParticle",
		"VectorTex",
		"VideoTexture",
	)

	for exe_item in exe_list:
		CopyToDst(src_bin_dir + exe_item + output_suffix + debug_suffix + exe_suffix, dst_bin_dir)

	CopyToDst(src_bin_dir + "MotionBlurDoFPy.zip", dst_bin_dir)
	CopyToDst(src_bin_dir + "ScenePlayerPy.zip", dst_bin_dir)

	CopyToDst("LICENSE", dst_sample_dir)
	CopyToDst("README.md", dst_sample_dir)
	CopyToDst("KlayGE/klayge_logo.ico", dst_sample_dir)

	print("Copying Samples/media folder...")
	if not os.path.exists(dst_sample_dir + "Samples"):
		os.mkdir(dst_sample_dir + "Samples")
	if os.path.exists(dst_sample_dir + "Samples/media"):
		shutil.rmtree(dst_sample_dir + "Samples/media")
	shutil.copytree("KlayGE/Samples/media", dst_sample_dir + "Samples/media")

	print("Deploying resources...")

	import subprocess
	cooker_path = dst_bin_dir + "Cooker" + debug_suffix + exe_suffix
	if build_info.is_windows:
		cooker_path = cooker_path.replace("/", "\\")
		platform = "d3d_11_0"
	elif build_info.is_darwin:
		platform = "gl_4_1"
	else:
		platform = "gl_4_6"

	cook_list = (
		"media/Models/*.glb",
		"Samples/media/AtmosphericScattering/*.glb",
		"Samples/media/CascadedShadowMap/*.glb",
		"Samples/media/CascadedShadowMap/*.jpg",
		"Samples/media/CausticsMap/*.glb",
		"Samples/media/Common/ScifiRoom/*.3DS",
		"Samples/media/Common/ScifiRoom/*.JPG",
		"Samples/media/Common/Sponza/*.glb",
		"Samples/media/Common/Sponza/*.jpg",
		"Samples/media/Common/Sponza/*.png",
		"Samples/media/Common/*.glb",
		"Samples/media/DeepGBuffers/*.glb",
		"Samples/media/DeepGBuffers/*.jpg",
		"Samples/media/Foliage/Grass1/*.glb",
		"Samples/media/Foliage/Grass1/*.jpg",
		"Samples/media/Foliage/Grass2/*.glb",
		"Samples/media/Foliage/Grass2/*.png",
		"Samples/media/Foliage/Tree1/*.meshml",
		"Samples/media/Foliage/Tree1/*.jpg",
		"Samples/media/Foliage/Tree1/*.png",
		"Samples/media/Foliage/Tree2/*.meshml",
		"Samples/media/Foliage/Tree2/*.jpg",
		"Samples/media/Foliage/Tree2/*.png",
		"Samples/media/Metalness/*.3ds",
		"Samples/media/OIT/*.glb",
		"Samples/media/OIT/*.jpg",
		"Samples/media/SkinnedMesh/*.meshml",
		"Samples/media/SSSSS/*.glb",
		"Samples/media/SSSSS/*.jpg",
		"Samples/media/SubSurface/*.glb",
		"Samples/media/SubSurface/*.jpg",
		"Samples/media/SubSurface/*.png",
		"Samples/media/VDMParticle/*.glb",
		"Samples/media/VDMParticle/*.jpg"
	)

	import glob
	for cook_item in cook_list:
		resource_path = dst_sample_dir + cook_item
		for file in glob.glob(resource_path):
			subprocess.call([cooker_path, "-P", platform, "-I", file])
			print(f"Removing {file}...")
			os.remove(file)

	for file in glob.glob(dst_bin_dir + lib_prefix + "KlayGE_DevHelper*" + dll_suffix):
		os.remove(file)
	os.remove(dst_bin_dir + "Cooker" + debug_suffix + exe_suffix)
	for file in glob.glob(dst_bin_dir + lib_prefix + "ToolCommon*" + dll_suffix):
		os.remove(file)
	for file in glob.glob(dst_bin_dir + lib_prefix + "assimp*" + dll_suffix):
		os.remove(file)
	for file in glob.glob(dst_bin_dir + lib_prefix + "FreeImage*" + dll_suffix):
		os.remove(file)	
	for file in glob.glob(dst_sample_dir + "Samples/media/CascadedShadowMap/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Common/ScifiRoom/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Common/Sponza/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Common/*.pfx"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/DeepGBuffers/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Foliage/Grass1/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Foliage/Grass2/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Foliage/Tree1/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Foliage/Tree2/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/Metalness/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/OIT/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/SSSSS/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/SubSurface/*.kmeta"):
		os.remove(file)
	for file in glob.glob(dst_sample_dir + "Samples/media/VDMParticle/*.kmeta"):
		os.remove(file)
	shutil.rmtree(dst_sample_dir + "media/PlatConf")
	os.remove(dst_sample_dir + "Samples/media/MotionBlurDoF/MotionBlurDoFPy.zip")
	os.remove(dst_sample_dir + "Samples/media/ScenePlayer/ScenePlayerPy.zip")

if __name__ == "__main__":
	if len(sys.argv) > 1:
		target_dir = sys.argv[1]
		build_info = BuildInfo.FromArgv(sys.argv, 1)
		for cfg in build_info.cfg:
			for compiler_info in build_info.compilers:
				PackageSamples(target_dir, build_info, compiler_info, cfg)
	else:
		print("Usage: PackageSamples.py target_dir [project] [compiler] [arch] [config]")
