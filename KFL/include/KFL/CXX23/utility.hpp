/**
 * @file utility.hpp
 * @author Minmin Gong
 */

#pragma once

#include <KFL/Config.hpp>

#if defined(KLAYGE_CXX23_LIBRARY_TO_UNDERLYING_SUPPORT)
#include <utility>
#else
#include <type_traits>

namespace std
{
	template <typename T>
	constexpr std::underlying_type_t<T> to_underlying(T e) noexcept
	{
		return static_cast<std::underlying_type_t<T>>(e);
	}
} // namespace std
#endif

#if defined(KLAYGE_CXX23_LIBRARY_UNREACHABLE_SUPPORT)
#include <utility>
#else
namespace std
{
	[[noreturn]] inline void unreachable()
	{
#if defined(KLAYGE_COMPILER_MSVC)
		__assume(false);
#else
		__builtin_unreachable();
#endif
	}
} // namespace std
#endif
