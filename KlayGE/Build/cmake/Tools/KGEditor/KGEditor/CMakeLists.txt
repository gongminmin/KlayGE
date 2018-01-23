DOWNLOAD_FILE("KlayGE/Tools/media/KGEditor/default_bg_c.dds" "1716601561dda04e4b9f5bc5ac509685401d23a2" "0bdca4602a264aae0e6a7957478a181bd78c4032")
DOWNLOAD_FILE("KlayGE/Tools/media/KGEditor/default_bg_y.dds" "1716601561dda04e4b9f5bc5ac509685401d23a2" "5a2e630a4b456382c6105c3682083213e7d36e09")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Camera.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "b8e2d89c418c800b4adb61de5155903ef8260928")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Light.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "0affe205574a9fc269e254625e7c582915160188")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Mesh.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "cd73d4e6f877a2f7266d2995ba52f330644d98e6")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Open.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "2494b313c7b71b2b4aec75541faacc720acc0b16")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Redo.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "2f540d3ddd56d164be8ddc17eed589f3840212c3")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Remove.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "171e1c37ae8c15e34ee6b5b9e34c74c049d36f29")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Rotation.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "ba00057e3d7aa4e965e3d527a144a385596af129")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Save.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "1bba12c7f5c9b339e15f5d73507988f14bfdd7e2")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/SaveAs.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "43561560c76dad3bd62c80edd17d569ad39651d7")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Scaling.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "ebc2ec31ee1ecb56918f82c0e345e799a89b290a")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Selection.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "bb13e3d5169686b04e253ead38eb0a9606b56a44")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Translation.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "f576b607913cfad949dbc08f041419b828ec8a65")
DOWNLOAD_FILE("KlayGE/Tools/src/KGEditor/KGEditor/Images/Undo.png" "1716601561dda04e4b9f5bc5ac509685401d23a2" "23c318d9bbe5c95b8b0fbbea662facd56a3a66dc")

INCLUDE(CSharpUtilities)

SET(EXE_NAME "KGEditor")

ENABLE_LANGUAGE(CSharp)

SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/App.config
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/App.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/App.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/Commands.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/MainWindow.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/MainWindow.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/OpenTexUserControlEditor.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/OpenTexUserControlEditor.xaml.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/SceneEntity.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/SceneEntityViewModel.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/SceneLight.cs
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/SliderUserControlEditor.xaml
	${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/SliderUserControlEditor.xaml.cs
)

SET(PROPERTY_LINKS
	Properties/AssemblyInfo.cs
	Properties/Resources.Designer.cs
	Properties/Resources.resx
	Properties/Settings.Designer.cs
	Properties/Settings.settings
)
FOREACH(link_var ${PROPERTY_LINKS})
	SET(property_file "${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/${link_var}")

	SET_SOURCE_FILES_PROPERTIES(${property_file} PROPERTIES
		VS_CSHARP_Link ${link_var}
	)

	SET(PROPERTY_FILES ${PROPERTY_FILES}
		${property_file}
	)
ENDFOREACH()

SET(IMAGE_LINKS
	Images/Camera.png
	Images/Light.png
	Images/Mesh.png
	Images/Open.png
	Images/Redo.png
	Images/Remove.png
	Images/Rotation.png
	Images/Save.png
	Images/SaveAs.png
	Images/Scaling.png
	Images/Selection.png
	Images/Translation.png
	Images/Undo.png
)
FOREACH(link_var ${IMAGE_LINKS})
	SET(image_file "${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/${link_var}")

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

SET_SOURCE_FILES_PROPERTIES(${KLAYGE_PROJECT_DIR}/Tools/src/KGEditor/KGEditor/App.xaml PROPERTIES
	VS_XAML_TYPE "ApplicationDefinition"
)
SET_SOURCE_FILES_PROPERTIES(${IMAGE_FILES} PROPERTIES
	VS_COPY_TO_OUT_DIR "PreserveNewest"
	VS_TOOL_OVERRIDE "Resource"
)

ADD_EXECUTABLE(${EXE_NAME} WIN32 ${SOURCE_FILES} ${PROPERTY_FILES} ${IMAGE_FILES})
ADD_DEPENDENCIES(${EXE_NAME} KGEditorCoreWrapper)

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES
	PROJECT_LABEL ${EXE_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${EXE_NAME}
	RUNTIME_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR}
	VS_GLOBAL_RootNamespace ${EXE_NAME}
	VS_GLOBAL_ApplicationIcon "${KLAYGE_PROJECT_DIR}/klayge_logo.ico"
	VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5"
	VS_DOTNET_REFERENCES "Microsoft.CSharp;PresentationCore;PresentationFramework;System;System.Core;System.Data;System.Data.DataSetExtensions;System.Windows.Controls.Ribbon;System.Windows.Forms;System.Xaml;System.Xml;System.Xml.Linq;WindowsBase;WindowsFormsIntegration"
	VS_DOTNET_REFERENCE_Xceed.Wpf.AvalonDock "${KLAYGE_BIN_DIR}/Xceed.Wpf.AvalonDock.dll"
	VS_DOTNET_REFERENCE_Xceed.Wpf.Toolkit "${KLAYGE_BIN_DIR}/Xceed.Wpf.Toolkit.dll"
)

TARGET_LINK_LIBRARIES(${EXE_NAME} ${EXTRA_LINKED_LIBRARIES})

SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES FOLDER "Tools/KGEditor")
