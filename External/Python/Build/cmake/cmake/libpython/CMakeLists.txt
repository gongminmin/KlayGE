add_definitions(-DPy_BUILD_CORE)
add_definitions(-DNDEBUG)
if(WINDOWS_STORE OR WINDOWS_PHONE)
  add_definitions(-D_WIN32_WINNT=0x0602)
  add_definitions(-DWINVER=0x0602)
  add_definitions(-DNTDDI_VERSION=0x06020000)
endif()

set(MODULE_SOURCES # Equivalent to MODULE_OBJS in Makefile.pre
    ${CMAKE_BINARY_DIR}/CMakeFiles/config.c
    ${SRC_DIR}/Modules/gcmodule.c
    ${SRC_DIR}/Modules/main.c
)
if(UNIX)
    list(APPEND MODULE_SOURCES
        ${SRC_DIR}/Modules/getpath.c
    )
    set(PYTHONPATH "${EXTRA_PYTHONPATH}:lib-dynload:plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}:lib-tk")
    endif()
    set_property(
        SOURCE ${SRC_DIR}/Modules/getpath.c
        PROPERTY COMPILE_DEFINITIONS
            PREFIX="${CMAKE_INSTALL_PREFIX}"
            EXEC_PREFIX="${CMAKE_INSTALL_PREFIX}"
            VERSION="${PY_VERSION_MAJOR}.${PY_VERSION_MINOR}"
            VPATH="."
            PYTHONPATH="${PYTHONPATH}"
      )
