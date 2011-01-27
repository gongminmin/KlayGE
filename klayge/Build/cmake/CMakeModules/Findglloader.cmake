# - Find glloader library
# Find the native glloader includes and library
# This module defines

#  GLLOADER_FOUND, If false, do not try to use glloader.
#  GLLOADER_INCLUDE_DIR, where to find glloader.h, etc.
#  GLLOADER_LIBRARY_DEBUG, GLLOADER_LIBRARY

SET( GLLOADER_ARCHITECTURE "_x86" )
IF( CMAKE_CL_64)
    SET( GLLOADER_ARCHITECTURE "_x64" )
ENDIF( CMAKE_CL_64 )

FIND_PATH( GLLOADER_INCLUDE_DIR glloader/glloader.h
    PATHS
    ${3RDPARTY_DIR}/include
    /usr/include
    /local/usr/include
)

FIND_LIBRARY( GLLOADER_LIBRARY 
    NAMES glloader${GLLOADER_ARCHITECTURE}
    PATHS
    ${3RDPARTY_DIR}/lib
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( GLLOADER_LIBRARY_DEBUG 
    NAMES glloader${GLLOADER_ARCHITECTURE}_d 
    PATHS
    ${3RDPARTY_DIR}/lib
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

SET(GLLOADER_FOUND "NO")
IF(GLLOADER_INCLUDE_DIR AND GLLOADER_LIBRARY)
    SET(GLLOADER_FOUND "YES")
ENDIF(GLLOADER_INCLUDE_DIR AND GLLOADER_LIBRARY)

