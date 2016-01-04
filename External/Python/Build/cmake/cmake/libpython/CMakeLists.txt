add_definitions(-DPy_BUILD_CORE)
add_definitions(-DNDEBUG)
if(WINDOWS_STORE OR WINDOWS_PHONE)
  add_definitions(-D_WIN32_WINNT=0x0602)
  add_definitions(-DWINVER=0x0602)
  add_definitions(-DNTDDI_VERSION=0x06020000)
endif()

set(LIBPYTHON_SOURCES
    ${CMAKE_BINARY_DIR}/CMakeFiles/config.c
    ${SRC_DIR}/Modules/_codecsmodule.c
    ${SRC_DIR}/Modules/_tracemalloc.c
    ${SRC_DIR}/Modules/errnomodule.c
    ${SRC_DIR}/Modules/faulthandler.c
    ${SRC_DIR}/Modules/gcmodule.c
    ${SRC_DIR}/Modules/getbuildinfo.c
    ${SRC_DIR}/Modules/hashtable.c
    ${SRC_DIR}/Modules/main.c
    ${SRC_DIR}/Modules/signalmodule.c
    ${SRC_DIR}/Modules/_sre.c
    ${SRC_DIR}/Modules/symtablemodule.c
    ${SRC_DIR}/Modules/_weakref.c
    ${SRC_DIR}/Modules/xxsubtype.c
    ${SRC_DIR}/Modules/zipimport.c
    ${SRC_DIR}/Objects/abstract.c
    ${SRC_DIR}/Objects/accu.c
    ${SRC_DIR}/Objects/boolobject.c
    ${SRC_DIR}/Objects/bytearrayobject.c
    ${SRC_DIR}/Objects/bytes_methods.c
    ${SRC_DIR}/Objects/bytesobject.c
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
    ${SRC_DIR}/Objects/namespaceobject.c
    ${SRC_DIR}/Objects/object.c
    ${SRC_DIR}/Objects/odictobject.c
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
    ${SRC_DIR}/Parser/acceler.c
    ${SRC_DIR}/Parser/bitset.c
    ${SRC_DIR}/Parser/firstsets.c
    ${SRC_DIR}/Parser/grammar1.c
    ${SRC_DIR}/Parser/grammar.c
    ${SRC_DIR}/Parser/listnode.c
    ${SRC_DIR}/Parser/metagrammar.c
    ${SRC_DIR}/Parser/myreadline.c
    ${SRC_DIR}/Parser/node.c
    ${SRC_DIR}/Parser/parser.c
    ${SRC_DIR}/Parser/parsetok.c
    ${SRC_DIR}/Parser/pgen.c
    ${SRC_DIR}/Parser/tokenizer.c
    ${SRC_DIR}/Python/asdl.c
    ${SRC_DIR}/Python/ast.c
    ${SRC_DIR}/Python/bltinmodule.c
    ${SRC_DIR}/Python/ceval.c
    ${SRC_DIR}/Python/codecs.c
    ${SRC_DIR}/Python/compile.c
    ${SRC_DIR}/Python/dtoa.c
    ${SRC_DIR}/Python/errors.c
    ${SRC_DIR}/Python/fileutils.c
    ${SRC_DIR}/Python/formatter_unicode.c
    ${SRC_DIR}/Python/frozen.c
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
    ${SRC_DIR}/Python/pyhash.c
    ${SRC_DIR}/Python/pylifecycle.c
    ${SRC_DIR}/Python/pymath.c
    ${SRC_DIR}/Python/pystate.c
    ${SRC_DIR}/Python/pystrcmp.c
    ${SRC_DIR}/Python/pystrtod.c
    ${SRC_DIR}/Python/pystrhex.c
    ${SRC_DIR}/Python/Python-ast.c
    ${SRC_DIR}/Python/pythonrun.c
    ${SRC_DIR}/Python/pytime.c
    ${SRC_DIR}/Python/random.c
    #${SRC_DIR}/Python/strdup.c   # Have some problems under Linux RelWithDebInfo
    ${SRC_DIR}/Python/structmember.c
    ${SRC_DIR}/Python/symtable.c
    ${SRC_DIR}/Python/sysmodule.c
    ${SRC_DIR}/Python/traceback.c
    ${SRC_DIR}/Python/_warnings.c
    ${builtin_source}
)

if(UNIX)
    foreach(filename class complex float int method string type unicode weakref)
        set_property(SOURCE ${SRC_DIR}/Objects/${filename}object.c PROPERTY COMPILE_FLAGS -Wno-unused-value)
    endforeach()
endif(UNIX)

if(WITH_THREAD)
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/Modules/_threadmodule.c
        ${SRC_DIR}/Python/thread.c
    )
endif()

if(UNIX OR MINGW)
    set_property(
        SOURCE ${SRC_DIR}/Python/getplatform.c
        PROPERTY COMPILE_DEFINITIONS
            PLATFORM="${PY_PLATFORM}"
    )
endif(UNIX OR MINGW)

