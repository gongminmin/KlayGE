# - Find libogg and vorbis libraries
# Find the native libogg and vorbis includes and libraries
# This module defines

#  LIBOGG_FOUND, If false, do not try to use libogg.
#  LIBOGG_INCLUDE_DIR, where to find ogg.h, etc.
#  LIBOGG_LIBRARY_DEBUG, LIBOGG_LIBRARY, LIBOGG_LIBRARIES

FIND_PATH( LIBOGG_INCLUDE_DIR ogg/ogg.h
    PATHS
    /usr/include
    /local/usr/include
)

FIND_LIBRARY( LIBOGG_LIBRARY 
    NAMES libogg libogg_static
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( LIBOGG_LIBRARY_DEBUG 
    NAMES libogg_d libogg_static_d
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

SET(LIBOGG_LIBRARIES ${LIBOGG_LIBRARY})
IF(LIBOGG_LIBRARY_DEBUG)
    SET(LIBOGG_LIBRARIES "debug ${LIBOGG_LIBRARY_DEBUG} optimized ${LIBOGG_LIBRARY}")
ENDIF(LIBOGG_LIBRARY_DEBUG)

SET(LIBOGG_FOUND "NO")
IF(LIBOGG_INCLUDE_DIR AND LIBOGG_LIBRARY)
    SET(LIBOGG_FOUND "YES")
ENDIF(LIBOGG_INCLUDE_DIR AND LIBOGG_LIBRARY)

#  VORBIS_FOUND, If false, do not try to use vorbis.
#  VORBIS_INCLUDE_DIR, where to find ogg.h, etc.
#  VORBIS_LIBRARY_DEBUG, VORBIS_LIBRARY, VORBISFILE_LIBRARY_DEBUG, VORBISFILE_LIBRARY, VORBIS_LIBRARIES
FIND_PATH( VORBIS_INCLUDE_DIR vorbis/vorbisenc.h
    PATHS
    /usr/include
    /local/usr/include
)

FIND_LIBRARY( VORBIS_LIBRARY 
    NAMES libvorbis libvorbis_static
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( VORBIS_LIBRARY_DEBUG 
    NAMES libvorbis_d libvorbis_static_d
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( VORBISFILE_LIBRARY 
    NAMES libvorbisfile libvorbisfile_static
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

FIND_LIBRARY( VORBISFILE_LIBRARY_DEBUG 
    NAMES libvorbisfile_d libvorbisfile_static_d
    PATHS
    /usr/lib
    /usr/lib64
    /local/usr/lib
    /local/usr/lib64
)

SET(VORBIS_LIBRARIES ${VORBIS_LIBRARY})
IF(VORBIS_LIBRARY_DEBUG)
    SET(VORBIS_LIBRARIES "debug ${VORBIS_LIBRARY_DEBUG} optimized ${VORBIS_LIBRARY}")
ENDIF(VORBIS_LIBRARY_DEBUG)

IF(VORBISFILE_LIBRARY_DEBUG)
    SET(VORBIS_LIBRARIES ${VORBIS_LIBRARIES} "debug ${VORBISFILE_LIBRARY_DEBUG} optimized ${VORBISFILE_LIBRARY}")
ELSE(VORBISFILE_LIBRARY_DEBUG)
    SET(VORBIS_LIBRARIES ${VORBIS_LIBRARIES} ${VORBISFILE_LIBRARY})
ENDIF(VORBISFILE_LIBRARY_DEBUG)

SET(VORBIS_FOUND "NO")
IF(VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
    SET(VORBIS_FOUND "YES")
ENDIF(VORBIS_INCLUDE_DIR AND VORBIS_LIBRARY AND VORBISFILE_LIBRARY)
