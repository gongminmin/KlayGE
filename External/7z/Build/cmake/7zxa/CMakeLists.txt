SET(LIB_NAME 7zxa)

SET(7ZIP_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/ICoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IDecl.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IPassword.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IProgress.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IStream.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/PropID.h
)
SET(7ZIP_COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CreateCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CWrappers.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/FilterCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/InBuffer.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LimitedStreams.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LockedStream.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodId.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/OutBuffer.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/ProgressUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamBinder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamObjects.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/VirtThread.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CreateCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CWrappers.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/FilterCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/InBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LimitedStreams.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LockedStream.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodId.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/OutBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/ProgressUtils.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/RegisterArc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/RegisterCodec.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamBinder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamObjects.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamUtils.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/VirtThread.h
)
SET(ARCHIVE_7Z_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zCompressionMode.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zDecode.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zExtract.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zFolderOutStream.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHandler.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHeader.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zIn.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zProperties.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zCompressionMode.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zDecode.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zFolderOutStream.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHandler.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHeader.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zIn.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zItem.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zProperties.h
)
SET(ARCHIVE_COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2MT.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2ST.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CrossThreadProgress.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/MultiStream.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ParseProperties.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2MT.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2ST.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CrossThreadProgress.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/MultiStream.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ParseProperties.h
)
SET(ARCHIVE_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/IArchive.h
)
SET(C_FILES
	${SEVENZIP_PROJECT_DIR}/C/7zCrc.c
	${SEVENZIP_PROJECT_DIR}/C/Aes.c
	${SEVENZIP_PROJECT_DIR}/C/Alloc.c
	${SEVENZIP_PROJECT_DIR}/C/Bra.c
	${SEVENZIP_PROJECT_DIR}/C/Bra86.c
	${SEVENZIP_PROJECT_DIR}/C/BraIA64.c
	${SEVENZIP_PROJECT_DIR}/C/CpuArch.c
	${SEVENZIP_PROJECT_DIR}/C/Delta.c
	${SEVENZIP_PROJECT_DIR}/C/Lzma2Dec.c
	${SEVENZIP_PROJECT_DIR}/C/LzmaDec.c
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7.c
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7Dec.c
	${SEVENZIP_PROJECT_DIR}/C/Sha256.c
	${SEVENZIP_PROJECT_DIR}/C/Threads.c
	${SEVENZIP_PROJECT_DIR}/C/7zCrc.h
	${SEVENZIP_PROJECT_DIR}/C/Aes.h
	${SEVENZIP_PROJECT_DIR}/C/Alloc.h
	${SEVENZIP_PROJECT_DIR}/C/Bra.h
	${SEVENZIP_PROJECT_DIR}/C/CpuArch.h
	${SEVENZIP_PROJECT_DIR}/C/Delta.h
	${SEVENZIP_PROJECT_DIR}/C/Lzma2Dec.h
	${SEVENZIP_PROJECT_DIR}/C/LzmaDec.h
	${SEVENZIP_PROJECT_DIR}/C/Ppmd.h
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7.h
	${SEVENZIP_PROJECT_DIR}/C/RotateDefs.h
	${SEVENZIP_PROJECT_DIR}/C/Sha256.h
	${SEVENZIP_PROJECT_DIR}/C/Threads.h
	${SEVENZIP_PROJECT_DIR}/C/Types.h
)
SET(COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/Common/CRC.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/IntToString.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyString.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyVector.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyWindows.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/NewHandler.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringConvert.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringToInt.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Wildcard.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Buffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/DynamicBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/IntToString.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyCom.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyException.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyInitGuid.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyString.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyUnknown.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyVector.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyWindows.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/NewHandler.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringConvert.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringToInt.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Types.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Wildcard.h
)
SET(7Z_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Coder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchMisc.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/ByteSwap.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Decoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Coder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchMisc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Decoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/RangeCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/RangeCoderBit.h
)
SET(BIT_CODER_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlEncoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitmDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitmEncoder.h
)
SET(BZIP2_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Crc.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Decoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Const.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Crc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Decoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Mtf8.h
)
SET(PPMD_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdDecoder.h
)
SET(ZIP_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateConst.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateDecoder.h
)
SET(COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeltaFilter.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzOutWindow.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/HuffmanDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzOutWindow.h
)
SET(CRYPTO_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAes.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAesRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/MyAes.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAes.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/MyAes.h
)
SET(SPEC_ICONS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/7z.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/arj.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/bz2.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/cab.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/cpio.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/deb.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/dmg.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/fat.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/gz.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/hfs.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/iso.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/lzh.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/lzma.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/ntfs.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/rar.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/rpm.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/split.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/squashfs.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/tar.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/vhd.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/wim.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/xar.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/xz.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/z.ico
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Icons/zip.ico
)
SET(SPEC_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Archive2.def
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/ArchiveExports.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CodecExports.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/DllExports2.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Bundles/Format7zExtract/resource.rc
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Bundles/Format7zExtract/StdAfx.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Bundles/Format7zExtract/StdAfx.h
)
SET(WINDOWS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/PropVariant.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/Synchronization.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/System.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/PropVariant.h
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/Synchronization.h
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/System.h
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/Thread.h
)

