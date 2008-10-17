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
	KlayGE::SceneManagerPtr const & OCTreeFactoryInstance(boost::program_options::variables_map const & vm)
	{
		static KlayGE::SceneManagerPtr sm = KlayGE::MakeSharedPtr<KlayGE::OCTree>(vm["octree.depth"].as<int>());
		return sm;
	}	

	std::string const & Name()
	{
		static std::string const name("OCTree");
		return name;
	}
}
