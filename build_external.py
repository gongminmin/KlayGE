#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(build_info, compiler_arch):
	additional_options = ""
	if "vc" == build_info.compiler_name:
		if 100 == build_info.compiler_version:
			additional_options += " -DWITH_ATOMIC:BOOL=\"ON\" -DWITH_CHRONO:BOOL=\"ON\" -DWITH_DATE_TIME:BOOL=\"ON\" -DWITH_FILESYSTEM:BOOL=\"ON\" \
				-DWITH_PROGRAM_OPTIONS:BOOL=\"ON\" -DWITH_SYSTEM -DWITH_THREAD:BOOL=\"ON\""
		elif build_info.compiler_version >= 110:
			if -1 == compiler_arch[0].find("_app"):
				additional_options += " -DWITH_PROGRAM_OPTIONS:BOOL=\"ON\""
	else:
		additional_options += " -DWITH_CHRONO:BOOL=\"ON\" -DWITH_FILESYSTEM:BOOL=\"ON\" -DWITH_PROGRAM_OPTIONS:BOOL=\"ON\" -DWITH_REGEX:BOOL=\"ON\" \
			-DWITH_SYSTEM:BOOL=\"ON\" -DWITH_THREAD:BOOL=\"ON\""
	build_a_project("boost", "External/boost", build_info, compiler_arch, True, additional_options)

def build_Python(build_info, compiler_arch):
	additional_options = "-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_HK:BOOL=\"ON\" -D BUILTIN_CODECS_ISO2022:BOOL=\"ON\" \
		-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_JP:BOOL=\"ON\" -D BUILTIN_CODECS_KR:BOOL=\"ON\" -D BUILTIN_CODECS_TW:BOOL=\"ON\" \
		-D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" -D BUILTIN_ITERTOOLS:BOOL=\"ON\" \
		-D BUILTIN_LOCALE:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" -D BUILTIN_MULTIBYTECODEC:BOOL=\"ON\" \
		-D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\" -D BUILTIN_MSVCRT:BOOL=\"OFF\" \
		-D BUILTIN_WINREG:BOOL=\"OFF\" -D BUILTIN_SUBPROCESS:BOOL=\"OFF\" -D BUILTIN_MULTIPROCESSING:BOOL=\"OFF\" \
		-D BUILTIN_SELECT:BOOL=\"OFF\" -D BUILTIN_SOCKET:BOOL=\"OFF\" -D BUILTIN_MMAP:BOOL=\"OFF\" \
		-D ENABLE_AUDIOOP:BOOL=\"OFF\" -D ENABLE_OSSAUDIODEV:BOOL=\"OFF\" \
		-D USE_SYSTEM_Curses:BOOL=\"OFF\" -D USE_SYSTEM_EXPAT:BOOL=\"OFF\" -D USE_SYSTEM_DB:BOOL=\"OFF\" -D USE_SYSTEM_GDBM:BOOL=\"OFF\" \
		-D USE_SYSTEM_OpenSSL:BOOL=\"OFF\" -D USE_SYSTEM_READLINE:BOOL=\"OFF\" -D USE_SYSTEM_SQLITE3:BOOL=\"OFF\" -D USE_SYSTEM_TCL:BOOL=\"OFF\" \
		-D USE_SYSTEM_ZLIB:BOOL=\"OFF\""
	build_a_project("Python", "External/Python", build_info, compiler_arch, False, additional_options)

def build_libogg(build_info, compiler_arch):
	build_a_project("libogg", "External/libogg", build_info, compiler_arch)

def build_libvorbis(build_info, compiler_arch):
	build_a_project("libvorbis", "External/libvorbis", build_info, compiler_arch)

def build_freetype(build_info, compiler_arch):
	build_a_project("freetype", "External/freetype", build_info, compiler_arch)

def build_7z(build_info, compiler_arch):
	build_a_project("7z", "External/7z", build_info, compiler_arch)

def setup_DXSDK(build_info, compiler_arch):
	build_a_project("DXSDK", "External/DXSDK", build_info, compiler_arch)

def setup_OpenALSDK(build_info, compiler_arch):
	build_a_project("OpenALSDK", "External/OpenALSDK", build_info, compiler_arch)

def setup_Cg(build_info, compiler_arch):
	build_a_project("Cg", "External/Cg", build_info, compiler_arch)

def build_external_libs(build_info):
	import glob

	if "win" == build_info.host_platform:
		bat_suffix = "bat"
	elif "linux" == build_info.host_platform:
		bat_suffix = "sh"
	if "win" == build_info.target_platform:
		dll_suffix = "dll"
	elif "linux" == build_info.target_platform:
		dll_suffix = "so"

	for arch in build_info.arch_list:
		platform_dir = "%s_%s" % (build_info.target_platform, arch[0])
		dst_dir = "KlayGE/bin/%s/" % platform_dir

		if not arch[3]:
			print("\nBuilding boost...\n")
			build_Boost(build_info, arch)

		if not arch[3]:
			print("\nBuilding Python...\n")
			build_Python(build_info, arch)

			if not build_info.prefer_static:
				if not os.path.exists("%sLib" % dst_dir):
					os.mkdir("%sLib" % dst_dir)
				copy_to_dst("External/Python/Lib/_abcoll.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/_weakrefset.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/abc.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/codecs.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/copyreg.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/genericpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/io.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/macpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/ntpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/os.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/os2emxpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/posixpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/stat.py", "%sLib/" % dst_dir)
				if not os.path.exists("%sLib/encodings" % dst_dir):
					os.mkdir("%sLib/encodings" % dst_dir)
				for fname in glob.iglob("External/Python/Lib/encodings/*.py"):
					copy_to_dst(fname, "%sLib/encodings/" % dst_dir)

		if not arch[3]:
			print("\nBuilding libogg...\n")
			build_libogg(build_info, arch)

		if not arch[3]:
			print("\nBuilding libvorbis...\n")
			build_libvorbis(build_info, arch)

		if (not arch[3]) and (build_info.target_platform != "android"):
			print("\nBuilding freetype...\n")
			build_freetype(build_info, arch)

		print("\nBuilding 7z...\n")
		build_7z(build_info, arch)

		if not build_info.prefer_static:
			for fname in glob.iglob("External/7z/lib/%s/*.%s" % (platform_dir, dll_suffix)):
				copy_to_dst(fname, dst_dir)

		if (not arch[3]) and (build_info.target_platform != "android"):
			if "win" == build_info.target_platform:
				print("\nSeting up DXSDK...\n")
				setup_DXSDK(build_info, arch)

		if (not arch[3]) and (build_info.target_platform != "android"):
			if ("win" == build_info.target_platform) and (arch[0] != "arm"):
				print("\nSeting up OpenAL SDK...\n")
				setup_OpenALSDK(build_info, arch)

		if (not arch[3]) and (arch[0] != "arm") and (build_info.target_platform != "android"):
			print("\nSeting up Cg...\n")
			setup_Cg(build_info, arch)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
