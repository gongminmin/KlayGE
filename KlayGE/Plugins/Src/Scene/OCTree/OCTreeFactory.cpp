// OCTreeFactory.cpp
// KlayGE OCTree���������� ʵ���ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ���ν��� (2008.10.17)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/OCTree/OCTree.hpp>
#include <KlayGE/OCTree/OCTreeFactory.hpp>

void MakeSceneManager(std::unique_ptr<KlayGE::SceneManager>& ptr)
{
	ptr = KlayGE::MakeUniquePtr<KlayGE::OCTree>();
}
