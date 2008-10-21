// OCTreeFactory.cpp
// KlayGE OCTree场景管理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.10.17)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/OCTree/OCTree.hpp>
#include <KlayGE/OCTree/OCTreeFactory.hpp>

extern "C"
{
	void MakeSceneManager(KlayGE::SceneManagerPtr& ptr, boost::program_options::variables_map const & vm)
	{
		ptr = KlayGE::MakeSharedPtr<KlayGE::OCTree>(vm["octree.depth"].as<int>());
	}	

	bool Match(char const * name, char const * compiler)
	{
		std::string cur_compiler_str = KLAYGE_COMPILER_TOOLSET;
#ifdef KLAYGE_DEBUG
		cur_compiler_str += "_d";
#endif

		if ((std::string("OCTree") == name) && (cur_compiler_str == compiler))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
