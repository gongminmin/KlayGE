#define KLAYGE_STRINGIZE(X) KLAYGE_DO_STRINGIZE(X)
#define KLAYGE_DO_STRINGIZE(X) #X

#ifndef KLAYGE_LIB_NAME
#error "Macro KLAYGE_LIB_NAME not set (internal error)"
#endif

#ifdef KLAYGE_COMPILER_MSVC

#ifdef KLAYGE_DEBUG
#define LIB_FILE_NAME KLAYGE_STRINGIZE(KLAYGE_LIB_NAME)"_d.lib"
#else
#define LIB_FILE_NAME KLAYGE_STRINGIZE(KLAYGE_LIB_NAME)".lib"
#endif

#pragma comment(lib, LIB_FILE_NAME)
#pragma message("Linking to lib file: " LIB_FILE_NAME)
#undef LIB_FILE_NAME

#endif

#undef KLAYGE_LIB_NAME
