/**
 * @file Operators.hpp
 * @author Minmin Gong
 */

#pragma once

#include <KFL/Util.hpp>

#define DEFAULT_BINARY_OPERATOR1(T, OP) \
friend T operator OP(T const& lhs, T const& rhs) noexcept \
{ \
	T temp(lhs); \
	temp OP##= rhs; \
	return temp; \
} \

#define DEFAULT_BINARY_OPERATOR2(T, U, OP) \
friend T operator OP(T const& lhs, U const& rhs) noexcept \
{ \
	T temp(lhs); \
	temp OP##= rhs; \
	return temp; \
} \

#define DEFAULT_BINARY_OPERATOR3(U, T, OP) \
friend T operator OP(U const& lhs, T const& rhs) noexcept \
{ \
	return rhs OP lhs; \
} \

#define DEFAULT_ADD_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), +)
#define DEFAULT_ADD_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), +)
#define DEFAULT_ADD_OPERATOR3(U, T) DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), +)

#define DEFAULT_SUB_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), -)
#define DEFAULT_SUB_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), -)

#define DEFAULT_MUL_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), *)
#define DEFAULT_MUL_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), *)
#define DEFAULT_MUL_OPERATOR3(U, T) DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), *)

#define DEFAULT_DIV_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), /)
#define DEFAULT_DIV_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), /)

#define DEFAULT_AND_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), &)
#define DEFAULT_AND_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), &)
#define DEFAULT_AND_OPERATOR3(U, T) DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), &)

#define DEFAULT_OR_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), |)
#define DEFAULT_OR_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), |)
#define DEFAULT_OR_OPERATOR3(U, T) DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), |)

#define DEFAULT_XOR_OPERATOR1(T) DEFAULT_BINARY_OPERATOR1(KLAYGE_ESC(T), ^)
#define DEFAULT_XOR_OPERATOR2(T, U) DEFAULT_BINARY_OPERATOR2(KLAYGE_ESC(T), KLAYGE_ESC(U), ^)
#define DEFAULT_XOR_OPERATOR3(U, T) DEFAULT_BINARY_OPERATOR3(KLAYGE_ESC(U), KLAYGE_ESC(T), ^)

#define DEFAULT_LESS_COMPARE_OPERATOR(T) \
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
} \

#define DEFAULT_EQUALITY_COMPARE_OPERATOR(T) \
friend bool operator!=(T const& lhs, T const& rhs) noexcept \
{ \
	return !(lhs == rhs); \
} \

