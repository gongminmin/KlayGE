FIND_PACKAGE(PythonInterp)

SET(TableGenOutput "${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.hpp" "${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/Tables.cpp")
ADD_CUSTOM_COMMAND(OUTPUT ${TableGenOutput}
	COMMAND "${PYTHON_EXECUTABLE}" "TableGen.py"
	WORKING_DIRECTORY "${KLAYGE_PROJECT_DIR}/Core/Src/Base/TableGen/"
	COMMENT "Generating tables...")
ADD_CUSTOM_TARGET(TableGen DEPENDS ${TableGenOutput})

SET_TARGET_PROPERTIES(TableGen PROPERTIES FOLDER "KlayGE/Engine/Core")
