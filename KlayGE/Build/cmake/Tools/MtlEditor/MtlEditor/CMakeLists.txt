DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Camera.png" "cbda47a1678ce70b6720856736100979d469e159" "db8779719cd08b70bcf0e3c1700e9dd8b112a609")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/klayge_logo.ico" "cbda47a1678ce70b6720856736100979d469e159" "b05ac9575a9a001af17a3729987ba8dd77e4c017")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Open.png" "cbda47a1678ce70b6720856736100979d469e159" "2494b313c7b71b2b4aec75541faacc720acc0b16")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Play.png" "cbda47a1678ce70b6720856736100979d469e159" "d6315bff786f86662874c6782b03d7332b1f3086")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Redo.png" "cbda47a1678ce70b6720856736100979d469e159" "2f540d3ddd56d164be8ddc17eed589f3840212c3")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Save.png" "cbda47a1678ce70b6720856736100979d469e159" "1bba12c7f5c9b339e15f5d73507988f14bfdd7e2")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/SaveAs.png" "cbda47a1678ce70b6720856736100979d469e159" "43561560c76dad3bd62c80edd17d569ad39651d7")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Skinning.png" "cbda47a1678ce70b6720856736100979d469e159" "19dd2371bf2b11742dcc2fd45b2d06d0b0a6427e")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Undo.png" "cbda47a1678ce70b6720856736100979d469e159" "23c318d9bbe5c95b8b0fbbea662facd56a3a66dc")

INCLUDE_EXTERNAL_MSPROJECT(
	MtlEditor ${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MtlEditor${KLAYGE_OUTPUT_SUFFIX}.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

ADD_DEPENDENCIES(MtlEditor MtlEditorCoreWrapper)

SET_TARGET_PROPERTIES(MtlEditor PROPERTIES FOLDER "Tools/MtlEditor")
