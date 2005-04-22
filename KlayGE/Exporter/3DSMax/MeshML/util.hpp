#ifndef _UTIL_HPP
#define _UTIL_HPP

#include <string>

std::string tstr_to_str(std::basic_string<TCHAR> const & tstr);

bool is_mesh(INode* node);
bool is_bone(INode* node);

#endif		// _UTIL_HPP
