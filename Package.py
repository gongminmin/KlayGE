#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, shutil, sys
from Build import BuildInfo
from DeployKlayGE import DeployKlayGE

def CopyToDst(src_name, dst_dir):
	print("Copying %s to %s..." % (src_name, dst_dir))
	shutil.copy2(src_name, dst_dir)

def PackageSamples(tareget_dir, build_info, compiler_arch, cfg):
	if not os.path.exists(tareget_dir):
		os.mkdir(tareget_dir)
	dst_sample_dir = "%s/KlayGE_Samples_%s%d_%s_%s/" % (tareget_dir, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_arch)
	if os.path.exists(dst_sample_dir):
		shutil.rmtree(dst_sample_dir)
	os.mkdir(dst_sample_dir)
	DeployKlayGE(dst_sample_dir, build_info, compiler_arch, cfg)

	output_suffix = "_%s%d" % (build_info.compiler_name, build_info.compiler_version)
	if cfg == "Debug":
		debug_suffix = "_d"
	else:
		debug_suffix = ""
	exe_suffix = ""
	dll_suffix = ""
	if build_info.is_windows:
		exe_suffix = ".exe"
		dll_suffix = ".dll"
	elif build_info.is_darwin:
		exe_suffix = ""
		dll_suffix = ".dylib"
	else:
		exe_suffix = ""
		dll_suffix = ".so"

	src_bin_dir = "KlayGE/bin/%s_%s/" % (build_info.target_platform, compiler_arch)
	dst_bin_dir = "%sbin/%s_%s/" % (dst_sample_dir, build_info.target_platform, compiler_arch)

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
		"KGEConfig",
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

	if build_info.is_windows:
		win_dir = os.environ["windir"]
		if cfg == "Debug":
			vc_debug_suffix = "d"
		else:
			vc_debug_suffix = ""
		CopyToDst("%s/Sysnative/msvcp140%s.dll" % (win_dir, vc_debug_suffix), dst_bin_dir)
		CopyToDst("%s/Sysnative/vcruntime140%s.dll" % (win_dir, vc_debug_suffix), dst_bin_dir)
		CopyToDst("%s/Sysnative/vcruntime140_1%s.dll" % (win_dir, vc_debug_suffix), dst_bin_dir)

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
	pd_path = dst_bin_dir + "PlatformDeployer" + debug_suffix + exe_suffix
	if build_info.is_windows:
		pd_path = pd_path.replace("/", "\\")
		platform = "d3d_11_0"
	elif build_info.is_darwin:
		platform = "gl_4_1"
	else:
		platform = "gl_4_6"

	pd_list = (
		("model", "media/Models/*.glb"),
		("model", "Samples/media/AtmosphericScattering/*.glb"),
		("model", "Samples/media/CascadedShadowMap/*.glb"),
		("albedo", "Samples/media/CascadedShadowMap/*.jpg"),
		("model", "Samples/media/CausticsMap/*.glb"),
		("model", "Samples/media/Common/ScifiRoom/*.3DS"),
		("albedo", "Samples/media/Common/ScifiRoom/*.JPG"),
		("model", "Samples/media/Common/Sponza/*.glb"),
		("albedo", "Samples/media/Common/Sponza/*.jpg"),
		("albedo", "Samples/media/Common/Sponza/*.png"),
		("model", "Samples/media/Common/*.glb"),
		("model", "Samples/media/DeepGBuffers/*.glb"),
		("albedo", "Samples/media/DeepGBuffers/*.jpg"),
		("model", "Samples/media/Foliage/Grass1/*.glb"),
		("albedo", "Samples/media/Foliage/Grass1/*.jpg"),
		("model", "Samples/media/Foliage/Grass2/*.glb"),
		("albedo", "Samples/media/Foliage/Grass2/*.png"),
		("model", "Samples/media/Foliage/Tree1/*.meshml"),
		("albedo", "Samples/media/Foliage/Tree1/*.jpg"),
		("albedo", "Samples/media/Foliage/Tree1/*.png"),
		("model", "Samples/media/Foliage/Tree2/*.meshml"),
		("albedo", "Samples/media/Foliage/Tree2/*.jpg"),
		("albedo", "Samples/media/Foliage/Tree2/*.png"),
		("model", "Samples/media/Metalness/*.3ds"),
		("model", "Samples/media/OIT/*.glb"),
		("albedo", "Samples/media/OIT/*.jpg"),
		("model", "Samples/media/SkinnedMesh/*.meshml"),
		("model", "Samples/media/SSSSS/*.glb"),
		("albedo", "Samples/media/SSSSS/*.jpg"),
		("model", "Samples/media/SubSurface/*.glb"),
		("albedo", "Samples/media/SubSurface/*.jpg"),
		("albedo", "Samples/media/SubSurface/*.png"),
		("model", "Samples/media/VDMParticle/*.glb"),
		("albedo", "Samples/media/VDMParticle/*.jpg")
	)

	import glob
	for pd_item in pd_list:
		resource_path = dst_sample_dir + pd_item[1]
		subprocess.call([pd_path, "-P", platform, "-T", pd_item[0], "-I", resource_path], shell = True)
		for file in glob.glob(resource_path):
			print("Removing %s..." % file)
			os.remove(file)

	for file in glob.glob(dst_bin_dir + "KlayGE_DevHelper*" + dll_suffix):
		os.remove(file)
	os.remove(dst_bin_dir + "PlatformDeployer" + debug_suffix + exe_suffix)
	for file in glob.glob(dst_bin_dir + "ToolCommon*" + dll_suffix):
		os.remove(file)
	for file in glob.glob(dst_bin_dir + "assimp*" + dll_suffix):
		os.remove(file)
	for file in glob.glob(dst_bin_dir + "FreeImage*" + dll_suffix):
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
				PackageSamples(target_dir, build_info, compiler_info.arch, cfg)
	else:
		print("Usage: PackageSamples.py target_dir [project] [compiler] [arch] [config]")
