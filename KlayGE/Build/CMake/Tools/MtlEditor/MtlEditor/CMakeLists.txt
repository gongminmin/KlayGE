DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/AssignMaterial.png" "37dc7283ae6bc273f9d07e4d7bb7fc92f24b3484")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Camera.png" "db8779719cd08b70bcf0e3c1700e9dd8b112a609")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/CopyMaterial.png" "65253fdee9aac94212308b2b2f8912452fee896c")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/DeleteMaterial.png" "d327acfee4bde5c1a7e186ffe025ee2e70207399")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ExportMaterial.png" "a00b3266059dc8e3736248149268a11e14e697b7")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ImportMaterial.png" "9b27264872a432464d6131868c567b3d6d220eab")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ImposterMode.png" "9111e3a0e8fd4852654094feb1262ac45a08cf4a")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Light.png" "0affe205574a9fc269e254625e7c582915160188")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/LineMode.png" "d50971c40c7811045d1b6e726e8f4cfc9e034dae")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Open.png" "2494b313c7b71b2b4aec75541faacc720acc0b16")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Play.png" "d6315bff786f86662874c6782b03d7332b1f3086")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Redo.png" "2f540d3ddd56d164be8ddc17eed589f3840212c3")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Save.png" "1bba12c7f5c9b339e15f5d73507988f14bfdd7e2")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/SaveAs.png" "43561560c76dad3bd62c80edd17d569ad39651d7")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Skinning.png" "19dd2371bf2b11742dcc2fd45b2d06d0b0a6427e")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Skeleton.png" "1163f1f6661a0d0a882d7b199d87baea47fa2130")
DOWNLOAD_DEPENDENCY("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Undo.png" "23c318d9bbe5c95b8b0fbbea662facd56a3a66dc")

INCLUDE(CSharpUtilities)

SET(EXE_NAME "MtlEditor")

ENABLE_LANGUAGE(CSharp)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/App.config
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/App.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/App.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/Commands.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MainWindow.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MainWindow.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MaterialEntity.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MaterialEntityViewModel.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MeshEntity.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MeshEntityViewModel.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/OpenTexUserControlEditor.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/OpenTexUserControlEditor.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/SliderUserControlEditor.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/SliderUserControlEditor.xaml.cs
)

SET(PROPERTY_LINKS
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings
)
FOREACH(link_var ${PROPERTY_LINKS})
	SET(property_file "${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/${link_var}")

	SET_SOURCE_FILES_PROPERTIES(${property_file} PROPERTIES
		VS_CSHARP_Link ${link_var}
	)

	SET(PROPERTY_FILES ${PROPERTY_FILES}
		${property_file}
	)
ENDFOREACH()

SET(IMAGE_LINKS
	Images/AssignMaterial.png
	Images/Camera.png
	Images/CopyMaterial.png
	Images/DeleteMaterial.png
	Images/ExportMaterial.png
	Images/ImportMaterial.png
	Images/ImposterMode.png
	Images/Light.png
	Images/LineMode.png
	Images/Open.png
	Images/Play.png
	Images/Redo.png
	Images/Save.png
	Images/SaveAs.png
	Images/Skeleton.png
	Images/Skinning.png
	Images/Undo.png
)
FOREACH(link_var ${IMAGE_LINKS})
	SET(image_file "${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/${link_var}")

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

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/App.xaml PROPERTIES
	VS_XAML_TYPE "ApplicationDefinition"
)
SET_SOURCE_FILES_PROPERTIES(${IMAGE_FILES} PROPERTIES
	VS_COPY_TO_OUT_DIR "PreserveNewest"
	VS_TOOL_OVERRIDE "Resource"
)

ADD_EXECUTABLE(${EXE_NAME} WIN32 ${SOURCE_FILES} ${PROPERTY_FILES} ${IMAGE_FILES})
ADD_DEPENDENCIES(${EXE_NAME} MtlEditorCoreWrapper Xceed.Wpf.Toolkit Xceed.Wpf.AvalonDock)

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR}
	PROJECT_LABEL ${EXE_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${EXE_NAME}
	VS_GLOBAL_RootNamespace ${EXE_NAME}
	VS_GLOBAL_ApplicationIcon "${KLAYGE_PROJECT_DIR}/klayge_logo.ico"
	VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
	VS_DOTNET_REFERENCES "Microsoft.CSharp;PresentationCore;PresentationFramework;System;System.Core;System.Data;System.Data.DataSetExtensions;System.Windows.Controls.Ribbon;System.Windows.Forms;System.Xaml;System.Xml;System.Xml.Linq;WindowsBase;WindowsFormsIntegration"
	VS_DOTNET_REFERENCE_Xceed.Wpf.AvalonDock "${KLAYGE_ROOT_DIR}/External/wpftoolkit/bin/${CMAKE_CFG_INTDIR}/Xceed.Wpf.AvalonDock.dll"
	VS_DOTNET_REFERENCE_Xceed.Wpf.Toolkit "${KLAYGE_ROOT_DIR}/External/wpftoolkit/bin/${CMAKE_CFG_INTDIR}/Xceed.Wpf.Toolkit.dll"
	FOLDER "KlayGE/Tools/MtlEditor"
)

TARGET_LINK_LIBRARIES(${EXE_NAME})
