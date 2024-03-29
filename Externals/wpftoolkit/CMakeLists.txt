SET(WPFTOOLKIT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wpftoolkit")

UpdateExternalLib("wpftoolkit" "https://github.com/xceedsoftware/wpftoolkit" "95104c6105c793f23e85274c24b37dcb744f94db" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Adjust-the-path-of-output-directory-and-sn.snk" "0002-Change-the-dotnet-frame-version")
		ApplyPatch("wpftoolkit" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock ${WPFTOOLKIT_PROJECT_DIR}/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock/Xceed.Wpf.AvalonDock.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Aero ${WPFTOOLKIT_PROJECT_DIR}/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Aero/Xceed.Wpf.AvalonDock.Themes.Aero.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.Metro ${WPFTOOLKIT_PROJECT_DIR}/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.Metro/Xceed.Wpf.AvalonDock.Themes.Metro.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.AvalonDock.Themes.VS2010 ${WPFTOOLKIT_PROJECT_DIR}/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.AvalonDock.Themes.VS2010/Xceed.Wpf.AvalonDock.Themes.VS2010.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)
INCLUDE_EXTERNAL_MSPROJECT(
	Xceed.Wpf.Toolkit ${WPFTOOLKIT_PROJECT_DIR}/ExtendedWPFToolkitSolution/Src/Xceed.Wpf.Toolkit/Xceed.Wpf.Toolkit.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

add_dependencies(Xceed.Wpf.Toolkit Xceed.Wpf.AvalonDock
	Xceed.Wpf.AvalonDock.Themes.Aero Xceed.Wpf.AvalonDock.Themes.Metro Xceed.Wpf.AvalonDock.Themes.VS2010)

set_target_properties("Xceed.Wpf.AvalonDock" "Xceed.Wpf.Toolkit"
	PROPERTIES FOLDER "Externals/wpftoolkit"
)
set_target_properties("Xceed.Wpf.AvalonDock.Themes.Aero" "Xceed.Wpf.AvalonDock.Themes.Metro" "Xceed.Wpf.AvalonDock.Themes.VS2010"
	PROPERTIES FOLDER "Externals/wpftoolkit/Themes"
)
