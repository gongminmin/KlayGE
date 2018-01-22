#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildBoost(build_info, compiler_info):
	BuildAProject("boost", "External/boost", build_info, compiler_info, build_info.is_windows_desktop)

def BuildPython(build_info, compiler_info):
	BuildAProject("Python", "External/Python", build_info, compiler_info)

def BuildLibogg(build_info, compiler_info):
	BuildAProject("libogg", "External/libogg", build_info, compiler_info)

def BuildLibvorbis(build_info, compiler_info):
	BuildAProject("libvorbis", "External/libvorbis", build_info, compiler_info)

def BuildFreetype(build_info, compiler_info):
	BuildAProject("freetype", "External/freetype", build_info, compiler_info)

def Build7z(build_info, compiler_info):
	BuildAProject("7z", "External/7z", build_info, compiler_info, build_info.is_windows)

def BuildUniversalDXSDK(build_info, compiler_info):
	BuildAProject("UniversalDXSDK", "External/UniversalDXSDK", build_info, compiler_info)

def BuildOpenALSDK(build_info, compiler_info):
	BuildAProject("OpenALSDK", "External/OpenALSDK", build_info, compiler_info)

def BuildRapidxml(build_info, compiler_info):
	BuildAProject("rapidxml", "External/rapidxml", build_info, compiler_info)

def BuildWpftoolkit(build_info, compiler_info):
	BuildAProject("wpftoolkit", "External/wpftoolkit", build_info, compiler_info)

def BuildAndroidNativeAppGlue(build_info, compiler_info):
	BuildAProject("android_native_app_glue", "External/android_native_app_glue", build_info, compiler_info)

def BuildAssimp(build_info, compiler_info):
	BuildAProject("assimp", "External/assimp", build_info, compiler_info, True)

def BuildNanosvg(build_info, compiler_info):
	BuildAProject("nanosvg", "External/nanosvg", build_info, compiler_info)

def BuildGtest(build_info, compiler_info):
	BuildAProject("googletest", "External/googletest", build_info, compiler_info)

def BuildFreeImage(build_info, compiler_info):
	BuildAProject("FreeImage", "External/FreeImage", build_info, compiler_info, build_info.is_windows)

def BuildExternalLibs(build_info):
	for compiler_info in build_info.compilers:
		BuildBoost(build_info, compiler_info)
		BuildPython(build_info, compiler_info)
		Build7z(build_info, compiler_info)
		BuildRapidxml(build_info, compiler_info)
		BuildAndroidNativeAppGlue(build_info, compiler_info)
		BuildLibogg(build_info, compiler_info)
		BuildLibvorbis(build_info, compiler_info)

		if build_info.is_dev_platform:
			BuildFreetype(build_info, compiler_info)
			BuildAssimp(build_info, compiler_info)
			BuildNanosvg(build_info, compiler_info)
			BuildGtest(build_info, compiler_info)
			BuildFreeImage(build_info, compiler_info)

			if (compiler_info.arch != "arm") and (compiler_info.arch != "arm64"):
				BuildUniversalDXSDK(build_info, compiler_info)

			if ("win" == build_info.target_platform) and (compiler_info.arch != "arm") and (compiler_info.arch != "arm64"):
				BuildOpenALSDK(build_info, compiler_info)

		if build_info.is_windows_desktop and ("x64" == compiler_info.arch) and ("vc" == build_info.compiler_name):
			BuildWpftoolkit(build_info, compiler_info)

if __name__ == "__main__":
	BuildExternalLibs(BuildInfo.FromArgv(sys.argv))
