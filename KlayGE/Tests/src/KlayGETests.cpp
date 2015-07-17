#include <KlayGE/KlayGE.hpp>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable" // Ignore some unused variables in unit_test.hpp
#endif
#define BOOST_TEST_MODULE KlayGETests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif
