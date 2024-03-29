DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/A.png" "acdbd5252f3ecd430d8bdc89f1d00f0296f87f52")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/B.png" "4a63684a7e5aa4d9f2def99fa94c9b74f0d8cbdd")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/G.png" "188b04cf3753f40c2184c017c3655c8df0ef0ca5")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/Open.png" "2494b313c7b71b2b4aec75541faacc720acc0b16")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/R.png" "98828e5fcbd53eab1a4a358456b008e7d9ba8801")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/Save.png" "1bba12c7f5c9b339e15f5d73507988f14bfdd7e2")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/TexViewer/TexViewer/Images/SaveAs.png" "43561560c76dad3bd62c80edd17d569ad39651d7")

INCLUDE(CSharpUtilities)

ENABLE_LANGUAGE(CSharp)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/App.config
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/App.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/App.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/MainWindow.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/MainWindow.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/SliderUserControlEditor.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/SliderUserControlEditor.xaml.cs
)

SET(PROPERTY_LINKS
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings
)
FOREACH(link_var ${PROPERTY_LINKS})
	SET(property_file "${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/${link_var}")

	SET_SOURCE_FILES_PROPERTIES(${property_file} PROPERTIES
		VS_CSHARP_Link ${link_var}
	)

	SET(PROPERTY_FILES ${PROPERTY_FILES}
		${property_file}
	)
ENDFOREACH()

SET(IMAGE_LINKS
	Images/A.png
	Images/B.png
	Images/G.png
	Images/Open.png
	Images/R.png
	Images/Save.png
	Images/SaveAs.png
)
FOREACH(link_var ${IMAGE_LINKS})
	SET(image_file "${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/${link_var}")

	SET_SOURCE_FILES_PROPERTIES(${image_file} PROPERTIES
		VS_CSHARP_Link ${link_var}
	)

	SET(IMAGE_FILES ${IMAGE_FILES}
		${image_file}
	)
ENDFOREACH()
SET(IMAGE_FILES ${IMAGE_FILES}
	"${KLAYGE_PROJECT_DIR}/klayge_logo.ico"	
)
SET_SOURCE_FILES_PROPERTIES("${KLAYGE_PROJECT_DIR}/klayge_logo.ico" PROPERTIES
	VS_CSHARP_Link "Images/klayge_logo.ico"
)

CSHARP_SET_DESIGNER_CS_PROPERTIES(${PROPERTY_FILES})
CSHARP_SET_XAML_CS_PROPERTIES(${SOURCE_FILES})

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Tools/src/TexViewer/TexViewer/App.xaml PROPERTIES
	VS_XAML_TYPE "ApplicationDefinition"
)
SET_SOURCE_FILES_PROPERTIES(${IMAGE_FILES} PROPERTIES
	VS_COPY_TO_OUT_DIR "PreserveNewest"
	VS_TOOL_OVERRIDE "Resource"
)

ADD_EXECUTABLE(TexViewer WIN32 ${SOURCE_FILES} ${PROPERTY_FILES} ${IMAGE_FILES})
ADD_DEPENDENCIES(TexViewer Xceed.Wpf.Toolkit Xceed.Wpf.AvalonDock)

SET_TARGET_PROPERTIES(TexViewer PROPERTIES
	OUTPUT_NAME TexViewer
	VS_GLOBAL_RootNamespace TexViewer
	VS_GLOBAL_ApplicationIcon "${KLAYGE_PROJECT_DIR}/klayge_logo.ico"
	VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8"
	VS_DOTNET_REFERENCES "Microsoft.CSharp;PresentationCore;PresentationFramework;System;System.Core;System.Data;System.Data.DataSetExtensions;System.Windows.Controls.Ribbon;System.Windows.Forms;System.Xaml;System.Xml;System.Xml.Linq;WindowsBase;WindowsFormsIntegration"
	VS_DOTNET_REFERENCE_Xceed.Wpf.AvalonDock "${KLAYGE_ROOT_DIR}/External/wpftoolkit/bin/${CMAKE_CFG_INTDIR}/Xceed.Wpf.AvalonDock.dll"
	VS_DOTNET_REFERENCE_Xceed.Wpf.Toolkit "${KLAYGE_ROOT_DIR}/External/wpftoolkit/bin/${CMAKE_CFG_INTDIR}/Xceed.Wpf.Toolkit.dll"
	FOLDER "KlayGE/Tools/TexViewer"
)

TARGET_LINK_LIBRARIES(TexViewer
	PRIVATE
		TexViewerCoreWrapper
)