elseif(WIN32)
    list(APPEND MODULE_SOURCES
        ${SRC_DIR}/PC/getpathp.c
    )
    # HACK To workaround limitation in escaping logic of CMake, the pythonpath
    #      separator is conditionally set depending of the version of Visual Studio.
    #      See http://cmake.org/Bug/view.php?id=14073
    if( ("${MSVC_VERSION}" VERSION_GREATER "1599") AND ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio") )
        set(PATHSEP "%3B")
    elseif( (${MSVC}) AND ("${CMAKE_GENERATOR}" MATCHES "^Ninja") )
        set(PATHSEP "\;")
    elseif( (${MSVC}) AND ("${CMAKE_GENERATOR}" MATCHES "^NMake") )
        set(PATHSEP "\;")
    elseif(MINGW)
        set(PATHSEP "\;")
    else()
        set(PATHSEP ";")
    endif()
    string(REPLACE "/" "\\\\" PYTHONHOME_ESCAPED ${PYTHONHOME})
    string(REPLACE "/" "\\\\" EXTENSION_INSTALL_DIR_ESCAPED ${EXTENSION_INSTALL_DIR})
    set(PYTHONPATH "${EXTRA_PYTHONPATH}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}\\\\${CMAKE_CFG_INTDIR}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\lib-tk")
    endif()

    set(_wide_char_modifier)
    if(IS_PY3)
        set(_wide_char_modifier "L")
    endif()

    set_property(
        SOURCE ${SRC_DIR}/PC/getpathp.c
        PROPERTY COMPILE_DEFINITIONS
            "LANDMARK=${_wide_char_modifier}\"${PYTHONHOME_ESCAPED}\\\\os.py\""
            "PYTHONPATH=${_wide_char_modifier}\"${PYTHONPATH}\""
    )
endif()

set(PARSER_COMMON_SOURCES # Equivalent to POBJS in Makefile.pre
    ${SRC_DIR}/Parser/acceler.c
    ${SRC_DIR}/Parser/bitset.c
    ${SRC_DIR}/Parser/firstsets.c
    ${SRC_DIR}/Parser/grammar1.c
    ${SRC_DIR}/Parser/grammar.c
    ${SRC_DIR}/Parser/listnode.c
    ${SRC_DIR}/Parser/metagrammar.c
    ${SRC_DIR}/Parser/node.c
    ${SRC_DIR}/Parser/parser.c
    ${SRC_DIR}/Parser/pgen.c
)

set(OBJECT2_SOURCES
    ${SRC_DIR}/Objects/bufferobject.c
    ${SRC_DIR}/Objects/cobject.c
    ${SRC_DIR}/Objects/intobject.c
    ${SRC_DIR}/Objects/stringobject.c
)

set(OBJECT3_SOURCES
    ${SRC_DIR}/Objects/accu.c
    ${SRC_DIR}/Objects/bytesobject.c
    ${SRC_DIR}/Objects/namespaceobject.c
    ${SRC_DIR}/Objects/odictobject.c
)
if(MSVC)
    list(APPEND OBJECT3_SOURCES
        ${SRC_DIR}/PC/invalid_parameter_handler.c
    )
endif()

set(OBJECT_COMMON_SOURCES # Equivalent to OBJECT_OBJS in Makefile.pre
    ${OBJECT${PY_VERSION_MAJOR}_SOURCES}
    ${SRC_DIR}/Objects/abstract.c
    ${SRC_DIR}/Objects/boolobject.c
    ${SRC_DIR}/Objects/bytearrayobject.c
    ${SRC_DIR}/Objects/bytes_methods.c
    ${SRC_DIR}/Objects/capsule.c
    ${SRC_DIR}/Objects/cellobject.c
    ${SRC_DIR}/Objects/classobject.c
    ${SRC_DIR}/Objects/codeobject.c
    ${SRC_DIR}/Objects/complexobject.c
    ${SRC_DIR}/Objects/descrobject.c
    ${SRC_DIR}/Objects/dictobject.c
    ${SRC_DIR}/Objects/enumobject.c
    ${SRC_DIR}/Objects/exceptions.c
    ${SRC_DIR}/Objects/fileobject.c
    ${SRC_DIR}/Objects/floatobject.c
    ${SRC_DIR}/Objects/frameobject.c
    ${SRC_DIR}/Objects/funcobject.c
    ${SRC_DIR}/Objects/genobject.c
    ${SRC_DIR}/Objects/iterobject.c
    ${SRC_DIR}/Objects/listobject.c
    ${SRC_DIR}/Objects/longobject.c
    ${SRC_DIR}/Objects/memoryobject.c
    ${SRC_DIR}/Objects/methodobject.c
    ${SRC_DIR}/Objects/moduleobject.c
    ${SRC_DIR}/Objects/object.c
    ${SRC_DIR}/Objects/obmalloc.c
    ${SRC_DIR}/Objects/rangeobject.c
    ${SRC_DIR}/Objects/setobject.c
    ${SRC_DIR}/Objects/sliceobject.c
    ${SRC_DIR}/Objects/structseq.c
    ${SRC_DIR}/Objects/tupleobject.c
    ${SRC_DIR}/Objects/typeobject.c
    ${SRC_DIR}/Objects/unicodectype.c
    ${SRC_DIR}/Objects/unicodeobject.c
    ${SRC_DIR}/Objects/weakrefobject.c
)

if(CMAKE_C_COMPILER_ID MATCHES GNU)
    foreach(filename class complex float int method string type unicode weakref)
        set_property(SOURCE ${SRC_DIR}/Objects/${filename}object.c PROPERTY COMPILE_FLAGS -Wno-unused-value)
    endforeach()
endif()

set(DYNLOAD_SOURCES
  )

if(UNIX AND HAVE_DLOPEN)
    list(APPEND DYNLOAD_SOURCES
        ${SRC_DIR}/Python/dynload_shlib.c
    )
    if(IS_PY3)
    set_property(
        SOURCE ${SRC_DIR}/Python/dynload_shlib.c
        PROPERTY COMPILE_DEFINITIONS
            SOABI="${SOABI}"
        )
    endif()
elseif(WIN32)
    list(APPEND DYNLOAD_SOURCES
        ${SRC_DIR}/PC/dl_nt.c
        ${SRC_DIR}/Python/dynload_win.c
        )
    set_property(
        SOURCE ${SRC_DIR}/PC/dl_nt.c
        PROPERTY COMPILE_DEFINITIONS Py_ENABLE_SHARED
    )
endif()

set(THREAD_SOURCES )
if(WITH_THREAD)
    list(APPEND THREAD_SOURCES
        ${SRC_DIR}/Python/thread.c
    )
endif()

set(PYTHON2_COMMON_SOURCES
    ${SRC_DIR}/Python/formatter_string.c
    ${SRC_DIR}/Python/pystrtod.c
)
if(WIN32)
    list(APPEND PYTHON2_COMMON_SOURCES
        ${SRC_DIR}/PC/import_nt.c
    )
endif()

set(PYTHON3_COMMON_SOURCES
    ${SRC_DIR}/Python/dynamic_annotations.c
    ${SRC_DIR}/Python/fileutils.c
    ${SRC_DIR}/Python/pyhash.c
    ${SRC_DIR}/Python/pylifecycle.c
    ${SRC_DIR}/Python/pystrhex.c
    ${SRC_DIR}/Python/pystrtod.c
    ${SRC_DIR}/Python/pytime.c
)

set(PYTHON_COMMON_SOURCES
    ${DYNLOAD_SOURCES}
    ${PYTHON${PY_VERSION_MAJOR}_COMMON_SOURCES}
    ${THREAD_SOURCES}
    ${SRC_DIR}/Python/asdl.c
    ${SRC_DIR}/Python/ast.c
    ${SRC_DIR}/Python/bltinmodule.c
    ${SRC_DIR}/Python/ceval.c
    ${SRC_DIR}/Python/codecs.c
    ${SRC_DIR}/Python/compile.c
    ${SRC_DIR}/Python/dtoa.c
    ${SRC_DIR}/Python/errors.c
    ${SRC_DIR}/Python/formatter_unicode.c
    ${SRC_DIR}/Python/future.c
    ${SRC_DIR}/Python/getargs.c
    ${SRC_DIR}/Python/getcompiler.c
    ${SRC_DIR}/Python/getcopyright.c
    ${SRC_DIR}/Python/getopt.c
    ${SRC_DIR}/Python/getplatform.c
    ${SRC_DIR}/Python/getversion.c
    ${SRC_DIR}/Python/graminit.c
    ${SRC_DIR}/Python/import.c
    ${SRC_DIR}/Python/importdl.c
    ${SRC_DIR}/Python/marshal.c
    ${SRC_DIR}/Python/modsupport.c
    ${SRC_DIR}/Python/mysnprintf.c
    ${SRC_DIR}/Python/mystrtoul.c
    ${SRC_DIR}/Python/peephole.c
    ${SRC_DIR}/Python/pyarena.c
    ${SRC_DIR}/Python/pyctype.c
    ${SRC_DIR}/Python/pyfpe.c
    ${SRC_DIR}/Python/pymath.c
    ${SRC_DIR}/Python/pystate.c
    ${SRC_DIR}/Python/pystrcmp.c
    ${SRC_DIR}/Python/Python-ast.c
    ${SRC_DIR}/Python/pythonrun.c
    ${SRC_DIR}/Python/random.c
    ${SRC_DIR}/Python/structmember.c
    ${SRC_DIR}/Python/symtable.c
    ${SRC_DIR}/Python/sysmodule.c
    ${SRC_DIR}/Python/traceback.c
    ${SRC_DIR}/Python/_warnings.c
)
if(UNIX)
    list(APPEND PYTHON_COMMON_SOURCES
        ${SRC_DIR}/Python/frozenmain.c
    )
else()
    list(APPEND PYTHON_COMMON_SOURCES
        ${SRC_DIR}/Python/frozen.c
    )
endif()

if(UNIX OR MINGW)
    set_property(
        SOURCE ${SRC_DIR}/Python/getplatform.c
        PROPERTY COMPILE_DEFINITIONS
            PLATFORM="${PY_PLATFORM}"
    )
    if(IS_PY3)
    set_property(
        SOURCE ${SRC_DIR}/Python/sysmodule.c
        PROPERTY COMPILE_DEFINITIONS
            ABIFLAGS="${ABIFLAGS}"
        )
    endif()
endif()

list(APPEND MODULE_SOURCES
    ${SRC_DIR}/Modules/_codecsmodule.c
    ${SRC_DIR}/Modules/errnomodule.c
    ${SRC_DIR}/Modules/signalmodule.c
    ${SRC_DIR}/Modules/_sre.c
    ${SRC_DIR}/Modules/symtablemodule.c
    ${SRC_DIR}/Modules/_weakref.c
    ${SRC_DIR}/Modules/xxsubtype.c
    ${SRC_DIR}/Modules/zipimport.c
)

set(LIBPYTHON_OMIT_FROZEN_SOURCES
    ${SRC_DIR}/Modules/getbuildinfo.c
    ${SRC_DIR}/Parser/myreadline.c
    ${SRC_DIR}/Parser/parsetok.c
    ${SRC_DIR}/Parser/tokenizer.c
    ${MODULE_SOURCES}
    ${OBJECT_COMMON_SOURCES}
    ${PARSER_COMMON_SOURCES}
    ${PYTHON_COMMON_SOURCES}
)

# List of builtin extensions
get_property(builtin_extensions GLOBAL PROPERTY builtin_extensions)

# Collect builtin extension sources
set(builtin_extension_sources)
foreach(name ${builtin_extensions})
    get_property(extension_${name}_sources GLOBAL PROPERTY extension_${name}_sources)
    list(APPEND builtin_extension_sources ${extension_${name}_sources})
endforeach()

# XXX Associate Py_NO_ENABLE_SHARED with "_ctypes/callbacks.c" if corresponding
#     extension is builtin.
if(WIN32)
    foreach(src ${builtin_extension_sources})
        get_filename_component(filename ${src} NAME)
        if(${filename} STREQUAL "callbacks.c")
            set_property(SOURCE ${src} APPEND PROPERTY COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED)
        endif()
    endforeach()
endif()
list(APPEND LIBPYTHON_OMIT_FROZEN_SOURCES ${builtin_extension_sources})

# Collect builtin extension link libraries
set(builtin_link_libraries)
foreach(name ${builtin_extensions})
    get_property(extension_${name}_link_libraries GLOBAL PROPERTY extension_${name}_link_libraries)
    list(APPEND builtin_link_libraries ${extension_${name}_link_libraries})
endforeach()

# Collect builtin extension includedirs
set(builtin_includedirs)
foreach(name ${builtin_extensions})
    get_property(extension_${name}_includedirs GLOBAL PROPERTY extension_${name}_includedirs)
    list(APPEND builtin_includedirs ${extension_${name}_includedirs})
endforeach()

# Collect builtin extension definitions
foreach(name ${builtin_extensions})
    get_property(extension_${name}_definitions GLOBAL PROPERTY extension_${name}_definitions)
    if(extension_${name}_definitions)
        set_property(SOURCE ${extension_${name}_sources}
            APPEND PROPERTY COMPILE_DEFINITIONS ${extension_${name}_definitions})
    endif()
endforeach()

include_directories(${builtin_includedirs})

# Create the parts of config.c for platform-specific and user-controlled
# builtin modules.
set(init_return_type_2 "void")
set(init_prefix_2 "init")

set(init_return_type_3 "PyObject*")
set(init_prefix_3 "PyInit_")

set(config_inits "")
set(config_entries "")
foreach(ext ${builtin_extensions})
    set(config_inits "${config_inits}extern ${init_return_type_${PY_VERSION_MAJOR}} ${init_prefix_${PY_VERSION_MAJOR}}${ext}(void);\n")
    set(config_entries "${config_entries}    {\"${ext}\", ${init_prefix_${PY_VERSION_MAJOR}}${ext}},\n")
endforeach()

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/config_${PY_VERSION_MAJOR}.c.in
    ${CMAKE_BINARY_DIR}/CMakeFiles/config.c
    )

