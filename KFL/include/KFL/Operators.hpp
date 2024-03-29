/**
 * @file Operators.hpp
 * @author Minmin Gong
 */

#pragma once

#include <KFL/Util.hpp>

#define KLAYGE_DEFAULT_BINARY_OPERATOR1(T, OP) \
friend T operator OP(T const& lhs, T const& rhs) noexcept \
{ \
	T temp(lhs); \
	temp OP##= rhs; \
	return temp; \
}

#define KLAYGE_DEFAULT_BINARY_OPERATOR2(T, U, OP) \
friend T operator OP(T const& lhs, U const& rhs) noexcept \
{ \
	T temp(lhs); \
	temp OP##= rhs; \
	return temp; \
}

#define KLAYGE_DEFAULT_BINARY_OPERATOR3(U, T, OP) \
friend T operator OP(U const& lhs, T const& rhs) noexcept \
{ \
	return rhs OP lhs; \
}

#define KLAYGE_DEFAULT_ADD_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), +)
#define KLAYGE_DEFAULT_ADD_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), +)
#define KLAYGE_DEFAULT_ADD_OPERATOR3(U, T) KLAYGE_DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), +)

#define KLAYGE_DEFAULT_SUB_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), -)
#define KLAYGE_DEFAULT_SUB_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), -)

#define KLAYGE_DEFAULT_MUL_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), *)
#define KLAYGE_DEFAULT_MUL_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), *)
#define KLAYGE_DEFAULT_MUL_OPERATOR3(U, T) KLAYGE_DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), *)

#define KLAYGE_DEFAULT_DIV_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), /)
#define KLAYGE_DEFAULT_DIV_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), /)

#define KLAYGE_DEFAULT_AND_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), &)
#define KLAYGE_DEFAULT_AND_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), &)
#define KLAYGE_DEFAULT_AND_OPERATOR3(U, T) KLAYGE_DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), &)

#define KLAYGE_DEFAULT_OR_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), |)
#define KLAYGE_DEFAULT_OR_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), |)
#define KLAYGE_DEFAULT_OR_OPERATOR3(U, T) KLAYGE_DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), |)

#define KLAYGE_DEFAULT_XOR_OPERATOR1(T) KLAYGE_DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), ^)
#define KLAYGE_DEFAULT_XOR_OPERATOR2(T, U) KLAYGE_DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), ^)
#define KLAYGE_DEFAULT_XOR_OPERATOR3(U, T) KLAYGE_DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), ^)

#define KLAYGE_DEFAULT_LESS_COMPARE_OPERATOR(T) \
friend bool operator>(T const& lhs, T const& rhs) noexcept \
{ \
	return rhs < lhs; \
} \
friend bool operator<=(T const& lhs, T const& rhs) noexcept \
{ \
	return !(rhs < lhs); \
} \
friend bool operator>=(T const& lhs, T const& rhs) noexcept \
{ \
	return !(lhs < rhs); \
}

#define KLAYGE_DEFAULT_EQUALITY_COMPARE_OPERATOR(T) \
friend bool operator!=(T const& lhs, T const& rhs) noexcept \
{ \
	return !(lhs == rhs); \
}
