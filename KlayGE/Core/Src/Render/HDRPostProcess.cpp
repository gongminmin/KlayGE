// HDRPostProcess.cpp
// KlayGE HDR后期处理类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
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
#include <KlayGE/ResLoader.hpp>
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
		: PostProcess(L"SumLum",
			std::vector<std::string>(),
			std::vector<std::string>(1, "src_tex"),
			std::vector<std::string>(1, "out_tex"),
			tech)
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

	TexturePtr const & SumLumPostProcess::InputPin(uint32_t index) const
	{
		return PostProcess::InputPin(index);
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
			: SumLumPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("SumLumLogCS"))
	{
	}

	void SumLumLogPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());

		*(technique_->Effect().ParameterByName("dst_tex_dim")) = int2(64, 64);

		this->OnRenderBegin();
		re.Dispatch(*technique_, 2, 2, 1);
		this->OnRenderEnd();
	}


	SumLumIterativePostProcess::SumLumIterativePostProcess()
			: SumLumPostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("SumLumIterative"))
	{
	}


	AdaptedLumPostProcess::AdaptedLumPostProcess()
			: PostProcess(L"AdaptedLum",
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
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
		*frame_delta_ep_ = static_cast<float>(timer_.elapsed());
		timer_.restart();

		last_index_ = !last_index_;
	}


	AdaptedLumPostProcessCS::AdaptedLumPostProcessCS()
			: PostProcess(L"AdaptedLumCS", 
					std::vector<std::string>(),
					std::vector<std::string>(1, "src_tex"),
					std::vector<std::string>(1, "out_tex"),
					Context::Instance().RenderFactoryInstance().LoadEffect("SumLum.fxml")->TechniqueByName("AdaptedLumCS"))
	{
		frame_delta_ep_ = technique_->Effect().ParameterByName("frame_delta");
	}

	void AdaptedLumPostProcessCS::Apply()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.BindFrameBuffer(re.DefaultFrameBuffer());

		this->OnRenderBegin();
		re.Dispatch(*technique_, 1, 1, 1);
		this->OnRenderEnd();
	}

	void AdaptedLumPostProcessCS::OnRenderBegin()
	{
		PostProcess::OnRenderBegin();

		*frame_delta_ep_ = static_cast<float>(timer_.elapsed());
		timer_.restart();
	}


	ToneMappingPostProcess::ToneMappingPostProcess(bool blue_shift)
		: PostProcess(L"ToneMapping")
	{
		input_pins_.push_back(std::make_pair("src_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("lum_tex", TexturePtr()));
		input_pins_.push_back(std::make_pair("bloom_tex", TexturePtr()));
		
		RenderEffectPtr effect = Context::Instance().RenderFactoryInstance().LoadEffect("ToneMapping.fxml");
		technique_ = effect->TechniqueByName("ToneMapping30");
		if (!technique_->Validate())
		{
			technique_ = effect->TechniqueByName("ToneMapping20");
		}

		this->UpdateBinds();

		*(technique_->Effect().ParameterByName("blue_shift")) = blue_shift;
	}


	HDRPostProcess::HDRPostProcess(bool bright_pass, bool blue_shift)
		: PostProcess(L"HDR")
	{
		RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		cs_support_ = caps.cs_support && (5 == caps.max_shader_model);

		if (bright_pass)
		{
			downsampler_ = LoadPostProcess(ResLoader::Instance().Load("Downsampler2x2.ppml"), "bright_pass_downsampler2x2");
		}
		else
		{
			downsampler_ = LoadPostProcess(ResLoader::Instance().Load("Downsampler2x2.ppml"), "downsampler2x2");
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

		TexturePtr downsample_tex = rf.MakeTexture2D(width / 2, height / 2, 1, 1, EF_ABGR16F, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, NULL);
		{
			downsampler_->InputPin(index, tex);
			downsampler_->OutputPin(index, downsample_tex);
		}

		TexturePtr blur_tex = rf.MakeTexture2D(width / 4, height / 4, 1, 1, EF_ABGR16F, 1, 0,
			EAH_GPU_Read | EAH_GPU_Write, NULL);
		{
			blur_->InputPin(index, downsample_tex);
			blur_->OutputPin(index, blur_tex);
		}

		if (cs_support_)
		{
			TexturePtr lum_tex = rf.MakeTexture2D(2, 2, 1, 1, EF_R16F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, NULL);

			float init_lum = 0;
			ElementInitData init_data;
			init_data.row_pitch = sizeof(float);
			init_data.slice_pitch = 0;
			init_data.data = &init_lum;
			TexturePtr adapted_lum_tex = rf.MakeTexture2D(1, 1, 1, 1, EF_R32F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write | EAH_GPU_Unordered, &init_data);

			sum_lums_1st_->InputPin(index, tex);
			sum_lums_1st_->OutputPin(index, lum_tex);
			adapted_lum_->InputPin(index, lum_tex);
			adapted_lum_->OutputPin(index, adapted_lum_tex);
			tone_mapping_->InputPin(1, adapted_lum_tex);
		}
		else
		{
			std::vector<TexturePtr> lum_texs(sum_lums_.size() + 1);
			try
			{
				int len = 1;
				for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
				{
					lum_texs[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, EF_R16F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, NULL);
					len *= 4;
				}

				{
					sum_lums_1st_->InputPin(index, tex);
					sum_lums_1st_->OutputPin(index, lum_texs[0]);
				}
				for (size_t i = 0; i < sum_lums_.size(); ++ i)
				{
					sum_lums_[i]->InputPin(index, lum_texs[i]);
					sum_lums_[i]->OutputPin(index, lum_texs[i + 1]);
				}
			}
			catch (...)
			{
				int len = 1;
				for (size_t i = 0; i < sum_lums_.size() + 1; ++ i)
				{
					lum_texs[sum_lums_.size() - i] = rf.MakeTexture2D(len, len, 1, 1, EF_ABGR16F, 1, 0,
						EAH_GPU_Read | EAH_GPU_Write, NULL);
					len *= 4;
				}

				{
					sum_lums_1st_->InputPin(index, tex);
					sum_lums_1st_->OutputPin(index, lum_texs[0]);
				}
				for (size_t i = 0; i < sum_lums_.size(); ++ i)
				{
					sum_lums_[i]->InputPin(index, lum_texs[i]);
					sum_lums_[i]->OutputPin(index, lum_texs[i + 1]);
				}
			}

			{
				adapted_lum_->InputPin(index, lum_texs[sum_lums_.size()]);
			}
		}

		tone_mapping_->InputPin(0, tex);
		tone_mapping_->InputPin(2, blur_tex);
	}

	TexturePtr const & HDRPostProcess::InputPin(uint32_t index) const
	{
		return downsampler_->InputPin(index);
	}

	void HDRPostProcess::OutputPin(uint32_t index, TexturePtr const & tex, int level, int array_index, int face)
	{
		tone_mapping_->OutputPin(index, tex, level, array_index, face);
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