# Collect libpython target libraries
set(LIBPYTHON_TARGET_LIBRARIES
  ${builtin_link_libraries}
  )
if(HAVE_LIBDL)
  list(APPEND LIBPYTHON_TARGET_LIBRARIES ${HAVE_LIBDL})
endif()
if(WITH_THREAD)
    list(APPEND LIBPYTHON_TARGET_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()
if(UNIX)
    if(HAVE_LIBUTIL)
        list(APPEND LIBPYTHON_TARGET_LIBRARIES ${LIBUTIL_LIBRARIES})
    endif()
    list(APPEND LIBPYTHON_TARGET_LIBRARIES ${M_LIBRARIES})
endif()
if(WIN32 AND IS_PY3)
    list(APPEND LIBPYTHON_TARGET_LIBRARIES ws2_32) # Required by signalmodule
endif()

if(BUILD_FREEZE_IMPORTLIB)
    set(LIBPYTHON_FROZEN_SOURCES )
    if(IS_PY3)

    # Build _freeze_importlib executable
    add_executable(_freeze_importlib
      ${SRC_DIR}/Programs/_freeze_importlib.c
      ${LIBPYTHON_OMIT_FROZEN_SOURCES}
      )
    target_link_libraries(_freeze_importlib ${LIBPYTHON_TARGET_LIBRARIES})

    # Freeze modules
    set(LIBPYTHON_FROZEN_SOURCES
      ${SRC_DIR}/Python/importlib_external.h
      ${SRC_DIR}/Python/importlib.h
    )
    add_custom_command(
      OUTPUT ${LIBPYTHON_FROZEN_SOURCES}
      COMMAND
        ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:_freeze_importlib>
          ${SRC_DIR}/Lib/importlib/_bootstrap_external.py
          ${SRC_DIR}/Python/importlib_external.h
      COMMAND
        ${CMAKE_CROSSCOMPILING_EMULATOR} $<TARGET_FILE:_freeze_importlib>
          ${SRC_DIR}/Lib/importlib/_bootstrap.py
          ${SRC_DIR}/Python/importlib.h
      DEPENDS
        _freeze_importlib
        ${SRC_DIR}/Lib/importlib/_bootstrap_external.py
        ${SRC_DIR}/Lib/importlib/_bootstrap.py
    )

    # This is a convenience target allowing to regenerate
    # the frozen sources.
    add_custom_target(freeze_modules DEPENDS ${LIBPYTHON_FROZEN_SOURCES})

    endif()
endif()

if(BUILD_PGEN)
    # Build pgen executable
    set(PGEN2_SOURCES
        ${SRC_DIR}/Parser/parsetok.c
    )
    set(PGEN3_SOURCES
        ${SRC_DIR}/Python/dynamic_annotations.c
        ${SRC_DIR}/Parser/parsetok_pgen.c
    )
    add_executable(pgen
        ${PARSER_COMMON_SOURCES}
        ${PGEN${PY_VERSION_MAJOR}_SOURCES}
        ${SRC_DIR}/Objects/obmalloc.c
        ${SRC_DIR}/Python/mysnprintf.c
        ${SRC_DIR}/Python/pyctype.c
        ${SRC_DIR}/Parser/tokenizer_pgen.c
        ${SRC_DIR}/Parser/printgrammar.c
        ${SRC_DIR}/Parser/pgenmain.c
    )
endif()

# Collect libpython sources
set(LIBPYTHON_SOURCES
    ${LIBPYTHON_OMIT_FROZEN_SOURCES}
    ${LIBPYTHON_FROZEN_SOURCES}
)
if(UNIX)
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/Python/frozen.c
    )
