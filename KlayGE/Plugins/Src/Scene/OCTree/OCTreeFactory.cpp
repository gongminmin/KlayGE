// OCTreeFactory.cpp
// KlayGE OCTree场景管理类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2008.10.17)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeSceneManager(std::unique_ptr<KlayGE::SceneManager>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::OCTree>();
	}
}
