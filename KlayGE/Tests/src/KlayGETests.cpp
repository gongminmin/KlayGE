#include <KlayGE/KlayGE.hpp>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable" // Ignore some unused variables in unit_test.hpp
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter in boost
#endif
#define BOOST_TEST_MODULE KlayGETests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif
