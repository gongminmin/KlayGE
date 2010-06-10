// D3D11Query.hpp
// KlayGE D3D11查询类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://www.klayge.org
//
// 3.8.0
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>

namespace KlayGE
{
	D3D11OcclusionQuery::D3D11OcclusionQuery()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DevicePtr const & d3d_device = re.D3DDevice();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_OCCLUSION;
		desc.MiscFlags = 0;

		ID3D11Query* query;
		d3d_device->CreateQuery(&desc, &query);
		query_ = MakeCOMPtr(query);
	}

	void D3D11OcclusionQuery::Begin()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->Begin(query_.get());
	}

	void D3D11OcclusionQuery::End()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->End(query_.get());
	}

	uint64_t D3D11OcclusionQuery::SamplesPassed()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		uint64_t ret;
		while (S_OK != d3d_imm_ctx->GetData(query_.get(), &ret, sizeof(ret), 0));
		return ret;
	}


	D3D11ConditionalRender::D3D11ConditionalRender()
	{
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DevicePtr const & d3d_device = render_eng.D3DDevice();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;
		desc.MiscFlags = 0;

		ID3D11Predicate* predicate;
		d3d_device->CreatePredicate(&desc, &predicate);
		predicate_ = MakeCOMPtr(predicate);
	}

	void D3D11ConditionalRender::Begin()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->Begin(predicate_.get());
	}

	void D3D11ConditionalRender::End()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->End(predicate_.get());
	}

	void D3D11ConditionalRender::BeginConditionalRender()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->SetPredication(predicate_.get(), false);
	}

	void D3D11ConditionalRender::EndConditionalRender()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		d3d_imm_ctx->SetPredication(NULL, false);
	}

	bool D3D11ConditionalRender::AnySamplesPassed()
	{
		D3D11RenderEngine const & re = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		BOOL ret;
		while (S_OK != d3d_imm_ctx->GetData(predicate_.get(), &ret, sizeof(ret), 0));
		return (TRUE == ret);
	}
}
