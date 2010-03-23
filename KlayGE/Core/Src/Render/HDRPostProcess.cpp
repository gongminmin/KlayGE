// HDRPostProcess.cpp
// KlayGE HDR后期处理类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.8.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>

#include <boost/assert.hpp>

#include <KlayGE/PostProcess.hpp>
#include <KlayGE/HDRPostProcess.hpp>

namespace KlayGE
{
	SumLumPostProcess::SumLumPostProcess(RenderTechniquePtr const & tech)
		: PostProcess(std::vector<std::string>(1, "src_tex"), std::vector<std::string>(1, "output"), tech)
	{
		tex_coord_offset_ep_ = technique_->Effect().ParameterByName("tex_coord_offset");
	}

	SumLumPostProcess::~SumLumPostProcess()
	{
	}

	void SumLumPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		PostProcess::InputPin(index, tex);
		this->GetSampleOffsets4x4(tex->Width(0), tex->Height(0));
	}

	void SumLumPostProcess::GetSampleOffsets4x4(uint32_t width, uint32_t height)
	{
		std::vector<float4> tex_coord_offset(2);

		float const tu = 1.0f / width;
		float const tv = 1.0f / height;

		// Sample from the 16 surrounding points.
		int index = 0;
		for (int y = -1; y <= 2; y += 2)
		{
			for (int x = -1; x <= 2; x += 4)
			{
				tex_coord_offset[index].x() = (x + 0) * tu;
				tex_coord_offset[index].y() = y * tv;
				tex_coord_offset[index].z() = (x + 2) * tu;
				tex_coord_offset[index].w() = y * tv;

				++ index;
			}
		}

		*tex_coord_offset_ep_ = tex_coord_offset;
	}


	SumLumLogPostProcess::SumLumLogPostProcess()
			: SumLumPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("SumLumLog"))
	{
	}


	SumLumLogPostProcessCS::SumLumLogPostProcessCS()
			: SumLumPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLumCS.fxml")->TechniqueByName("SumLumLog"))
	{
		buff_ = Context::Instance().RenderFactoryInstance().MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, NULL, EF_R32F);
		*(technique_->Effect().ParameterByName("out_buff")) = buff_;
	}

	void SumLumLogPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		this->OnRenderBegin();
		re.Dispatch(*technique_, buff_->Size() / sizeof(float), 1, 1);
		this->OnRenderEnd();
	}

	void SumLumLogPostProcessCS::InputPin(uint32_t index, TexturePtr const & tex)
	{
		SumLumPostProcess::InputPin(index, tex);
		*(technique_->Effect().ParameterByName("src_tex_dim")) = Vector_T<int32_t, 2>(tex->Width(0), tex->Height(0));
	}

	void SumLumLogPostProcessCS::DestinateSize(uint32_t width, uint32_t height)
	{
		buff_->Resize(width * height / (256 * 4) * sizeof(float));
		*(technique_->Effect().ParameterByName("dst_tex_dim")) = Vector_T<int32_t, 2>(width, height);
	}


	SumLumIterativePostProcess::SumLumIterativePostProcess()
			: SumLumPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("SumLumIterative"))
	{
	}


	AdaptedLumPostProcess::AdaptedLumPostProcess()
			: PostProcess(std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "output"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("AdaptedLum")),
				last_index_(false)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		std::vector<float> data_v(4, 0);
		ElementInitData init_data;
		init_data.row_pitch = sizeof(float);
		init_data.slice_pitch = 0;
		init_data.data = &data_v[0];
		try
		{
			adapted_textures_[0] = rf.MakeTexture2D(1, 1, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
			adapted_textures_[1] = rf.MakeTexture2D(1, 1, 1, 1, EF_R32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
			this->OutputPin(0, adapted_textures_[last_index_]);
		}
		catch (...)
		{
			init_data.row_pitch = 4 * sizeof(float);
			adapted_textures_[0] = rf.MakeTexture2D(1, 1, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
			adapted_textures_[1] = rf.MakeTexture2D(1, 1, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, &init_data);
			this->OutputPin(0, adapted_textures_[last_index_]);
		}

		last_lum_tex_ep_ = technique_->Effect().ParameterByName("last_lum_tex");
		frame_delta_ep_ = technique_->Effect().ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcess::Apply()
	{
		this->OutputPin(0, adapted_textures_[!last_index_]);

		PostProcess::Apply();
	}

	void AdaptedLumPostProcess::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*last_lum_tex_ep_ = adapted_textures_[last_index_];
		*frame_delta_ep_ = float(timer_.elapsed());
		timer_.restart();

		last_index_ = !last_index_;
	}


	AdaptedLumPostProcessCS::AdaptedLumPostProcessCS()
			: PostProcess(std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "output"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SumLumCS.fxml")->TechniqueByName("AdaptedLum"))
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		float data_v = 0;
		ElementInitData init_data;
		init_data.row_pitch = sizeof(float);
		init_data.slice_pitch = 0;
		init_data.data = &data_v;
		adapted_buff_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_R32F);
		*(technique_->Effect().ParameterByName("out_buff")) = adapted_buff_;

		frame_delta_ep_ = technique_->Effect().ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		this->OnRenderBegin();
		re.Dispatch(*technique_, 1, 1, 1);
		this->OnRenderEnd();
	}

	void AdaptedLumPostProcessCS::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*frame_delta_ep_ = float(timer_.elapsed());
		timer_.restart();
	}

	void AdaptedLumPostProcessCS::SetLumBuff(GraphicsBufferPtr const & lum_buff)
	{
		*(technique_->Effect().ParameterByName("src_buff")) = lum_buff;
	}

	GraphicsBufferPtr const & AdaptedLumPostProcessCS::AdaptedLum() const
	{
		return adapted_buff_;
	}


	ToneMappingPostProcess::ToneMappingPostProcess(bool blue_shift)
	{
		input_pins_.push_back(std::make_pair("src_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("lum_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("bloom_tex", TexturePtr()));
		
		RenderEffectPtr effect = Context::Instance().RenderFactoryInstance().LoadEffect("ToneMapping.fxml");
		technique_ = effect->TechniqueByName("ToneMapping50");
		if (Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().cs_support
			&& technique_->Validate())
		{
			lum_buff_ep_ = technique_->Effect().ParameterByName("lum_buff");
		}
		else
		{
			technique_ = effect->TechniqueByName("ToneMapping30");
			if (!technique_->Validate())
			{
				technique_ = effect->TechniqueByName("ToneMapping20");
			}
		}

		this->Technique(technique_);

		*(technique_->Effect().ParameterByName("blue_shift")) = blue_shift;
	}

	void ToneMappingPostProcess::SetLumBuff(GraphicsBufferPtr const & lum_buff)
	{
		*lum_buff_ep_ = lum_buff;
	}


	HDRPostProcess::HDRPostProcess(bool bright_pass, bool blue_shift)
		: cs_support_(Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().cs_support)
	{
		if (bright_pass)
		{
			downsampler_ = MakeSharedPtr<BrightPassDownsampler2x2PostProcess>();
		}
		else
		{
			downsampler_ = MakeSharedPtr<Downsampler2x2PostProcess>();
		}

		blur_ = MakeSharedPtr<BlurPostProcess<SeparableGaussianFilterPostProcess> >(8, 2.0f);
		if (cs_support_)
		{
			sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcessCS>();
			sum_lums_.resize(0);
			adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcessCS>();
		}
		else
		{
			sum_lums_1st_ = MakeSharedPtr<SumLumLogPostProcess>();
			sum_lums_.resize(NUM_TONEMAP_TEXTURES);
			for (size_t i = 0; i < sum_lums_.size(); ++ i)
			{
				sum_lums_[i] = MakeSharedPtr<SumLumIterativePostProcess>();
			}
			adapted_lum_ = MakeSharedPtr<AdaptedLumPostProcess>();
		}
		tone_mapping_ = MakeSharedPtr<ToneMappingPostProcess>(blue_shift);
	}

	void HDRPostProcess::InputPin(uint32_t index, TexturePtr const & tex)
	{
		uint32_t const width = tex->Width(0);
		uint32_t const height = tex->Height(0);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		downsample_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, NULL);
		{
			downsampler_->InputPin(index, tex);
			downsampler_->OutputPin(index, downsample_tex_);
		}

		blur_tex_ = rf.MakeTexture2D(width / 4, height / 4, 1, 1, EF_ABGR16F, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, NULL);
		{
			blur_->InputPin(index, downsample_tex_);
			blur_->OutputPin(index, blur_tex_);
		}

		if (cs_support_)
		{
			sum_lums_1st_->InputPin(index, tex);
			checked_pointer_cast<SumLumLogPostProcessCS>(sum_lums_1st_)->DestinateSize(64, 64);
			checked_pointer_cast<AdaptedLumPostProcessCS>(adapted_lum_)->SetLumBuff(checked_pointer_cast<SumLumLogPostProcessCS>(sum_lums_1st_)->SumLumBuff());
			checked_pointer_cast<ToneMappingPostProcess>(tone_mapping_)->SetLumBuff(checked_pointer_cast<AdaptedLumPostProcessCS>(adapted_lum_)->AdaptedLum());
		}
		else
		{
			lum_texs_.resize(sum_lums_.size() + 1);
			try
			{
				int len = 1;
				for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
				{
					lum_texs_[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, EF_R16F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, NULL);
					len *= 4;
				}

				{
					sum_lums_1st_->InputPin(index, tex);
					sum_lums_1st_->OutputPin(index, lum_texs_[0]);
				}
				for (size_t i = 0; i < sum_lums_.size(); ++ i)
				{
					sum_lums_[i]->InputPin(index, lum_texs_[i]);
					sum_lums_[i]->OutputPin(index, lum_texs_[i + 1]);
				}
			}
			catch (...)
			{
				int len = 1;
				for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
				{
					lum_texs_[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, EF_ABGR16F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, NULL);
					len *= 4;
				}

				{
					sum_lums_1st_->InputPin(index, tex);
					sum_lums_1st_->OutputPin(index, lum_texs_[0]);
				}
				for (size_t i = 0; i < sum_lums_.size(); ++ i)
				{
					sum_lums_[i]->InputPin(index, lum_texs_[i]);
					sum_lums_[i]->OutputPin(index, lum_texs_[i + 1]);
				}
			}

			{
				adapted_lum_->InputPin(index, lum_texs_[sum_lums_.size()]);
			}
		}

		tone_mapping_->InputPin(0, tex);
		tone_mapping_->InputPin(2, blur_tex_);
	}

	TexturePtr const & HDRPostProcess::InputPin(uint32_t index) const
	{
		return downsampler_->InputPin(index);
	}

	void HDRPostProcess::OutputPin(uint32_t index, TexturePtr const & tex)
	{
		tone_mapping_->OutputPin(index, tex);
	}

	TexturePtr const & HDRPostProcess::OutputPin(uint32_t index) const
	{
		return tone_mapping_->OutputPin(index);
	}

	void HDRPostProcess::Apply()
	{
		// 降采样
		downsampler_->Apply();
		// Blur
		blur_->Apply();

		// 降采样4x4 log
		sum_lums_1st_->Apply();
		for (size_t i = 0; i < sum_lums_.size(); ++ i)
		{
			// 降采样4x4
			sum_lums_[i]->Apply();
		}

		adapted_lum_->Apply();

		{
			// Tone mapping
			if (!cs_support_)
			{
				tone_mapping_->InputPin(1, adapted_lum_->OutputPin(0));
			}
			tone_mapping_->Apply();
		}
	}
}
