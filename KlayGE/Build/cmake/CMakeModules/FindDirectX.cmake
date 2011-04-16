SET( D3D10_FOUND FALSE )
SET( D3D11_FOUND FALSE )
SET( DINPUT_FOUND FALSE )
SET( DSOUND_FOUND FALSE )

IF( WIN32 )
    SET( DX_ARCHITECTURE "x86" )
    IF( CMAKE_CL_64)
        SET( DX_ARCHITECTURE "x64" )
    ENDIF( CMAKE_CL_64 )
    
    FIND_PATH( DX_ROOT_DIR
               NAMES
               Include/d3d10.h Include/d3d11.h
               PATHS
               $ENV{PATH}
               $ENV{DXSDK_DIR}
               $ENV{PROGRAMFILES}/Microsoft DirectX SDK
    )
    
    FIND_PATH( D3D10_INCLUDE_DIR d3d10.h
               PATHS
               ${DX_ROOT_DIR}/Include
    )
    
    FIND_LIBRARY( D3D10_LIBRARY d3d10.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    FIND_LIBRARY( D3DX10_LIBRARY d3dx10.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    SET( D3D10_LIBRARIES
         ${D3D10_LIBRARY} ${D3DX10_LIBRARY}
    )
    
    FIND_PATH( D3D11_INCLUDE_DIR d3d11.h
               PATHS
               ${DX_ROOT_DIR}/Include
    )
    
    FIND_LIBRARY( D3D11_LIBRARY d3d11.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    FIND_LIBRARY( D3DX11_LIBRARY d3dx11.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    SET( D3D11_LIBRARIES
         ${D3D11_LIBRARY} ${D3DX11_LIBRARY}
    )
    
    FIND_PATH( DINPUT_INCLUDE_DIR dinput.h
               PATHS
               ${DX_ROOT_DIR}/Include
    )
    
    FIND_LIBRARY( DINPUT_LIBRARY dinput7.lib dinput8.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    SET( DINPUT_LIBRARIES ${DINPUT_LIBRARY} )
    
    FIND_PATH( DSOUND_INCLUDE_DIR dsound.h
               PATHS
               ${DX_ROOT_DIR}/Include
    )
    
    FIND_LIBRARY( DSOUND_LIBRARY dsound.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    SET( DSOUND_LIBRARIES ${DSOUND_LIBRARY} )
    
    FIND_LIBRARY( DX_COMPILER_LIBRARY d3dcompiler.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    FIND_LIBRARY( DX_GUID_LIBRARY dxguid.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    FIND_LIBRARY( DX_ERR_LIBRARY dxerr.lib
                  PATHS
                  ${DX_ROOT_DIR}/Lib/${DX_ARCHITECTURE}
    )
    
    SET( DX_UTIL_LIBRARIES
         ${DX_COMPILER_LIBRARY} ${DX_GUID_LIBRARY} ${DX_ERR_LIBRARY}
    )
    
    IF( D3D10_INCLUDE_DIR AND D3D10_LIBRARY )
        SET( D3D10_FOUND TRUE )
    ENDIF( D3D10_INCLUDE_DIR AND D3D10_LIBRARY )
    
    IF( D3D11_INCLUDE_DIR AND D3D11_LIBRARY )
        SET( D3D11_FOUND TRUE )
    ENDIF( D3D11_INCLUDE_DIR AND D3D11_LIBRARY )
    
    IF( DINPUT_INCLUDE_DIR AND DINPUT_LIBRARY )
        SET( DINPUT_FOUND TRUE )
    ENDIF( DINPUT_INCLUDE_DIR AND DINPUT_LIBRARY )
    
    IF( DSOUND_INCLUDE_DIR AND DSOUND_LIBRARY )
        SET( DSOUND_FOUND TRUE )
    ENDIF( DSOUND_INCLUDE_DIR AND DSOUND_LIBRARY )
ENDIF( WIN32 )
