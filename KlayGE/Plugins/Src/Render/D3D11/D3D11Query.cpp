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
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Fence.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11Query.hpp>

namespace KlayGE
{
	D3D11Query::D3D11Query()
	{
		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const& re = checked_cast<D3D11RenderEngine const&>(rf.RenderEngineInstance());

		auto* d3d_device_5 = re.D3DDevice5();
		if (d3d_device_5 != nullptr)
		{
			fence_ = rf.MakeFence();
		}
	}

	D3D11Query::~D3D11Query() noexcept = default;

	void D3D11Query::SignalFence()
	{
		if (fence_)
		{
			fence_val_ = fence_->Signal(Fence::FT_Render);
		}
	}

	void D3D11Query::WaitForFence()
	{
		if (fence_)
		{
			fence_->Wait(fence_val_);
		}
	}


	D3D11OcclusionQuery::D3D11OcclusionQuery()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_OCCLUSION;
		desc.MiscFlags = 0;
		d3d_device->CreateQuery(&desc, query_.put());
	}

	void D3D11OcclusionQuery::Begin()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->Begin(query_.get());
	}

	void D3D11OcclusionQuery::End()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->End(query_.get());

		this->SignalFence();
	}

	uint64_t D3D11OcclusionQuery::SamplesPassed()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		this->WaitForFence();

		uint64_t ret;
		while (S_OK != d3d_imm_ctx->GetData(query_.get(), &ret, sizeof(ret), 0));
		return ret;
	}


	D3D11ConditionalRender::D3D11ConditionalRender()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;
		desc.MiscFlags = 0;
		d3d_device->CreatePredicate(&desc, predicate_.put());
	}

	void D3D11ConditionalRender::Begin()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->Begin(predicate_.get());
	}

	void D3D11ConditionalRender::End()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->End(predicate_.get());

		this->SignalFence();
	}

	void D3D11ConditionalRender::BeginConditionalRender()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->SetPredication(predicate_.get(), false);
	}

	void D3D11ConditionalRender::EndConditionalRender()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->SetPredication(nullptr, false);
	}

	bool D3D11ConditionalRender::AnySamplesPassed()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		this->WaitForFence();

		BOOL ret;
		while (S_OK != d3d_imm_ctx->GetData(predicate_.get(), &ret, sizeof(ret), 0));
		return (TRUE == ret);
	}


	D3D11TimerQuery::D3D11TimerQuery()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();

		D3D11_QUERY_DESC disjoint_desc;
		disjoint_desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		disjoint_desc.MiscFlags = 0;
		d3d_device->CreateQuery(&disjoint_desc, timestamp_disjoint_query_.put());

		D3D11_QUERY_DESC timestamp_desc;
		timestamp_desc.Query = D3D11_QUERY_TIMESTAMP;
		timestamp_desc.MiscFlags = 0;
		d3d_device->CreateQuery(&timestamp_desc, timestamp_start_query_.put());
		d3d_device->CreateQuery(&timestamp_desc, timestamp_end_query_.put());
	}

	void D3D11TimerQuery::Begin()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->Begin(timestamp_disjoint_query_.get());
		d3d_imm_ctx->End(timestamp_start_query_.get());
	}

	void D3D11TimerQuery::End()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->End(timestamp_end_query_.get());
		d3d_imm_ctx->End(timestamp_disjoint_query_.get());

		this->SignalFence();
	}

	double D3D11TimerQuery::TimeElapsed()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		this->WaitForFence();

		uint64_t start_timestamp, end_timestamp;
		while (S_OK != d3d_imm_ctx->GetData(timestamp_start_query_.get(), &start_timestamp, sizeof(start_timestamp), 0));
		while (S_OK != d3d_imm_ctx->GetData(timestamp_end_query_.get(), &end_timestamp, sizeof(end_timestamp), 0));

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
		while (S_OK != d3d_imm_ctx->GetData(timestamp_disjoint_query_.get(), &disjoint, sizeof(disjoint), 0));

		return disjoint.Disjoint ? -1 : static_cast<double>(end_timestamp - start_timestamp) / disjoint.Frequency;
	}


	D3D11SOStatisticsQuery::D3D11SOStatisticsQuery()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11Device1* d3d_device = re.D3DDevice1();

		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_SO_STATISTICS;
		desc.MiscFlags = 0;
		d3d_device->CreateQuery(&desc, so_stat_query_.put());
	}

	void D3D11SOStatisticsQuery::Begin()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->Begin(so_stat_query_.get());
	}

	void D3D11SOStatisticsQuery::End()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		d3d_imm_ctx->End(so_stat_query_.get());

		this->SignalFence();
	}

	uint64_t D3D11SOStatisticsQuery::NumPrimitivesWritten()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		this->WaitForFence();

		D3D11_QUERY_DATA_SO_STATISTICS data;
		while (S_OK != d3d_imm_ctx->GetData(so_stat_query_.get(), &data, sizeof(data), 0));
		return data.NumPrimitivesWritten;
	}

	uint64_t D3D11SOStatisticsQuery::PrimitivesGenerated()
	{
		auto const& re = checked_cast<D3D11RenderEngine const&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContext1* d3d_imm_ctx = re.D3DDeviceImmContext1();

		this->WaitForFence();

		D3D11_QUERY_DATA_SO_STATISTICS data;
		while (S_OK != d3d_imm_ctx->GetData(so_stat_query_.get(), &data, sizeof(data), 0));
		return data.PrimitivesStorageNeeded;
	}
}
