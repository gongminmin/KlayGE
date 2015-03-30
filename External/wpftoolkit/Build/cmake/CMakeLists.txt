PROJECT(wpftoolkit)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

IF(COMMAND cmake_policy)
	CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(WPFTOOLKIT_MAJOR_VERSION 2)
SET(WPFTOOLKIT_MINOR_VERSION 3)
SET(WPFTOOLKIT_VERSION ${WPFTOOLKIT_MAJOR_VERSION}.${WPFTOOLKIT_MINOR_VERSION})
SET(CHANGESET_NUM 110921)

FIND_PACKAGE(PythonInterp)

SET(WPFTOOLKIT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
SET(KLAYGE_ROOT_DIR "${WPFTOOLKIT_PROJECT_DIR}/../..")

INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Common.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Platform.cmake)
INCLUDE(${KLAYGE_ROOT_DIR}/cmake/Compiler.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(REL_PATH "External/Downloads/wpftoolkit-${CHANGESET_NUM}.zip")
SET(DST_PATH "${KLAYGE_ROOT_DIR}/External/wpftoolkit/Source/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/AssemblyVersionInfo.cs")
SET(SEVENZIP_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
SET(PATCH_PATH "${KLAYGE_ROOT_DIR}/External/Downloads/patch.py")

SET(REGEN FALSE)
IF(EXISTS ${DST_PATH})
	SET(WPFTOOLKIT_VERSION_STR 0)
	FILE(STRINGS ${DST_PATH} WPFTOOLKIT_VERSION_STR REGEX "  public const string BaseVersion = ")
	STRING(FIND "${WPFTOOLKIT_VERSION_STR}" "${WPFTOOLKIT_VERSION}" POS)
	IF(NOT POS EQUAL 37)
		SET(REGEN TRUE)
	ENDIF()
ELSE()
	SET(REGEN TRUE)
ENDIF()

IF(REGEN)
	IF(KLAYGE_PLATFORM_WINDOWS)
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.exe")
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.dll")
	ELSE()
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z")
		DOWNLOAD_FILE("External/Downloads/${KLAYGE_PLATFORM_NAME}/7z.so")
		FILE(COPY ${SEVENZIP_PATH} DESTINATION "${KLAYGE_ROOT_DIR}/External/Downloads")
		FILE(COPY "${KLAYGE_ROOT_DIR}/External/Downloads/7z" DESTINATION "${KLAYGE_ROOT_DIR}/External/Downloads/${KLAYGE_PLATFORM_NAME}/" FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
	ENDIF()
	DOWNLOAD_FILE("External/Downloads/patch.py")
	#DOWNLOAD_FILE_URL(${REL_PATH} "http://download-codeplex.sec.s-msft.com/Download/SourceControlFileDownload.ashx?ProjectName=wpftoolkit&changeSetId=${CHANGESET_NUM}")
	DOWNLOAD_FILE("External/Downloads/wpftoolkit-${CHANGESET_NUM}.zip")
	DOWNLOAD_FILE("External/wpftoolkit/Patches/ForVS.patch")

	EXECUTE_PROCESS(COMMAND "${SEVENZIP_PATH}" "x" "-y" "${KLAYGE_ROOT_DIR}/${REL_PATH}" WORKING_DIRECTORY "${KLAYGE_ROOT_DIR}/External/wpftoolkit")
	FILE(COPY "${KLAYGE_ROOT_DIR}/External/wpftoolkit/Main/" DESTINATION "${KLAYGE_ROOT_DIR}/External/wpftoolkit/")
	FILE(REMOVE_RECURSE "${KLAYGE_ROOT_DIR}/External/wpftoolkit/Main")
	FILE(REMOVE_RECURSE "${KLAYGE_ROOT_DIR}/External/wpftoolkit/BuildProcessTemplates")
	FILE(REMOVE_RECURSE "${KLAYGE_ROOT_DIR}/External/wpftoolkit/Source/ExtendedWPFToolkitSolution_35")
	EXECUTE_PROCESS(COMMAND "${PYTHON_EXECUTABLE}" "${PATCH_PATH}" "-p0" "Patches/ForVS.patch" WORKING_DIRECTORY "${KLAYGE_ROOT_DIR}/External/wpftoolkit")
ENDIF()

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

SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Aero PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.Metro PROPERTIES FOLDER "Themes")
SET_TARGET_PROPERTIES(Xceed.Wpf.AvalonDock.Themes.VS2010 PROPERTIES FOLDER "Themes")