endif()

# Build python libraries
function(add_libpython name type install component)
    add_library(${name} ${type} ${LIBPYTHON_SOURCES})
    target_link_libraries(${name} ${LIBPYTHON_TARGET_LIBRARIES})

    if(MSVC)
        # Explicitly disable COMDAT folding. Note that this was not required
        # in the original "pcbuild.sln" solution file because it was side effect
        # of having "/Zi" flag set.
        set_target_properties(${name} PROPERTIES LINK_FLAGS /OPT:NOICF)
    endif()

    set_target_properties(${name} PROPERTIES
#        OUTPUT_NAME ${LIBPYTHON}${ABIFLAGS}
        OUTPUT_NAME ${LIBPYTHON}${KLAYGE_OUTPUT_SUFFIX}
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIBPYTHON_LIBDIR}
    )
    if(HAVE_POSITION_INDEPENDENT_CODE)
        set_target_properties(${name} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
        )
    endif()

    # Export target
    set_property(GLOBAL APPEND PROPERTY PYTHON_TARGETS ${name})

    if(install)
        install(TARGETS ${name} EXPORT PythonTargets
            ARCHIVE DESTINATION ${LIBPYTHON_ARCHIVEDIR}
            LIBRARY DESTINATION ${LIBPYTHON_LIBDIR}
            RUNTIME DESTINATION ${LIBPYTHON_LIBDIR}
            COMPONENT ${component}
        )
    endif()
