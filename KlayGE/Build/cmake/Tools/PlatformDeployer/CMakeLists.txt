SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Tools/src/PlatformDeployer/PlatformDeployer.cpp
)

SET(RESOURCE_FILES ${RESOURCE_FILES}
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/d3d_11_0.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/d3d_11_1.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/d3d_12_0.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/d3d_12_1.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_1.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_2.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_3.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_4.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_5.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gl_4_6.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gles_3_0.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gles_3_1.plat
	${KLAYGE_PROJECT_DIR}/Tools/media/PlatformDeployer/PlatConf/gles_3_2.plat
)

SET(EXTRA_LINKED_LIBRARIES ${EXTRA_LINKED_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${KLAYGE_FILESYSTEM_LIBRARY})

SETUP_TOOL(PlatformDeployer)
