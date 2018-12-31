SET(WPFTOOLKIT_MAJOR_VERSION 2)
SET(WPFTOOLKIT_MINOR_VERSION 8)
SET(WPFTOOLKIT_VERSION ${WPFTOOLKIT_MAJOR_VERSION}.${WPFTOOLKIT_MINOR_VERSION})

SET(WPFTOOLKIT_PROJECT_DIR "${EXTERNAL_PROJECT_DIR}/wpftoolkit")

SET(REL_PATH "External/Downloads/wpftoolkit-${WPFTOOLKIT_VERSION}.patched.7z")
SET(PACKAGE_FILE_ID "8f25248ff7d2a31f9f99eaddc5d8fc1072ee33d4")

DOWNLOAD_PACKAGE("wpftoolkit" ${REL_PATH} ${PACKAGE_FILE_ID})

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

add_dependencies(Xceed.Wpf.Toolkit Xceed.Wpf.AvalonDock
	Xceed.Wpf.AvalonDock.Themes.Aero Xceed.Wpf.AvalonDock.Themes.Metro Xceed.Wpf.AvalonDock.Themes.VS2010)

foreach(target
	"Xceed.Wpf.AvalonDock" "Xceed.Wpf.Toolkit")
	set_target_properties(${target} PROPERTIES FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/wpftoolkit")
endforeach()
foreach(target
	"Xceed.Wpf.AvalonDock.Themes.Aero" "Xceed.Wpf.AvalonDock.Themes.Metro" "Xceed.Wpf.AvalonDock.Themes.VS2010")
	set_target_properties(${target} PROPERTIES FOLDER "${EXTERNAL_VS_FOLDER_PREFIX}/wpftoolkit/Themes")
endforeach()