endfunction()

if(BUILD_SHARED)
    add_libpython(libpython-shared SHARED 1 Runtime)
    set_target_properties(libpython-shared PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_ARCHIVEDIR}
    )
    if(APPLE)
        # HACK For python <= 2.7.3, this fix link error related to undefined _environ symbol and
        #      is equivalent to solution implemented in commit http://hg.python.org/cpython/rev/864b983
        #      The property is set here because source file properties can't be in one directory
        #      and used to build a target in an other directory.
        set_property(
            SOURCE ${SRC_DIR}/Modules/posixmodule.c
            APPEND PROPERTY COMPILE_DEFINITIONS WITH_NEXT_FRAMEWORK)
    endif()

    set(targetname "libpython3-shared")

    if(IS_PY3 AND MSVC)
        # XXX Add BuildPython3_dDef

        # Generate 'python3stub.def'
        set(pythonstub_def ${CMAKE_BINARY_DIR}/${LIBPYTHON_ARCHIVEDIR}/${CMAKE_CFG_INTDIR}/python3stub.def)
        add_custom_command(
            OUTPUT ${pythonstub_def}
            COMMAND ${CMAKE_COMMAND}
                -DINPUT_DEF_FILE:PATH=${SRC_DIR}/PC/python3.def
                -DOUTPUT_DEF_FILE:PATH=${PROJECT_BINARY_DIR}/CMakeFiles/python3stub.def
                -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_libpythonstub_def.cmake
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${PROJECT_BINARY_DIR}/CMakeFiles/python3stub.def
                ${pythonstub_def}
        )
        add_custom_target(generate_libpythonstub_def DEPENDS ${pythonstub_def})

        # Build 'python3.dll'
        add_library(${targetname} SHARED ${SRC_DIR}/PC/python3dll.c ${SRC_DIR}/PC/python3.def)
        set_target_properties(${targetname} PROPERTIES
            OUTPUT_NAME python3
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
            INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIBPYTHON_LIBDIR}
        )
        add_dependencies(${targetname} generate_libpythonstub_def)

        # Build 'python3stub.lib' before linking 'python3.dll'
        set(python3stub_lib ${CMAKE_BINARY_DIR}/${LIBPYTHON_ARCHIVEDIR}/${CMAKE_CFG_INTDIR}/python3stub.lib)
        set(machine X86)
        if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
            set(machine X64)
        endif()
        add_custom_command(
            TARGET ${targetname} PRE_LINK
            COMMAND lib /nologo /def:${pythonstub_def} /out:${python3stub_lib} /MACHINE:${machine}
            COMMENT "Rebuilding python3stub.lib"
            VERBATIM
        )

        target_link_libraries(${targetname} ${python3stub_lib})
    endif()

    if(IS_PY3 AND UNIX AND NOT APPLE)
        add_library(${targetname} SHARED ${PROJECT_SOURCE_DIR}/cmake/empty.c)
        set_target_properties(${targetname} PROPERTIES
            LINK_FLAGS "-Wl,--no-as-needed"
            OUTPUT_NAME python3
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
            INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIBPYTHON_LIBDIR}
        )
        target_link_libraries(${targetname} libpython-shared)
    endif()

    # Export target
    if(TARGET ${targetname})
        set_property(GLOBAL APPEND PROPERTY PYTHON_TARGETS ${targetname})

        install(TARGETS ${targetname} EXPORT PythonTargets
            ARCHIVE DESTINATION ${LIBPYTHON_ARCHIVEDIR}
            LIBRARY DESTINATION ${LIBPYTHON_LIBDIR}
            RUNTIME DESTINATION ${LIBPYTHON_LIBDIR}
        )
    endif()

endif()

if(BUILD_STATIC)
    add_libpython(libpython-static STATIC ${INSTALL_DEVELOPMENT} Development)
    if(HAVE_TARGET_COMPILE_DEFINITIONS)
        target_compile_definitions(libpython-static PUBLIC Py_NO_ENABLE_SHARED)
    else()
        set_target_properties(libpython-static PROPERTIES
            COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED
        )
    endif()
    set_target_properties(libpython-static PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
    )
    if(INSTALL_DEVELOPMENT)
        install(TARGETS libpython-static
            ARCHIVE DESTINATION ${PYTHONHOME}/config/
            COMPONENT Development)
    endif()
endif()
