# 
# Try to find nVidia's Cg compiler, runtime libraries, and include path. 
# Once done this will define 
# 
# CG_FOUND        - system has NVidia Cg and it can be used.  
# CG_INCLUDE_PATH = directory where cg.h resides 
# CG_LIBRARY = full path to libCg.so (Cg.DLL on win32) 
# CG_GL_LIBRARY = full path to libCgGL.so (CgGL.dll on win32) 
# CG_COMPILER = full path to cgc (cgc.exe on win32) 
#
 
# On OSX default to using the framework version of Cg. 

IF (APPLE) 
  INCLUDE(${CMAKE_ROOT}/Modules/CMakeFindFrameworks.cmake) 
  SET(CG_FRAMEWORK_INCLUDES) 
  CMAKE_FIND_FRAMEWORKS(Cg) 
  IF (Cg_FRAMEWORKS) 
    FOREACH(dir ${Cg_FRAMEWORKS}) 
      SET(CG_FRAMEWORK_INCLUDES ${CG_FRAMEWORK_INCLUDES} 
        ${dir}/Headers ${dir}/PrivateHeaders) 
    ENDFOREACH(dir) 
 
    #Find the include  dir 
    FIND_PATH(CG_INCLUDE_PATH cg.h 
      ${CG_FRAMEWORK_INCLUDES} 
      ) 
 
    #Since we are using Cg framework, we must link to it. 
    SET(CG_LIBRARY "-framework Cg" CACHE STRING "Cg library") 
    SET(CG_GL_LIBRARY "-framework Cg" CACHE STRING "Cg GL library") 
  ENDIF (Cg_FRAMEWORKS) 
  FIND_PROGRAM(CG_COMPILER cgc 
    ${PROJECT_BINARY_DIR}/bin 
    ${PROJECT_SOURCE_DIR}/bin 
    /usr/bin 
    /usr/local/bin 
    DOC "The Cg compiler" 
    NO_DEFAULT_PATH 
    ) 
  FIND_PROGRAM(CG_COMPILER cgc) 
