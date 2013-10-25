# Install all the header files in Include/

set(includedir ${INCLUDE_BUILD_DIR})
file(GLOB_RECURSE hfiles RELATIVE ${includedir} "${includedir}/*")

foreach(file ${hfiles})
    get_filename_component(path ${file} PATH)
    install(FILES ${includedir}/${file} DESTINATION ${INCLUDE_INSTALL_DIR}/${path} COMPONENT Development)
endforeach(file)
