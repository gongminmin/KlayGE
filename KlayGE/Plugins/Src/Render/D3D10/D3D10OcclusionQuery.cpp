// D3D10OcclusionQuery.hpp
// KlayGE D3D10遮挡检测类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d10.h>

#include <KlayGE/D3D10/D3D10RenderEngine.hpp>
#include <KlayGE/D3D10/D3D10OcclusionQuery.hpp>

namespace KlayGE
{
	D3D10OcclusionQuery::D3D10OcclusionQuery()
	{
		D3D10RenderEngine const & render_eng = *checked_cast<D3D10RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D10DevicePtr const & d3d_device = render_eng.D3DDevice();

		D3D10_QUERY_DESC desc;
		desc.Query = D3D10_QUERY_OCCLUSION;
		desc.MiscFlags = 0;

		ID3D10Query* query;
		d3d_device->CreateQuery(&desc, &query);
		query_ = MakeCOMPtr(query);
	}

	void D3D10OcclusionQuery::Begin()
	{
		query_->Begin();
	}

	void D3D10OcclusionQuery::End()
	{
		query_->End();
	}

	uint64_t D3D10OcclusionQuery::SamplesPassed()
	{
		uint64_t ret;
		while (S_OK != query_->GetData(&ret, sizeof(ret), 0));
		return ret;
	}
}