ELSE (APPLE) 
  IF (WIN32) 
    FIND_PROGRAM( CG_COMPILER cgc 
      "C:/Program Files/NVIDIA Corporation/Cg/bin" 
      "C:/Program Files/Cg" 
      ${PROJECT_SOURCE_DIR}/../Cg 
      DOC "The Cg Compiler" 
      ) 
    IF (CG_COMPILER) 
      GET_FILENAME_COMPONENT(CG_COMPILER_DIR ${CG_COMPILER} PATH) 
      GET_FILENAME_COMPONENT(CG_COMPILER_SUPER_DIR ${CG_COMPILER_DIR} PATH) 
    ELSE (CG_COMPILER) 
      SET (CG_COMPILER_DIR .) 
      SET (CG_COMPILER_SUPER_DIR ..) 
    ENDIF (CG_COMPILER) 
    FIND_PATH( CG_INCLUDE_PATH Cg/cg.h 
      "C:/Program Files/NVIDIA Corporation/Cg/include" 
      "C:/Program Files/Cg" 
      ${PROJECT_SOURCE_DIR}/../Cg 
      ${CG_COMPILER_SUPER_DIR}/include 
      ${CG_COMPILER_DIR} 
      DOC "The directory where Cg/cg.h resides" 
      ) 
    FIND_LIBRARY( CG_LIBRARY 
      NAMES Cg 
      PATHS 
      "C:/Program Files/NVIDIA Corporation/Cg/lib" 
      "C:/Program Files/Cg" 
      ${PROJECT_SOURCE_DIR}/../Cg 
      ${CG_COMPILER_SUPER_DIR}/lib 
      ${CG_COMPILER_DIR} 
      DOC "The Cg runtime library" 
      ) 
    FIND_LIBRARY( CG_GL_LIBRARY 
      NAMES CgGL 
      PATHS 
      "C:/Program Files/NVIDIA Corporation/Cg/lib" 
      "C:/Program Files/Cg" 
      ${PROJECT_SOURCE_DIR}/../Cg 
      ${CG_COMPILER_SUPER_DIR}/lib 
      ${CG_COMPILER_DIR} 
      DOC "The Cg runtime library" 
      ) 
  ELSE (WIN32) 
  MESSAGE(STATUS "project binary dir is ${PROJECT_BINARY_DIR}") 
  MESSAGE(STATUS "project source dir is ${PROJECT_SOURCE_DIR}") 
    FIND_PROGRAM( CG_COMPILER cgc 
      PATHS 
      ${PROJECT_BINARY_DIR}/bin 
      ${PROJECT_SOURCE_DIR}/bin 
      ENV PATH 
       /usr/bin 
      /usr/local/bin 
      DOC "The Cg Compiler" 
      NO_DEFAULT_PATH 
      ) 
    FIND_PROGRAM(CG_COMPILER cgc) 
    GET_FILENAME_COMPONENT(CG_COMPILER_DIR "${CG_COMPILER}" PATH) 
    GET_FILENAME_COMPONENT(CG_COMPILER_SUPER_DIR "${CG_COMPILER_DIR}" PATH) 
    FIND_LIBRARY( CG_LIBRARY Cg 
      PATHS 
      ${PROJECT_BINARY_DIR}/lib64 
      ${PROJECT_BINARY_DIR}/lib 
      ${PROJECT_SOURCE_DIR}/lib64 
      ${PROJECT_SOURCE_DIR}/lib 
      ${CG_COMPILER_SUPER_DIR}/lib64 
      ${CG_COMPILER_SUPER_DIR}/lib 
      ENV LD_LIBRARY_PATH 
      ENV LIBRARY_PATH 
      /usr/lib64 
      /usr/lib 
      /usr/local/lib64 
      /usr/local/lib 
      DOC "The Cg runtime library" 
      NO_DEFAULT_PATH 
      ) 
    FIND_LIBRARY( CG_LIBRARY Cg) 
    FIND_LIBRARY( CG_GL_LIBRARY CgGL 
      PATHS 
      ${PROJECT_BINARY_DIR}/lib64 
      ${PROJECT_BINARY_DIR}/lib 
      ${PROJECT_SOURCE_DIR}/lib64 
      ${PROJECT_SOURCE_DIR}/lib 
      ${CG_COMPILER_SUPER_DIR}/lib64 
      ${CG_COMPILER_SUPER_DIR}/lib 
      ENV LD_LIBRARY_PATH 
      ENV LIBRARY_PATH 
      /usr/lib64 
      /usr/lib 
      /usr/local/lib64 
      /usr/local/lib 
      DOC "The Cg runtime library" 
      NO_DEFAULT_PATH 
      ) 
    FIND_LIBRARY( CG_GL_LIBRARY CgGL) 
    GET_FILENAME_COMPONENT(CG_LIBRARY_DIR "${CG_LIBRARY}" PATH) 
    GET_FILENAME_COMPONENT(CG_LIBRARY_SUPER_DIR "${CG_LIBRARY_DIR}" PATH) 
    FIND_PATH( CG_INCLUDE_PATH Cg/cg.h 
      ${PROJECT_BINARY_DIR}/include 
      ${PROJECT_SOURCE_DIR}/include 
      ${CG_LIBRARY_SUPER_DIR}/include 
      ENV CPATH 
      /usr/include 
      /usr/local/include 
      DOC "The directory where Cg/cg.h resides" 
      NO_DEFAULT_PATH 
      ) 
    FIND_PATH( CG_INCLUDE_PATH Cg/cg.h) 
  ENDIF (WIN32) 
ENDIF (APPLE) 
 
IF (CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY) 
  SET( CG_FOUND TRUE) 
ELSE (CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY) 
  SET( CG_FOUND FALSE) 
ENDIF (CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY) 
 
IF(CG_FOUND) 
  IF(NOT CG_FIND_QUIETLY) 
    MESSAGE(STATUS "Found CG: ${CG_LIBRARY},${CG_GL_LIBRARY}") 
  ENDIF(NOT CG_FIND_QUIETLY) 
ELSE(CG_FOUND) 
  IF(CG_FIND_REQUIRED) 
    MESSAGE(FATAL_ERROR "Could not find CG") 
  ENDIF(CG_FIND_REQUIRED) 
ENDIF(CG_FOUND) 
 
#MARK_AS_ADVANCED(CG_FOUND CG_INCLUDE_PATH CG_LIBRARY CG_GL_LIBRARY CG_COMPILER) 
