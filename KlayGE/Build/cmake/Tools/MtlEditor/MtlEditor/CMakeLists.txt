DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/AssignMaterial.png" "043cc004649304f9e07fdd052953f654b529c758" "37dc7283ae6bc273f9d07e4d7bb7fc92f24b3484")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Camera.png" "cbda47a1678ce70b6720856736100979d469e159" "db8779719cd08b70bcf0e3c1700e9dd8b112a609")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/CopyMaterial.png" "043cc004649304f9e07fdd052953f654b529c758" "65253fdee9aac94212308b2b2f8912452fee896c")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/DeleteMaterial.png" "043cc004649304f9e07fdd052953f654b529c758" "774f2591d3bfb67f48f4da740f5c9fff058961c8")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ExportMaterial.png" "dce7404a90a601e7d0dcdeab60b2a3e5258c2b30" "a00b3266059dc8e3736248149268a11e14e697b7")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ImportMaterial.png" "dce7404a90a601e7d0dcdeab60b2a3e5258c2b30" "9b27264872a432464d6131868c567b3d6d220eab")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/ImposterMode.png" "67446d28c443c84b44f0f53d653a9313102dd911" "9111e3a0e8fd4852654094feb1262ac45a08cf4a")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/klayge_logo.ico" "cbda47a1678ce70b6720856736100979d469e159" "b05ac9575a9a001af17a3729987ba8dd77e4c017")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Light.png" "1e41de326949a845341262431bf9ceca3488acef" "0affe205574a9fc269e254625e7c582915160188")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/LineMode.png" "bb0000e115d4c725f5da35ea74bb152f30f583a4" "d50971c40c7811045d1b6e726e8f4cfc9e034dae")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Open.png" "cbda47a1678ce70b6720856736100979d469e159" "2494b313c7b71b2b4aec75541faacc720acc0b16")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Play.png" "cbda47a1678ce70b6720856736100979d469e159" "d6315bff786f86662874c6782b03d7332b1f3086")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Redo.png" "cbda47a1678ce70b6720856736100979d469e159" "2f540d3ddd56d164be8ddc17eed589f3840212c3")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Save.png" "cbda47a1678ce70b6720856736100979d469e159" "1bba12c7f5c9b339e15f5d73507988f14bfdd7e2")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/SaveAs.png" "cbda47a1678ce70b6720856736100979d469e159" "43561560c76dad3bd62c80edd17d569ad39651d7")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Skinning.png" "cbda47a1678ce70b6720856736100979d469e159" "19dd2371bf2b11742dcc2fd45b2d06d0b0a6427e")
DOWNLOAD_FILE("KlayGE/Tools/src/MtlEditor/MtlEditor/Images/Undo.png" "cbda47a1678ce70b6720856736100979d469e159" "23c318d9bbe5c95b8b0fbbea662facd56a3a66dc")

STRING(REPLACE "/" "\\" WRAPPER_DIR ${CMAKE_BINARY_DIR})
CONFIGURE_FILE(
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MtlEditor.csproj.in
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MtlEditor${KLAYGE_OUTPUT_SUFFIX}.csproj
	@ONLY
)

INCLUDE_EXTERNAL_MSPROJECT(
	MtlEditor ${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditor/MtlEditor${KLAYGE_OUTPUT_SUFFIX}.csproj
	TYPE FAE04EC0-301F-11D3-BF4B-00C04F79EFBC)

ADD_DEPENDENCIES(MtlEditor MtlEditorCoreWrapper)

SET_TARGET_PROPERTIES(MtlEditor PROPERTIES FOLDER "Tools/MtlEditor")
