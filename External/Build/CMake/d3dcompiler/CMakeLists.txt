set(D3DCOMPILER_DIR "${EXTERNAL_PROJECT_DIR}/d3dcompiler")

set(D3DCOMPILER_NAME "d3dcompiler_47.dll")
if(KLAYGE_PLATFORM_WINDOWS)
	set(D3DCOMPILER_ARCH_NAME ${KLAYGE_ARCH_NAME})
else()
	set(D3DCOMPILER_ARCH_NAME "x86")
endif()
set(staging_path "${D3DCOMPILER_DIR}/Redist/${D3DCOMPILER_ARCH_NAME}/${D3DCOMPILER_NAME}")

if(NOT EXISTS "${staging_path}")
	set(ProgramFilesX86 "ProgramFiles(x86)")
	if(KLAYGE_PLATFORM_WINDOWS)
		find_path(win_sdk_dir Redist/D3D
			PATHS
				"$ENV{${WindowsSdkDir}}"
				"$ENV{${ProgramFilesX86}}/Windows Kits/10"
				"$ENV{${ProgramFilesX86}}/Windows Kits/8.1"
			)

		if(win_sdk_dir)
			message(STATUS "Copying d3dcompiler from ${win_sdk_dir}/Redist/ ...")
	
			foreach(arch
				"x86" "x64")
				file(COPY "${win_sdk_dir}/Redist/D3D/${arch}/${D3DCOMPILER_NAME}" DESTINATION "${D3DCOMPILER_DIR}/Redist/${arch}/")
			endforeach()
		endif()
	endif()

	if(NOT win_sdk_dir)
		message(STATUS "Downloading ${D3DCOMPILER_NAME} ...")

		foreach(arch
				"x86" "x64")
			set(url "https://raw.githubusercontent.com/hrydgard/minidx9/7751cf73f5c06f1be21f5f31c3e2d9a7bacd3a93/10/Redist/D3D/${arch}/${D3DCOMPILER_NAME}")
			set(dest "${D3DCOMPILER_DIR}/Redist/${arch}/${D3DCOMPILER_NAME}")
			file(DOWNLOAD ${url} ${dest} STATUS ERR)
			list(GET ERR 0 ERR_CODE)
			if(ERR_CODE)
				file(REMOVE ${dest})
				list(GET ERR 1 ERR_MSG)
				message(FATAL_ERROR "Failed to download file ${URL}: ${ERR_MSG}")
			endif()
		endforeach()
	endif()
endif()

set(REDIST_PATH "${KLAYGE_BIN_DIR}/${D3DCOMPILER_NAME}")
add_custom_command(OUTPUT ${REDIST_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${staging_path} ${REDIST_PATH})
add_custom_target(D3DCompilerRedist ALL DEPENDS ${REDIST_PATH})

set_target_properties(D3DCompilerRedist PROPERTIES FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/D3DCompiler")
