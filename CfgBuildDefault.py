#!/usr/bin/env python
#-*- coding: ascii -*-

class CfgBuildDefault:
	def __init__(self):
		#################################################
		# !!!! DO NOT DELETE ANY FIELD OF THIS CLASS !!!!
		#################################################

		# The path of cmake executable. Could fill in the path, or "auto".
		self.cmake_path = "auto"

		# Project type.
		#   On Windows desktop, could be "vs2019", "vs2017", "vs2015", "make", "ninja", "auto".
		#   On Windows store, could be "vs2019", "vs2017", "vs2015", "auto".
		#   On Android, could be "make", "auto".
		#   On Linux, could be "make", "ninja", "auto".
		#   On macOS, could be "xcode", "ninja", "auto".
		#   On iOS, could be "xcode", "ninja", "auto".
		self.project = "auto"

		# Compiler name.
		#   On Windows desktop, could be "vc142", "vc141", "vc140", "mingw", "auto".
		#   On Windows store, could be "vc142", "vc141", "vc140", "auto".
		#   On Android, could be "clang", "auto".
		#   On Linux, could be "gcc", "auto".
		#   On macOS, could be "clang", "auto".
		#   On iOS, could be "clang", "auto".
		self.compiler = "auto"

		# Target CPU architecture.
		#   On Windows desktop, could be "arm64", "x64".
		#   On Windows store, could be "arm64", "arm", "x64".
		#   On Android, cound be "arm64-v8a", "x86", "x86_64".
		#   On Linux, could be "x64".
		#   On macOS, could be "x64".
		#   On iOS, could be "arm", "x86".
		self.arch = ("x64", )

		# Configuration. Could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo".
		self.config = ("Debug", "RelWithDebInfo")

		# Target platform for cross compiling.
		#   On Windows desktop, could be "auto".
		#   On Windows store, could be "win_store 10.0".
		#   On Android, cound be "android 5.1", "android 6.0", "android 7.0", "android 7.1".
		#   On Linux, could be "auto".
		#   On macOS, could be "auto".
		#   On iOS, could be "ios".
		self.target = "auto"

		# A name for offline FXML compiling. Could be one of "d3d_11_0", "gles_3_0", "gles_3_1", "gles_3_2", or "auto".
		self.shader_platform_name = "auto"

		# The path of GLES SDK's include. Could fill in the path, or "auto".
		self.gles_include_dir = "auto"

		# The path of LibOVR. Could fill in the path, or "auto".
		self.libovr_path = "auto"

def ActivedCfgBuild():
	return CfgBuildDefault()
