DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Camera.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/klayge_logo.ico" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/LineMode.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Open.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Play.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Save.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Skinning.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MeshMLViewer/MeshMLViewer/Images/Smooth.png" "cbda47a1678ce70b6720856736100979d469e159")

INCLUDE_EXTERNAL_MSPROJECT(
	MeshMLViewer ${KLAYGE_PROJECT_DIR}/Tools/src/MeshMLViewer/MeshMLViewer/MeshMLViewer${KLAYGE_OUTPUT_SUFFIX}.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

ADD_DEPENDENCIES(MeshMLViewer MeshMLViewerCoreWrapper)

SET_TARGET_PROPERTIES(MeshMLViewer PROPERTIES FOLDER "MeshMLViewer")
