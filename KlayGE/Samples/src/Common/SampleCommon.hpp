#ifdef KLAYGE_COMPILER_MSVC
	#ifdef KLAYGE_DEBUG
		#define DEBUG_SUFFIX "_d"
	#else
		#define DEBUG_SUFFIX ""
	#endif

	#define LIB_FILE_NAME "SampleCommon_"KFL_STRINGIZE(KLAYGE_COMPILER_NAME)"_"KFL_STRINGIZE(KLAYGE_COMPILER_TARGET) DEBUG_SUFFIX ".lib"

	#pragma comment(lib, LIB_FILE_NAME)

	#undef LIB_FILE_NAME
	#undef DEBUG_SUFFIX
#endif

int SampleMain();
