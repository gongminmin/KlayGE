PROJECT(wpftoolkit)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(WPFTOOLKIT_PROJECT_DIR "${CMAKE_HOME_DIRECTORY}/../..")
SET(KLAYGE_ROOT_DIR "${WPFTOOLKIT_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(CHANGESET_NUM 110921)
SET(REL_PATH "External/Downloads/wpftoolkit-${CHANGESET_NUM}.zip")
SET(DST_PATH "${KLAYGE_ROOT_DIR}/External/wpftoolkit/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/AssemblyVersionInfo.cs")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
IF(KLAYGE_PLATFORM_WINDOWS)
	SET(PATCH_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/patc")
ELSE()
	SET(PATCH_PATH "patch")
ENDIF()

IF(KLAYGE_PLATFORM_WINDOWS)
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.exe")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.dll")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/patc.exe")
ELSE()
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
	DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.so")
ENDIF()
DOWNLOAD_FILE_URL(${REL_PATH} "http://download-codeplex.sec.s-msft.com/Download/SourceControlFileDownload.ashx?ProjectName=wpftoolkit&changeSetId=${CHANGESET_NUM}")

IF(EXISTS ${DST_PATH})
	SET(WPFTOOLKIT_VERSION_STR 0)
	FILE(STRINGS ${DST_PATH} WPFTOOLKIT_VERSION_STR REGEX "  public const string BaseVersion = ")
	STRING(FIND "${WPFTOOLKIT_VERSION_STR}" "  public const string BaseVersion = \"2.3\";" POS)
	IF(NOT POS EQUAL 0)
		FILE(REMOVE ${DST_PATH})
	ENDIF()
ENDIF()

ADD_CUSTOM_COMMAND(OUTPUT ${DST_PATH}
	COMMAND ${SEVENZIP_PATH} x -y ${KLAYGE_ROOT_DIR}/${REL_PATH}
	COMMAND ${CMAKE_COMMAND} -E copy_directory Main .
	COMMAND ${CMAKE_COMMAND} -E remove_directory Main
	COMMAND ${CMAKE_COMMAND} -E remove_directory BuildProcessTemplates
	COMMAND ${PATCH_PATH} -p0 -i Patches/ForVS.patch
	WORKING_DIRECTORY ${KLAYGE_ROOT_DIR}/External/wpftoolkit)
ADD_CUSTOM_TARGET(wpftoolkit_download ALL DEPENDS ${DST_PATH})

INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock ${WPFTOOLKIT_PROJECT_DIR}/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock/Xceed.Wpf.AvalonDock.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Aero ${WPFTOOLKIT_PROJECT_DIR}/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/Xceed.Wpf.AvalonDock.Themes.Aero.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Metro ${WPFTOOLKIT_PROJECT_DIR}/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Metro/Xceed.Wpf.AvalonDock.Themes.Metro.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.VS2010 ${WPFTOOLKIT_PROJECT_DIR}/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.VS2010/Xceed.Wpf.AvalonDock.Themes.VS2010.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.Toolkit ${WPFTOOLKIT_PROJECT_DIR}/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.Toolkit/Xceed.Wpf.Toolkit.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
ADD_DEPENDENCIES(Xceed.Wpf.AvalonDock "wpftoolkit_download")
ADD_DEPENDENCIES(Xceed.Wpf.AvalonDock.Themes.Aero "wpftoolkit_download")
ADD_DEPENDENCIES(Xceed.Wpf.AvalonDock.Themes.Metro "wpftoolkit_download")
ADD_DEPENDENCIES(Xceed.Wpf.AvalonDock.Themes.VS2010 "wpftoolkit_download")
ADD_DEPENDENCIES(Xceed.Wpf.Toolkit "wpftoolkit_download")

SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Aero PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Metro PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.VS2010 PROPERTIES FOLDER "Themes")
