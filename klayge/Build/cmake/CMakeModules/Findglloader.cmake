# - Find glloader library
# Find the native glloader includes and library
# This module defines

#  GLLOADER_FOUND, If false, do not try to use glloader.
#  GLLOADER_INCLUDE_DIR, where to find glloader.h, etc.
#  GLLOADER_LIBRARY_DEBUG, GLLOADER_LIBRARY, GLLOADER_LIBRARIES

FIND_PATH( GLLOADER_INCLUDE_DIR glloader/glloader.h
    PATHS
    /usr/include
    /local/usr/include
)

FIND_LIBRARY( GLLOADER_LIBRARY 
    NAMES glloader 
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( GLLOADER_LIBRARY_DEBUG 
    NAMES glloaderd 
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

SET(GLLOADER_LIBRARIES ${GLLOADER_LIBRARY})
IF(GLLOADER_LIBRARY_DEBUG)
    SET(GLLOADER_LIBRARIES "debug ${GLLOADER_LIBRARY_DEBUG} optimized ${GLLOADER_LIBRARY}")
ENDIF(GLLOADER_LIBRARY_DEBUG)

SET(GLLOADER_FOUND "NO")
IF(GLLOADER_INCLUDE_DIR AND GLLOADER_LIBRARY)
    SET(GLLOADER_FOUND "YES")
ENDIF(GLLOADER_INCLUDE_DIR AND GLLOADER_LIBRARY)

