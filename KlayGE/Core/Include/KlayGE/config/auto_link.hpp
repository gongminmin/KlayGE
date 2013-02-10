#ifndef KLAYGE_LIB_NAME
	#error "Macro KLAYGE_LIB_NAME not set (internal error)"
#endif

#ifdef KLAYGE_COMPILER_MSVC
	#ifdef KLAYGE_DEBUG
		#define DEBUG_SUFFIX "_d"
	#else
		#define DEBUG_SUFFIX ""
	#endif

	#define LIB_FILE_NAME KFL_STRINGIZE(KLAYGE_LIB_NAME)"_"KFL_STRINGIZE(KLAYGE_COMPILER_NAME) DEBUG_SUFFIX ".lib"

	#pragma comment(lib, LIB_FILE_NAME)
	//#pragma message("Linking to lib file: " LIB_FILE_NAME)
	#undef LIB_FILE_NAME
	#undef DEBUG_SUFFIX
#endif

#undef KLAYGE_LIB_NAME
