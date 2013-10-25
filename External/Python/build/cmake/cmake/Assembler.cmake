# CMake ignores .S files, so we have to add compile commands for them manually
macro(add_assembler ADD_ASSEMBLER_OUTPUTVAR ADD_ASSEMBLER_FILE)
    get_filename_component(ADD_ASSEMBLER_PATH ${ADD_ASSEMBLER_FILE} PATH)
    get_filename_component(ADD_ASSEMBLER_NAME_WE ${ADD_ASSEMBLER_FILE} NAME_WE)

    # We're going to create an .o file in the binary directory
    set(ADD_ASSEMBLER_OUTPUT "${CMAKE_BINARY_DIR}/CMakeBuild/${ADD_ASSEMBLER_PATH}/${ADD_ASSEMBLER_NAME_WE}.o")

    # Make sure the parent directory exists
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/CMakeBuild/${ADD_ASSEMBLER_PATH}/")

    # Build up a list of cflags to pass to gcc.
    # Start with the normal cmake ones.
    set(ADD_ASSEMBLER_CFLAGS ${CMAKE_C_FLAGS})
    separate_arguments(ADD_ASSEMBLER_CFLAGS)

    # Add -I flags for include directories that are set on this subdirectory.
    # Also take any additional directories passed to this function.
    get_directory_property(ADD_ASSEMBLER_INCLUDE_DIRS INCLUDE_DIRECTORIES)
    foreach(ADD_ASSEMBLER_INCLUDE_DIR ${ADD_ASSEMBLER_INCLUDE_DIRS} ${ARGN})
        list(APPEND ADD_ASSEMBLER_CFLAGS "-I${ADD_ASSEMBLER_INCLUDE_DIR}")
    endforeach(ADD_ASSEMBLER_INCLUDE_DIR)

    # Add the command to compile the assembler.
    add_custom_command(
        OUTPUT ${ADD_ASSEMBLER_OUTPUT}
        COMMAND ${CMAKE_C_COMPILER} ${ADD_ASSEMBLER_CFLAGS}
                -c ${SRC_DIR}/${ADD_ASSEMBLER_FILE}
                -o ${ADD_ASSEMBLER_OUTPUT}
        DEPENDS ${SRC_DIR}/${ADD_ASSEMBLER_FILE}
    )

    # Link this .o in the target.
    list(APPEND ${ADD_ASSEMBLER_OUTPUTVAR} ${ADD_ASSEMBLER_OUTPUT})
endmacro(add_assembler)
