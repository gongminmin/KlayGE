PROJECT(wpftoolkit)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(WPFTOOLKIT_MAJOR_VERSION 2)
SET(WPFTOOLKIT_MINOR_VERSION 8)
SET(WPFTOOLKIT_VERSION ${WPFTOOLKIT_MAJOR_VERSION}.${WPFTOOLKIT_MINOR_VERSION})

SET(WPFTOOLKIT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${WPFTOOLKIT_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/External/Build/CMake/ExternalCommon.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(REL_PATH "External/Downloads/wpftoolkit-${WPFTOOLKIT_VERSION}.patched.7z")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_HOST_PLATFORM_NAME}/7z")
SET(PACKAGE_COMMIT_ID "d42168878829ba587e385c0b3a6140d6bb34d64e")
SET(PACKAGE_FILE_ID "8f25248ff7d2a31f9f99eaddc5d8fc1072ee33d4")

DOWNLOAD_PACKAGE("wpftoolkit" ${REL_PATH} ${PACKAGE_COMMIT_ID} ${PACKAGE_FILE_ID} "Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/AssemblyVersionInfo.cs")

INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock ${WPFTOOLKIT_PROJECT_DIR}/Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock/Xceed.Wpf.AvalonDock.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Aero ${WPFTOOLKIT_PROJECT_DIR}/Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/Xceed.Wpf.AvalonDock.Themes.Aero.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Metro ${WPFTOOLKIT_PROJECT_DIR}/Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Metro/Xceed.Wpf.AvalonDock.Themes.Metro.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.VS2010 ${WPFTOOLKIT_PROJECT_DIR}/Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.VS2010/Xceed.Wpf.AvalonDock.Themes.VS2010.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.Toolkit ${WPFTOOLKIT_PROJECT_DIR}/Main/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.Toolkit/Xceed.Wpf.Toolkit.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Aero PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Metro PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.VS2010 PROPERTIES FOLDER "Themes")
