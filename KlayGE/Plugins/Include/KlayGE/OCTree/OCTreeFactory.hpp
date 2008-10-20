// OCTreeFactory.hpp
// KlayGE OCTree场景管理类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.10.17)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OCTREEFACTORY_HPP
#define _OCTREEFACTORY_HPP

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4251 4275 4512 4702)
#endif
#include <boost/program_options.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_OCTREE_SM_SOURCE				// Build dll
		#define KLAYGE_OCTREE_SM_API __declspec(dllexport)
	#else										// Use dll
		#define KLAYGE_OCTREE_SM_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_OCTREE_SM_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_OCTREE_SM_API void MakeSceneManager(KlayGE::SceneManagerPtr& ptr, boost::program_options::variables_map const & vm);
	KLAYGE_OCTREE_SM_API bool Match(std::string const & name, std::string const & compiler);
}

#endif			// _OCTREEFACTORY_HPP
