// D3D9OcclusionQuery.hpp
// KlayGE D3D9遮挡检测类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 初次建立 (2005.9.27)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d9.h>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>
#include <KlayGE/D3D9/D3D9OcclusionQuery.hpp>

namespace KlayGE
{
	D3D9OcclusionQuery::D3D9OcclusionQuery()
	{
		D3D9RenderEngine& render_eng(*checked_cast<D3D9RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance()));
		ID3D9DevicePtr d3d_device = render_eng.D3DDevice();

		IDirect3DQuery9* query;
		d3d_device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &query);
		query_ = MakeCOMPtr(query);
	}

	void D3D9OcclusionQuery::Begin()
	{
		TIF(query_->Issue(D3DISSUE_BEGIN));
	}

	void D3D9OcclusionQuery::End()
	{
		TIF(query_->Issue(D3DISSUE_END));
	}

	uint32_t D3D9OcclusionQuery::SamplesPassed()
	{
		uint32_t ret;
		TIF(query_->GetData(&ret, sizeof(ret), D3DGETDATA_FLUSH));
		return ret;
	}
}
