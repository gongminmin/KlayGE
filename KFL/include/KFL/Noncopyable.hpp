/**
 * @file Noncopyable.hpp
 * @author Minmin Gong
 */

#pragma once

#define KLAYGE_NONCOPYABLE(T) \
T(T const& rhs) = delete; \
T& operator=(T const& rhs) = delete;

#define KLAYGE_NONMOVEABLE(T) \
T(T&& rhs) = delete; \
T& operator=(T&& rhs) = delete;