# Platform-specific source files
if(UNIX)
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/Python/dynload_shlib.c
        ${SRC_DIR}/Modules/getpath.c
    )
    set_property(
        SOURCE ${SRC_DIR}/Modules/getpath.c
        PROPERTY COMPILE_FLAGS
            -Wno-deprecated-declarations
        )
    set(PYTHONPATH "${EXTRA_PYTHONPATH}:lib-dynload:plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}:lib-tk")
    endif(ENABLE_TKINTER)
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
    # HACK To workaround limitation in escaping logic of CMake, the pythonpath
    #      separator is conditionally set depending of the version of Visual Studio.
    #      See http://cmake.org/Bug/view.php?id=14073
    if( ("${MSVC_VERSION}" VERSION_GREATER "1599") AND ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio") )
        set(PATHSEP "%3B")
    elseif( (${MSVC}) AND ("${CMAKE_GENERATOR}" MATCHES "^Ninja") )
        set(PATHSEP "\;")
    else()
        set(PATHSEP ";")
    endif()
    list(APPEND LIBPYTHON_SOURCES
        ${SRC_DIR}/PC/dl_nt.c
        ${SRC_DIR}/PC/getpathp.c
        ${SRC_DIR}/PC/invalid_parameter_handler.c
        ${SRC_DIR}/Python/dynload_win.c
    )
    string(REPLACE "/" "\\\\" PYTHONHOME_ESCAPED ${PYTHONHOME})
    string(REPLACE "/" "\\\\" EXTENSION_INSTALL_DIR_ESCAPED ${EXTENSION_INSTALL_DIR})
    set(PYTHONPATH "${EXTRA_PYTHONPATH}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${EXTENSION_INSTALL_DIR_ESCAPED}\\\\${CMAKE_CFG_INTDIR}")
    set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\plat-${PY_PLATFORM}")
    if(ENABLE_TKINTER)
        set(PYTHONPATH "${PYTHONPATH}${PATHSEP}.\\\\${PYTHONHOME_ESCAPED}\\\\lib-tk")
    endif(ENABLE_TKINTER)

    if(NOT MINGW)
        set_property(
            SOURCE ${SRC_DIR}/PC/getpathp.c
            PROPERTY COMPILE_DEFINITIONS
                LANDMARK=L"${PYTHONHOME_ESCAPED}\\\\os.py"
                "PYTHONPATH=L\"${PYTHONPATH}\""
        )
    endif(NOT MINGW)
    set_property(
        SOURCE ${SRC_DIR}/PC/dl_nt.c
        PROPERTY COMPILE_DEFINITIONS Py_ENABLE_SHARED
    )
endif(UNIX)

# Create the parts of config.c for platform-specific and user-controlled
# builtin modules.

include_directories(${builtin_includedirs})

set(config_inits "")
set(config_entries "")
foreach(ext ${builtin_extensions})
    set(config_inits "${config_inits}extern PyObject* PyInit_${ext}(void);\n")
    set(config_entries "${config_entries}    {\"${ext}\", PyInit_${ext}},\n")
endforeach(ext)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/config.c.in
    ${CMAKE_BINARY_DIR}/CMakeFiles/config.c
    )


function(add_libpython name type)
    add_library(${name} ${type} ${LIBPYTHON_SOURCES})
    if(BUILD_SHARED)
        target_link_libraries(${name}
            ${builtin_link_libraries}
            ${HAVE_LIBDL}
        )
    else(BUILD_SHARED)
        target_link_libraries(${name}
            ${builtin_link_libraries}
        )
    endif(BUILD_SHARED)
    if(WITH_THREAD)
        target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})
    endif(WITH_THREAD)

    if(UNIX)
        target_link_libraries(${name} ${HAVE_LIBUTIL} ${HAVE_LIBM})
    endif(UNIX)

    set_target_properties(${name} PROPERTIES
        OUTPUT_NAME ${LIBPYTHON}${KLAYGE_OUTPUT_SUFFIX}
        DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_LIBDIR}
        INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${LIBPYTHON_LIBDIR}
    )

    if(builtin_definitions)
        set_target_properties(${name} PROPERTIES COMPILE_DEFINITIONS "${builtin_definitions}")
    endif(builtin_definitions)

    # Export target
    set_property(GLOBAL APPEND PROPERTY PYTHON_TARGETS ${name})

    install(TARGETS ${name} EXPORT PythonTargets
        ARCHIVE DESTINATION ${LIBPYTHON_ARCHIVEDIR}
        LIBRARY DESTINATION ${LIBPYTHON_LIBDIR}
        RUNTIME DESTINATION ${LIBPYTHON_LIBDIR}
    )
endfunction(add_libpython)

if(BUILD_SHARED)
    add_libpython(libpython-shared SHARED)
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
            PROPERTY COMPILE_DEFINITIONS WITH_NEXT_FRAMEWORK)
    endif(APPLE)
endif(BUILD_SHARED)

if(BUILD_STATIC)
    add_libpython(libpython-static STATIC)
    set_target_properties(libpython-static PROPERTIES
        COMPILE_DEFINITIONS Py_NO_ENABLE_SHARED
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
        ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/${LIBPYTHON_STATIC_ARCHIVEDIR}
    )
    install(TARGETS libpython-static ARCHIVE DESTINATION ${PYTHONHOME}/config/)

endif(BUILD_STATIC)
