DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Camera.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/klayge_logo.ico" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Open.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Play.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Redo.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Save.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/SaveAs.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Skinning.png" "cbda47a1678ce70b6720856736100979d469e159")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Undo.png" "cbda47a1678ce70b6720856736100979d469e159")

INCLUDE_EXTERNAL_MSPROJECT(
	MtlEditor ${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MtlEditor.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

ADD_DEPENDENCIES(MtlEditor MtlEditorCoreWrapper)

SET_TARGET_PROPERTIES(MtlEditor PROPERTIES FOLDER "MtlEditor")