SOURCE_GROUP("7zip" FILES ${7ZIP_FILES})
SOURCE_GROUP("7zip Common" FILES ${7ZIP_COMMON_FILES})
SOURCE_GROUP("Archive\\7z" FILES ${ARCHIVE_7Z_FILES})
SOURCE_GROUP("Archive\\Archive common" FILES ${ARCHIVE_COMMON_FILES})
SOURCE_GROUP("Archive" FILES ${ARCHIVE_FILES})
SOURCE_GROUP("C" FILES ${C_FILES})
SOURCE_GROUP("Common" FILES ${COMMON_FILES})
SOURCE_GROUP("Compress\\7z Compress" FILES ${7Z_COMPRESS_FILES})
SOURCE_GROUP("Compress\\Bit Coder" FILES ${BIT_CODER_FILES})
SOURCE_GROUP("Compress\\BZip2 Compress" FILES ${BZIP2_COMPRESS_FILES})
SOURCE_GROUP("Compress\\PPMD" FILES ${PPMD_FILES})
SOURCE_GROUP("Compress\\Zip Compress" FILES ${ZIP_COMPRESS_FILES})
SOURCE_GROUP("Compress" FILES ${COMPRESS_FILES})
SOURCE_GROUP("Crypto" FILES ${CRYPTO_FILES})
SOURCE_GROUP("Spec\\Icons" FILES ${SPEC_ICONS_FILES})
SOURCE_GROUP("Spec" FILES ${SPEC_FILES})
SOURCE_GROUP("Windows" FILES ${WINDOWS_FILES})

ADD_DEFINITIONS(-DNO_ASM)
ADD_DEFINITIONS(-DMY7Z_EXPORTS)
ADD_DEFINITIONS(-DNO_REGISTRY)
ADD_DEFINITIONS(-DEXTRACT_ONLY)
ADD_DEFINITIONS(-D_7ZIP_LARGE_PAGES)
ADD_DEFINITIONS(-D_7ZIP_ST)

INCLUDE_DIRECTORIES(${SEVENZIP_PROJECT_DIR}/CPP/)

ADD_LIBRARY(${LIB_NAME} ${KLAYGE_PREFERRED_LIB_TYPE}
	${7ZIP_FILES} ${7ZIP_COMMON_FILES} ${ARCHIVE_7Z_FILES} ${ARCHIVE_COMMON_FILES}
	${ARCHIVE_FILES} ${C_FILES} ${COMMON_FILES} ${7Z_COMPRESS_FILES} ${BIT_CODER_FILES}
	${BZIP2_COMPRESS_FILES} ${PPMD_FILES} ${ZIP_COMPRESS_FILES} ${COMPRESS_FILES}
	${CRYPTO_FILES} ${SPEC_ICONS_FILES} ${SPEC_FILES} ${WINDOWS_FILES}
)
	
SET(LIB_OUTPUT_NAME ${LIB_NAME})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY ${SEVENZIP_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${SEVENZIP_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${SEVENZIP_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${SEVENZIP_OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${SEVENZIP_OUTPUT_DIR}
	PROJECT_LABEL ${LIB_NAME}
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	OUTPUT_NAME ${LIB_OUTPUT_NAME}${KLAYGE_OUTPUT_SUFFIX}
)
IF(KLAYGE_PLATFORM_DARWIN OR KLAYGE_PLATFORM_LINUX)
	SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ${KLAYGE_BIN_DIR}
		LIBRARY_OUTPUT_DIRECTORY_DEBUG ${KLAYGE_BIN_DIR}
		LIBRARY_OUTPUT_DIRECTORY_RELEASE ${KLAYGE_BIN_DIR}
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${KLAYGE_BIN_DIR}
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${KLAYGE_BIN_DIR})
ENDIF()

IF(KLAYGE_COMPILER_MSVC)
	ADD_PRECOMPILED_HEADER(${LIB_NAME} "StdAfx.h" "${SEVENZIP_PROJECT_DIR}/CPP/7zip/Bundles/Format7zExtract/"
		"${SEVENZIP_PROJECT_DIR}/CPP/7zip/Bundles/Format7zExtract/StdAfx.cpp")

	SET_SOURCE_FILES_PROPERTIES(${C_FILES} PROPERTIES COMPILE_FLAGS "/Y-")
ENDIF()

TARGET_LINK_LIBRARIES(${LIB_NAME})

IF(KLAYGE_PLATFORM_WINDOWS)
	INSTALL(TARGETS ${LIB_NAME}
		RUNTIME DESTINATION ${KLAYGE_BIN_DIR}
		LIBRARY DESTINATION ${KLAYGE_BIN_DIR}
	)
ENDIF()
