// RenderView.cpp
// KlayGE ��Ⱦ��ͼ�� ʵ���ļ�
// Ver 3.3.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.3.0
// ���ν��� (2006.5.31)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/RenderView.hpp>

namespace KlayGE
{
	void RenderView::OnBind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
	
	void RenderView::OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}


	void UnorderedAccessView::OnBind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
	
	void UnorderedAccessView::OnUnbind(FrameBuffer& /*fb*/, uint32_t /*att*/)
	{
	}
}
