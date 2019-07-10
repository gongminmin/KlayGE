#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/InfTerrain.hpp>
#include <KlayGE/LensFlare.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/LightShaft.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KFL/Half.hpp>

#include <sstream>

#include "SampleCommon.hpp"
#include "ProceduralTerrain.hpp"
#include "OceanSimulator.hpp"
#include "Ocean.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderOcean : public InfTerrainRenderable
	{
	public:
		RenderOcean(float base_level, float strength)
			: InfTerrainRenderable(L"Ocean", 384)
		{
			this->BindDeferredEffect(SyncLoadRenderEffect("Ocean.fxml"));
			gbuffer_alpha_blend_front_mrt_tech_ = effect_->TechniqueByName("OceanGBufferAlphaBlendFrontMRT");
			reflection_alpha_blend_front_tech_ = effect_->TechniqueByName("OceanReflectionAlphaBlendFront");
			special_shading_alpha_blend_front_tech_ = effect_->TechniqueByName("OceanSpecialShadingAlphaBlendFront");
			technique_ = gbuffer_alpha_blend_front_mrt_tech_;

			reflection_tex_param_ = effect_->ParameterByName("reflection_tex");

			this->SetStretch(strength);
			this->SetBaseLevel(base_level);

			this->ModelMatrix(float4x4::Identity());

			mtl_ = MakeSharedPtr<RenderMaterial>();
			mtl_->Albedo(float4(0.07f, 0.15f, 0.2f, 1));
			mtl_->Metalness(1);
			mtl_->Glossiness(0.5f);

			effect_attrs_ |= EA_TransparencyFront;
			effect_attrs_ |= EA_SpecialShading;
		}

		void PatchLength(float patch_length)
		{
			*(effect_->ParameterByName("patch_length")) = patch_length;
		}

		void DisplacementParam(float3 const & min_disp0, float3 const & min_disp1, float3 const & disp_range0, float3 const & disp_range1)
		{
			*(effect_->ParameterByName("min_disp0")) = min_disp0;
			*(effect_->ParameterByName("min_disp1")) = min_disp1;
			*(effect_->ParameterByName("disp_range0")) = disp_range0;
			*(effect_->ParameterByName("disp_range1")) = disp_range1;
		}

		void DisplacementMap(TexturePtr const & tex0, TexturePtr const & tex1)
		{
			*(effect_->ParameterByName("displacement_tex_0")) = tex0;
			*(effect_->ParameterByName("displacement_tex_1")) = tex1;
		}

		void GradientMap(TexturePtr const & tex0, TexturePtr const & tex1)
		{
			*(effect_->ParameterByName("gradient_tex_0")) = tex0;
			*(effect_->ParameterByName("gradient_tex_1")) = tex1;
		}

		void DisplacementMapArray(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("displacement_tex_array")) = tex;
		}

		void GradientMapArray(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("gradient_tex_array")) = tex;
		}

		void Frames(int2 const & frames)
		{
			*(effect_->ParameterByName("frames")) = frames;
		}

		void InterpolateFrac(float frac)
		{
			*(effect_->ParameterByName("interpolate_frac")) = frac;
		}

		void SkylightTex(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			*(effect_->ParameterByName("skybox_tex")) = y_cube;
			*(effect_->ParameterByName("skybox_C_tex")) = c_cube;
		}

		void FogColor(Color const & fog_color)
		{
			*(effect_->ParameterByName("fog_color")) = float3(fog_color.r(), fog_color.g(), fog_color.b());
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			reflection_tex_ = tex;
		}

		void OnRenderBegin()
		{
			InfTerrainRenderable::OnRenderBegin();

			auto drl = Context::Instance().DeferredRenderingLayerInstance();

			switch (type_)
			{
			case PT_OpaqueGBufferMRT:
			case PT_TransparencyBackGBufferMRT:
			case PT_TransparencyFrontGBufferMRT:
				*opaque_depth_tex_param_ = drl->CurrFrameResolvedDepthTex(drl->ActiveViewport());
				break;

			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
				*(effect_->ParameterByName("g_buffer_rt0_tex")) = drl->GBufferResolvedRT0Tex(drl->ActiveViewport());
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
					*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
					float4 const near_q_far = camera.NearQFarParam();
					*(effect_->ParameterByName("near_q_far")) = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());
					*(effect_->ParameterByName("ray_length")) = camera.FarPlane() - camera.NearPlane();
					*(effect_->ParameterByName("min_samples")) = static_cast<int32_t>(20);
					*(effect_->ParameterByName("max_samples")) = static_cast<int32_t>(30);
					*(effect_->ParameterByName("view")) = camera.ViewMatrix();
					*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
				}
				*(effect_->ParameterByName("front_side_depth_tex")) = drl->CurrFrameResolvedDepthTex(drl->ActiveViewport());
				*(effect_->ParameterByName("front_side_tex")) = drl->CurrFrameResolvedShadingTex(drl->ActiveViewport());
				break;

			case PT_OpaqueSpecialShading:
			case PT_TransparencyBackSpecialShading:
			case PT_TransparencyFrontSpecialShading:
				*(effect_->ParameterByName("opaque_shading_tex")) = drl->CurrFrameResolvedShadingTex(drl->ActiveViewport());
				*(effect_->ParameterByName("g_buffer_rt0_tex")) = drl->GBufferResolvedRT0Tex(drl->ActiveViewport());
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					float4 const near_q_far = camera.NearQFarParam();
					*(effect_->ParameterByName("near_q_far")) = float3(near_q_far.x(), near_q_far.y(), near_q_far.z());
					*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
				}
				*reflection_tex_param_ = reflection_tex_;
				break;

			default:
				break;
			}
		}

	private:
		TexturePtr reflection_tex_;
	};

	class OceanRenderableComponent : public InfTerrainRenderableComponent
	{
	public:
		OceanRenderableComponent() : InfTerrainRenderableComponent(MakeSharedPtr<RenderOcean>(0.0f, 10.0f))
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();

			base_level_ = 0;
			strength_ = 10;

			auto& ocean_renderable = this->BoundRenderableOfType<RenderOcean>();

			ocean_plane_ = MathLib::from_point_normal(float3(0, base_level_, 0), float3(0, 1, 0));
			reflect_mat_ = MathLib::reflect(ocean_plane_);

			// The size of displacement map.
			ocean_param_.dmap_dim			= 512;
			// The side length (world space) of square sized patch
			ocean_param_.patch_length		= 42;

			ocean_param_.time_peroid		= 3;
			ocean_param_.num_frames			= 24;

			// Adjust this parameter to control the simulation speed
			ocean_param_.time_scale			= 0.8f;
			// A scale to control the amplitude. Not the world space height
			ocean_param_.wave_amplitude		= 0.003f;
			// 2D wind direction. No need to be normalized
			// The bigger the wind speed, the larger scale of wave crest.
			// But the wave scale can be no larger than patch_length
			ocean_param_.wind_speed			= float2(0.8f, 0.6f) * 6.0f;
			// Damp out the components opposite to wind direction.
			// The smaller the value, the higher wind dependency
			ocean_param_.wind_dependency	= 0.1f;
			// Control the scale of horizontal movement. Higher value creates
			// pointy crests.
			ocean_param_.choppy_scale		= 1.1f;

			ocean_simulator_ = MakeSharedPtr<OceanSimulator>();

			use_tex_array_ = re.DeviceCaps().max_texture_array_length >= ocean_param_.num_frames;

			ocean_renderable.PatchLength(ocean_param_.patch_length);

			TexturePtr disp_tex = LoadSoftwareTexture("OceanDisplacement.dds");
			TexturePtr grad_tex = LoadSoftwareTexture("OceanGradient.dds");
			TexturePtr disp_param_tex = LoadSoftwareTexture("OceanDisplacementParam.dds");

			bool use_load_tex;
			if (disp_tex && grad_tex, disp_param_tex
				&& (disp_tex->ArraySize() == ocean_param_.num_frames)
				&& (disp_tex->Width(0) == static_cast<uint32_t>(ocean_param_.dmap_dim))
				&& (disp_tex->Height(0) == disp_tex->Width(0))
				&& (grad_tex->ArraySize() == ocean_param_.num_frames)
				&& (grad_tex->Width(0) == static_cast<uint32_t>(ocean_param_.dmap_dim))
				&& (grad_tex->Height(0) == grad_tex->Width(0))
				&& (disp_param_tex->Width(0) == ocean_param_.num_frames)
				&& (disp_param_tex->Height(0) >= 2))
			{
				auto const& disp_param_init_data = checked_cast<SoftwareTexture&>(*disp_param_tex).SubresourceData();
				float4 const * row0 = static_cast<float4 const *>(disp_param_init_data[0].data);
				float4 const * row1 = row0 + disp_param_init_data[0].row_pitch / sizeof(float4);
				displacement_params_.resize(ocean_param_.num_frames * 2);
				for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
				{
					displacement_params_[i + ocean_param_.num_frames * 0] = float3(row0[i]);
					displacement_params_[i + ocean_param_.num_frames * 1] = float3(row1[i]);
				}

				use_load_tex = true;
				dirty_ = false;
			}
			else
			{
				use_load_tex = false;
				dirty_ = true;
			}

			if (use_tex_array_)
			{
				std::span<ElementInitData const> did;
				std::span<ElementInitData const> gid;
				if (use_load_tex)
				{
					did = checked_cast<SoftwareTexture&>(*disp_tex).SubresourceData();
					gid = checked_cast<SoftwareTexture&>(*grad_tex).SubresourceData();
				}

				displacement_tex_array_ = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					1, ocean_param_.num_frames, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, did);

				TexturePtr gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					1, ocean_param_.num_frames, EF_GR8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, gid);
				gradient_tex_array_ = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					0, ocean_param_.num_frames, EF_GR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
				for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
				{
					gta->CopyToSubTexture2D(*gradient_tex_array_,
						i, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						i, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
				}
				gradient_tex_array_->BuildMipSubLevels();

				ocean_renderable.DisplacementMapArray(displacement_tex_array_);
				ocean_renderable.GradientMapArray(gradient_tex_array_);
			}
			else
			{
				displacement_tex_.resize(ocean_param_.num_frames);
				gradient_tex_.resize(ocean_param_.num_frames);
				for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
				{
					auto const fmt = re.DeviceCaps().BestMatchTextureRenderTargetFormat(MakeSpan({EF_GR8, EF_ABGR8}), 1, 0);
					BOOST_ASSERT(fmt != EF_Unknown);

					std::span<ElementInitData const> did;
					if (use_load_tex)
					{
						auto const& disp_init_data = checked_cast<SoftwareTexture&>(*disp_tex).SubresourceData();
						did = MakeSpan<1>(disp_init_data[i * disp_tex->NumMipMaps()]);
					}

					gradient_tex_[i] = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						0, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
					displacement_tex_[i] = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, did);

					if (use_load_tex)
					{
						auto const & grad_init_data = checked_cast<SoftwareTexture&>(*grad_tex).SubresourceData();

						TexturePtr gta;
						if (EF_GR8 == fmt)
						{
							gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
								1, 1, fmt, 1, 0, EAH_CPU_Read | EAH_CPU_Write, MakeSpan<1>(grad_init_data[i * grad_tex->NumMipMaps()]));
						}
						else
						{
							gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
								1, 1, fmt, 1, 0, EAH_CPU_Read | EAH_CPU_Write);

							Texture::Mapper mapper(*gta, 0, 0, TMA_Write_Only, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
							ElementInitData const & gid = grad_init_data[i * grad_tex->NumMipMaps()];
							uint8_t const * src = static_cast<uint8_t const *>(gid.data);
							uint8_t* dst = mapper.Pointer<uint8_t>();
							uint32_t const src_pitch = gid.row_pitch;
							uint32_t const dst_pitch = mapper.RowPitch();
							for (int y = 0; y < ocean_param_.dmap_dim; ++ y)
							{
								for (int x = 0; x < ocean_param_.dmap_dim; ++ x)
								{
									dst[y * dst_pitch + x * 4 + 0] = src[y * src_pitch + x * 2 + 0];
									dst[y * dst_pitch + x * 4 + 1] = src[y * src_pitch + x * 2 + 1];
									dst[y * dst_pitch + x * 4 + 2] = 0;
									dst[y * dst_pitch + x * 4 + 3] = 0;
								}
							}
						}
						
						gta->CopyToSubTexture2D(*gradient_tex_[i],
							0, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim,
							0, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
					}

					gradient_tex_[i]->BuildMipSubLevels();
				}
			}

			this->OnMainThreadUpdate().Connect([this](SceneComponent& component, float app_time, float elapsed_time)
				{
					KFL_UNUSED(component);
					KFL_UNUSED(elapsed_time);

					if (dirty_)
					{
						this->GenWaveTextures();

						dirty_ = false;
					}

					auto& ocean_renderable = this->BoundRenderableOfType<RenderOcean>();

					float t = app_time * ocean_param_.time_scale / ocean_param_.time_peroid;
					float frame = (t - floor(t)) * ocean_param_.num_frames;
					int frame0 = static_cast<int>(frame);
					int frame1 = frame0 + 1;
					ocean_renderable.InterpolateFrac(frame - frame0);
					frame0 %= ocean_param_.num_frames;
					frame1 %= ocean_param_.num_frames;
					if (use_tex_array_)
					{
						ocean_renderable.Frames(int2(frame0, frame1));
					}
					else
					{
						ocean_renderable.DisplacementMap(displacement_tex_[frame0], displacement_tex_[frame1]);
						ocean_renderable.GradientMap(gradient_tex_[frame0], gradient_tex_[frame1]);
					}
					ocean_renderable.DisplacementParam(displacement_params_[frame0], displacement_params_[frame1],
						displacement_params_[ocean_param_.num_frames + frame0], displacement_params_[ocean_param_.num_frames + frame1]);
				});
		}

		Plane const & OceanPlane() const
		{
			return ocean_plane_;
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			auto& ocean_renderable = this->BoundRenderableOfType<RenderOcean>();
			ocean_renderable.ReflectionTex(tex);
		}

		void ReflectViewParams(float3& reflect_eye, float3& reflect_at, float3& reflect_up,
			float3 const & eye, float3 const & at, float3 const & up)
		{
			reflect_eye = MathLib::transform_coord(eye, reflect_mat_);
			reflect_at = MathLib::transform_coord(at, reflect_mat_);
			reflect_up = MathLib::transform_normal(up, reflect_mat_);
			reflect_up *= -1.0f;
		}

		int DMapDim() const
		{
			return ocean_param_.dmap_dim;
		}
		void DMapDim(int dmap_dim)
		{
			if (ocean_param_.dmap_dim != dmap_dim)
			{
				ocean_param_.dmap_dim = dmap_dim;
				dirty_ = true;
			}
		}

		float PatchLength() const
		{
			return ocean_param_.patch_length;
		}
		void PatchLength(float patch_length)
		{
			if (!MathLib::equal(ocean_param_.patch_length, patch_length))
			{
				ocean_param_.patch_length = patch_length;
				dirty_ = true;
			}
		}

		float TimeScale() const
		{
			return ocean_param_.time_scale;
		}
		void TimeScale(float time_scale)
		{
			if (!MathLib::equal(ocean_param_.time_scale, time_scale))
			{
				ocean_param_.time_scale = time_scale;
				dirty_ = true;
			}
		}

		float WaveAmplitude() const
		{
			return ocean_param_.wave_amplitude;
		}
		void WaveAmplitude(float amp)
		{
			if (!MathLib::equal(ocean_param_.wave_amplitude, amp))
			{
				ocean_param_.wave_amplitude = amp;
				dirty_ = true;
			}
		}

		float WindSpeedX() const
		{
			return ocean_param_.wind_speed.x();
		}
		void WindSpeedX(float speed)
		{
			if (!MathLib::equal(ocean_param_.wind_speed.x(), speed))
			{
				ocean_param_.wind_speed.x() = speed;
				dirty_ = true;
			}
		}

		float WindSpeedY() const
		{
			return ocean_param_.wind_speed.y();
		}
		void WindSpeedY(float speed)
		{
			if (!MathLib::equal(ocean_param_.wind_speed.y(), speed))
			{
				ocean_param_.wind_speed.y() = speed;
				dirty_ = true;
			}
		}

		float WindDependency() const
		{
			return ocean_param_.wind_dependency;
		}
		void WindDependency(float dep)
		{
			if (!MathLib::equal(ocean_param_.wind_dependency, dep))
			{
				ocean_param_.wind_dependency = dep;
				dirty_ = true;
			}
		}

		float ChoppyScale() const
		{
			return ocean_param_.choppy_scale;
		}
		void ChoppyScale(float choppy)
		{
			if (!MathLib::equal(ocean_param_.choppy_scale, choppy))
			{
				ocean_param_.choppy_scale = choppy;
				dirty_ = true;
			}
		}

	private:
		void GenWaveTextures()
		{
			auto& ocean_renderable = this->BoundRenderableOfType<RenderOcean>();
			ocean_renderable.PatchLength(ocean_param_.patch_length);

			ocean_simulator_->Parameters(ocean_param_);

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			TexturePtr disp_cpu_tex = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					1, 1, EF_ABGR16F, 1, 0, EAH_CPU_Read | EAH_CPU_Write);

			std::vector<float4> min_disps(ocean_param_.num_frames, float4(+1e10f, +1e10f, +1e10f, +1e10f));
			std::vector<float4> disp_ranges(ocean_param_.num_frames);
			std::vector<uint32_t> abgr8_disp(ocean_param_.dmap_dim * ocean_param_.dmap_dim);
			for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
			{
				ocean_simulator_->Update(i);

				TexturePtr const & sim_disp_tex = ocean_simulator_->DisplacementTex();
				TexturePtr const & sim_grad_tex = ocean_simulator_->GradientTex();

				sim_disp_tex->CopyToSubTexture2D(*disp_cpu_tex,
					0, 0, 0, 0, sim_disp_tex->Width(0), sim_disp_tex->Height(0),
					0, 0, 0, 0, sim_disp_tex->Width(0), sim_disp_tex->Height(0));

				{
					float4 max_disp = float4(-1e10f, -1e10f, -1e10f, -1e10f);
					Texture::Mapper mapper(*disp_cpu_tex, 0, 0, TMA_Read_Only, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
					half* p = mapper.Pointer<half>();
					for (int y = 0; y < ocean_param_.dmap_dim; ++ y)
					{
						for (int x = 0; x < ocean_param_.dmap_dim; ++ x)
						{
							float4 disp = float4(static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 0]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 1]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 2]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 3]));
							min_disps[i] = MathLib::minimize(min_disps[i], disp);
							max_disp = MathLib::maximize(max_disp, disp);
						}
					}

					disp_ranges[i] = max_disp - min_disps[i];

					for (int y = 0; y < ocean_param_.dmap_dim; ++ y)
					{
						for (int x = 0; x < ocean_param_.dmap_dim; ++ x)
						{
							float4 disp = float4(static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 0]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 1]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 2]),
								static_cast<float>(p[y * mapper.RowPitch() / sizeof(half) + x * 4 + 3]));
							float4 normalized_disp = (disp - min_disps[i]) / disp_ranges[i];
							abgr8_disp[y * ocean_param_.dmap_dim + x] = Color(&normalized_disp.x()).ABGR();
						}
					}
				}

				ElementInitData init_data;
				init_data.data = &abgr8_disp[0];
				init_data.row_pitch = ocean_param_.dmap_dim * sizeof(uint32_t);
				init_data.slice_pitch = init_data.row_pitch * ocean_param_.dmap_dim;
				TexturePtr disp_8 = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, MakeSpan<1>(init_data));

				if (use_tex_array_)
				{
					disp_8->CopyToSubTexture2D(*displacement_tex_array_,
						i, 0, 0, 0, sim_disp_tex->Width(0), sim_disp_tex->Height(0),
						0, 0, 0, 0, sim_disp_tex->Width(0), sim_disp_tex->Height(0));
							
					sim_grad_tex->CopyToSubTexture2D(*gradient_tex_array_,
						i, 0, 0, 0, sim_grad_tex->Width(0), sim_grad_tex->Height(0),
						0, 0, 0, 0, sim_grad_tex->Width(0), sim_grad_tex->Height(0));
				}
				else
				{
					disp_8->CopyToTexture(*displacement_tex_[i]);
					sim_grad_tex->CopyToSubTexture2D(*gradient_tex_[i],
						0, 0, 0, 0, sim_grad_tex->Width(0), sim_grad_tex->Height(0),
						0, 0, 0, 0, sim_grad_tex->Width(0), sim_grad_tex->Height(0));
					gradient_tex_[i]->BuildMipSubLevels();
				}
			}

			if (use_tex_array_)
			{
				gradient_tex_array_->BuildMipSubLevels();
			}

			displacement_params_.resize(ocean_param_.num_frames * 2);
			for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
			{
				displacement_params_[i + ocean_param_.num_frames * 0] = float3(min_disps[i]);
				displacement_params_[i + ocean_param_.num_frames * 1] = float3(disp_ranges[i]);
			}

			this->SaveWaveTextures();
		}

		void SaveWaveTextures()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			std::vector<float4> disp_params(ocean_param_.num_frames * 2);
			for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
			{
				float3 const & min_disp = displacement_params_[i + ocean_param_.num_frames * 0];
				float3 const & disp_range = displacement_params_[i + ocean_param_.num_frames * 1];
				disp_params[i + ocean_param_.num_frames * 0] = float4(min_disp.x(), min_disp.y(), min_disp.z(), 1);
				disp_params[i + ocean_param_.num_frames * 1] = float4(disp_range.x(), disp_range.y(), disp_range.z(), 1);
			}

			std::string const PREFIX = "../../Samples/media/Ocean/";

			ElementInitData param_init_data;
			param_init_data.data = &disp_params[0];
			param_init_data.row_pitch = ocean_param_.num_frames * sizeof(float4);
			param_init_data.slice_pitch = param_init_data.row_pitch * 2;
			TexturePtr ocean_displacement_param_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, ocean_param_.num_frames, 2, 1,
				1, 1, EF_ABGR32F, true);
			ocean_displacement_param_tex->CreateHWResource(MakeSpan<1>(param_init_data), nullptr);
			SaveTexture(ocean_displacement_param_tex, PREFIX + "OceanDisplacementParam.dds");

			if (use_tex_array_)
			{
				SaveTexture(displacement_tex_array_, PREFIX + "OceanDisplacement.dds");

				TexturePtr gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
					1, ocean_param_.num_frames, EF_GR8, 1, 0, EAH_CPU_Read | EAH_CPU_Write);
				for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
				{
					gradient_tex_array_->CopyToSubTexture2D(*gta,
						i, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						i, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
				}
				SaveTexture(gta, PREFIX + "OceanGradient.dds");
			}
			else
			{
				{
					uint32_t const disp_pixel_size = NumFormatBytes(displacement_tex_[0]->Format());
					std::vector<ElementInitData> disp_init_data(ocean_param_.num_frames);
					std::vector<std::vector<uint8_t>> disp_data(ocean_param_.num_frames);
					TexturePtr disp_slice = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						1, 1, displacement_tex_[0]->Format(), 1, 0, EAH_CPU_Read | EAH_CPU_Write);
					for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
					{
						displacement_tex_[i]->CopyToTexture(*disp_slice);

						disp_init_data[i].row_pitch = ocean_param_.dmap_dim * disp_pixel_size;
						disp_init_data[i].slice_pitch = disp_init_data[i].row_pitch * ocean_param_.dmap_dim;

						disp_data[i].resize(disp_init_data[i].slice_pitch);
						disp_init_data[i].data = &disp_data[i][0];

						Texture::Mapper mapper(*disp_slice, 0, 0, TMA_Read_Only, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
						for (int y = 0; y < ocean_param_.dmap_dim; ++ y)
						{
							memcpy(&disp_data[i][y * disp_init_data[i].row_pitch],
								mapper.Pointer<uint8_t>() + y * mapper.RowPitch(),
								ocean_param_.dmap_dim * disp_pixel_size);
						}
					}

					TexturePtr ocean_displacement_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D,
						ocean_param_.dmap_dim, ocean_param_.dmap_dim, 1, 1, ocean_param_.num_frames,
						displacement_tex_[0]->Format(), true);
					ocean_displacement_tex->CreateHWResource(disp_init_data, nullptr);
					SaveTexture(ocean_displacement_tex, PREFIX + "OceanDisplacement.dds");
				}

				{
					uint32_t const grad_pixel_size = NumFormatBytes(gradient_tex_[0]->Format());
					std::vector<ElementInitData> grad_init_data(ocean_param_.num_frames);
					std::vector<std::vector<uint8_t>> grad_data(ocean_param_.num_frames);
					TexturePtr grad_slice = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						1, 1, gradient_tex_[0]->Format(), 1, 0, EAH_CPU_Read | EAH_CPU_Write);
					for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
					{
						gradient_tex_[i]->CopyToSubTexture2D(*grad_slice,
							0, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim,
							0, 0, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);

						grad_init_data[i].row_pitch = ocean_param_.dmap_dim * grad_pixel_size;
						grad_init_data[i].slice_pitch = grad_init_data[i].row_pitch * ocean_param_.dmap_dim;

						grad_data[i].resize(grad_init_data[i].slice_pitch);
						grad_init_data[i].data = &grad_data[i][0];

						Texture::Mapper mapper(*grad_slice, 0, 0, TMA_Read_Only, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
						for (int y = 0; y < ocean_param_.dmap_dim; ++ y)
						{
							memcpy(&grad_data[i][y * grad_init_data[i].row_pitch],
								mapper.Pointer<uint8_t>() + y * mapper.RowPitch(),
								ocean_param_.dmap_dim * grad_pixel_size);
						}
					}

					TexturePtr ocean_gradient_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D,
						ocean_param_.dmap_dim, ocean_param_.dmap_dim, 1, 1, ocean_param_.num_frames,
						gradient_tex_[0]->Format(), true);
					ocean_gradient_tex->CreateHWResource(grad_init_data, nullptr);
					SaveTexture(ocean_gradient_tex, PREFIX + "OceanGradient.dds");
				}
			}
		}

	private:
		OceanParameter ocean_param_;
		bool dirty_;

		Plane ocean_plane_;
		float4x4 reflect_mat_;

		std::shared_ptr<OceanSimulator> ocean_simulator_;

		bool use_tex_array_;
		std::vector<TexturePtr> displacement_tex_;
		std::vector<TexturePtr> gradient_tex_;
		TexturePtr displacement_tex_array_;
		TexturePtr gradient_tex_array_;
		std::vector<float3> displacement_params_;
	};

	class RenderableFoggySkyBox : public RenderableSkyBox
	{
	public:
		RenderableFoggySkyBox()
		{
			RenderEffectPtr effect = SyncLoadRenderEffect("Ocean.fxml");

			gbuffer_mrt_tech_ = effect->TechniqueByName("GBufferSkyBoxMRTTech");
			special_shading_tech_ = effect->TechniqueByName("SpecialShadingFoggySkyBox");
			this->Technique(effect, gbuffer_mrt_tech_);
		}
		
		void FogColor(Color const & clr)
		{
			*(effect_->ParameterByName("fog_color")) = float3(clr.r(), clr.g(), clr.b());
		}
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}

int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.deferred_rendering = true;
	Context::Instance().Config(cfg);

	OceanApp app;
	app.Create();
	app.Run();

	return 0;
}

OceanApp::OceanApp()
			: App3DFramework("Ocean"),
				light_shaft_on_(true)
{
	ResLoader::Instance().AddPath("../../Samples/media/Ocean");
}

void OceanApp::OnCreate()
{
	this->LookAt(float3(-3455.78f, 23.4f, 8133.55f), float3(-3456.18f, 23.4f, 8134.49f));
	this->Proj(0.1f, 7000);

	TexturePtr c_cube = ASyncLoadTexture("DH001cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr y_cube = ASyncLoadTexture("DH001cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);

	font_ = SyncLoadFont("gkai00mp.kfont");

	deferred_rendering_ = Context::Instance().DeferredRenderingLayerInstance();
	deferred_rendering_->SSVOEnabled(1, false);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	auto sun_light_node = MakeSharedPtr<SceneNode>(0);
	sun_light_ = MakeSharedPtr<DirectionalLightSource>();
	// TODO: Fix the shadow flicking
	sun_light_->Attrib(LightSource::LSA_NoShadow);
	sun_light_->Color(float3(1, 0.7f, 0.5f));
	sun_light_node->TransformToParent(
		MathLib::to_matrix(MathLib::axis_to_axis(float3(0, 0, 1), float3(0.267835f, -0.0517653f, -0.960315f))));
	sun_light_node->AddComponent(sun_light_);
	sun_light_node->AddComponent(MakeSharedPtr<LensFlareRenderableComponent>());
	root_node.AddChild(sun_light_node);
	
	Color fog_color(0.61f, 0.52f, 0.62f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		fog_color.r() = MathLib::srgb_to_linear(fog_color.r());
		fog_color.g() = MathLib::srgb_to_linear(fog_color.g());
		fog_color.b() = MathLib::srgb_to_linear(fog_color.b());
	}

	auto terrain_renderable = MakeSharedPtr<ProceduralTerrain>();
	terrain_renderable->TextureLayer(0, ASyncLoadTexture("RealSand40BoH.dds", EAH_GPU_Read | EAH_Immutable));
	terrain_renderable->TextureLayer(1, ASyncLoadTexture("snow_DM.dds", EAH_GPU_Read | EAH_Immutable));
	terrain_renderable->TextureLayer(2, ASyncLoadTexture("GrassGreenTexture0002.dds", EAH_GPU_Read | EAH_Immutable));
	terrain_renderable->TextureLayer(3, ASyncLoadTexture("Dirt.dds", EAH_GPU_Read | EAH_Immutable));
	terrain_renderable->TextureScale(0, float2(7, 7));
	terrain_renderable->TextureScale(1, float2(1, 1));
	terrain_renderable->TextureScale(2, float2(3, 3));
	terrain_renderable->TextureScale(3, float2(1, 1));
	auto terrain_node =
		MakeSharedPtr<SceneNode>(MakeSharedPtr<HQTerrainRenderableComponent>(terrain_renderable), L"TerrainNode", SceneNode::SOA_Moveable);
	root_node.AddChild(terrain_node);

	ocean_ = MakeSharedPtr<OceanRenderableComponent>();
	auto ocean_node = MakeSharedPtr<SceneNode>(ocean_, L"OceanNode", SceneNode::SOA_Moveable);
	root_node.AddChild(ocean_node);
	ocean_->BoundRenderableOfType<RenderOcean>().SkylightTex(y_cube, c_cube);
	ocean_->BoundRenderableOfType<RenderOcean>().FogColor(fog_color);

	terrain_renderable->ReflectionPlane(checked_pointer_cast<OceanRenderableComponent>(ocean_)->OceanPlane());

	auto skybox = MakeSharedPtr<RenderableFoggySkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	skybox->FogColor(fog_color);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	fog_pp_ = SyncLoadPostProcess("Fog.ppml", "fog");
	fog_pp_->SetParam(1, float3(fog_color.r(), fog_color.g(), fog_color.b()));
	fog_pp_->SetParam(2, 1.0f / 5000);
	fog_pp_->SetParam(3, this->ActiveCamera().FarPlane());
	deferred_rendering_->AtmosphericPostProcess(fog_pp_);

	light_shaft_pp_ = MakeSharedPtr<LightShaftPostProcess>();
	light_shaft_pp_->SetParam(1, sun_light_->Color());

	Camera& scene_camera = this->ActiveCamera();
	reflection_fb_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	reflection_fb_->Viewport()->Camera()->ProjParams(scene_camera.FOV(), scene_camera.Aspect(),
		scene_camera.NearPlane(), scene_camera.FarPlane());

	auto reflection_camera_node =
		MakeSharedPtr<SceneNode>(L"ReflectionCameraNode", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
	reflection_camera_node->AddComponent(reflection_fb_->Viewport()->Camera());
	root_node.AddChild(reflection_camera_node);

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("Ocean.uiml"));
	dialog_params_ = UIManager::Instance().GetDialog("Parameters");
	id_dmap_dim_static_ = dialog_params_->IDFromName("DMapDimStatic");
	id_dmap_dim_slider_ = dialog_params_->IDFromName("DMapDimSlider");
	id_patch_length_static_ = dialog_params_->IDFromName("PatchLengthStatic");
	id_patch_length_slider_ = dialog_params_->IDFromName("PatchLengthSlider");
	id_time_scale_static_ = dialog_params_->IDFromName("TimeScaleStatic");
	id_time_scale_slider_ = dialog_params_->IDFromName("TimeScaleSlider");
	id_wave_amplitude_static_ = dialog_params_->IDFromName("WaveAmplitudeStatic");
	id_wave_amplitude_slider_ = dialog_params_->IDFromName("WaveAmplitudeSlider");
	id_wind_speed_x_static_ = dialog_params_->IDFromName("WindSpeedXStatic");
	id_wind_speed_x_slider_ = dialog_params_->IDFromName("WindSpeedXSlider");
	id_wind_speed_y_static_ = dialog_params_->IDFromName("WindSpeedYStatic");
	id_wind_speed_y_slider_ = dialog_params_->IDFromName("WindSpeedYSlider");
	id_wind_dependency_static_ = dialog_params_->IDFromName("WindDependencyStatic");
	id_wind_dependency_slider_ = dialog_params_->IDFromName("WindDependencySlider");
	id_choppy_scale_static_ = dialog_params_->IDFromName("ChoppyScaleStatic");
	id_choppy_scale_slider_ = dialog_params_->IDFromName("ChoppyScaleSlider");
	id_light_shaft_ = dialog_params_->IDFromName("LightShaft");
	id_fps_camera_ = dialog_params_->IDFromName("FPSCamera");

	dialog_params_->Control<UISlider>(id_dmap_dim_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->DMapDimChangedHandler(sender);
		});
	this->DMapDimChangedHandler(*dialog_params_->Control<UISlider>(id_dmap_dim_slider_));

	dialog_params_->Control<UISlider>(id_patch_length_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->PatchLengthChangedHandler(sender);
		});
	this->PatchLengthChangedHandler(*dialog_params_->Control<UISlider>(id_patch_length_slider_));

	dialog_params_->Control<UISlider>(id_time_scale_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->TimeScaleChangedHandler(sender);
		});
	this->TimeScaleChangedHandler(*dialog_params_->Control<UISlider>(id_time_scale_slider_));

	dialog_params_->Control<UISlider>(id_wave_amplitude_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->WaveAmplitudeChangedHandler(sender);
		});
	this->WaveAmplitudeChangedHandler(*dialog_params_->Control<UISlider>(id_wave_amplitude_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_x_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->WindSpeedXChangedHandler(sender);
		});
	this->WindSpeedXChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_x_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_y_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->WindSpeedYChangedHandler(sender);
		});
	this->WindSpeedYChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_y_slider_));

	dialog_params_->Control<UISlider>(id_wind_dependency_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->WindDependencyChangedHandler(sender);
		});
	this->WindDependencyChangedHandler(*dialog_params_->Control<UISlider>(id_wind_dependency_slider_));

	dialog_params_->Control<UISlider>(id_choppy_scale_slider_)->OnValueChangedEvent().Connect(
		[this](UISlider const & sender)
		{
			this->ChoppyScaleChangedHandler(sender);
		});
	this->ChoppyScaleChangedHandler(*dialog_params_->Control<UISlider>(id_choppy_scale_slider_));

	dialog_params_->Control<UICheckBox>(id_light_shaft_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->LightShaftHandler(sender);
		});

	dialog_params_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().Connect(
		[this](UICheckBox const & sender)
		{
			this->FPSCameraHandler(sender);
		});
}

void OceanApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	deferred_rendering_->SetupViewport(1, re.CurFrameBuffer(), 0);

	reflection_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1,
		deferred_rendering_->ShadingTex(1)->Format(), 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	reflection_ds_tex_ = rf.MakeTexture2D(width / 2, height / 2, 1, 1,
		EF_D16, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	reflection_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(reflection_tex_, 0, 1, 0));
	reflection_fb_->Attach(rf.Make2DDsv(reflection_ds_tex_, 0, 1, 0));

	deferred_rendering_->SetupViewport(0, reflection_fb_,
		VPAM_NoTransparencyBack | VPAM_NoTransparencyFront | VPAM_NoSimpleForward | VPAM_NoGI | VPAM_NoSSVO);

	screen_camera_ = re.CurFrameBuffer()->Viewport()->Camera();
}

void OceanApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void OceanApp::DMapDimChangedHandler(UISlider const & sender)
{
	int dmap_dim = 1UL << (3 * sender.GetValue());

	std::wostringstream stream;
	stream << L"DMap dim: " << dmap_dim;
	dialog_params_->Control<UIStatic>(id_dmap_dim_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->DMapDim(dmap_dim);
}

void OceanApp::PatchLengthChangedHandler(UISlider const & sender)
{
	float patch_length = sender.GetValue() * 0.5f;

	std::wostringstream stream;
	stream << L"Patch length: " << patch_length;
	dialog_params_->Control<UIStatic>(id_patch_length_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->PatchLength(patch_length);
}

void OceanApp::TimeScaleChangedHandler(UISlider const & sender)
{
	float time_scale = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Time scale: " << time_scale;
	dialog_params_->Control<UIStatic>(id_time_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->TimeScale(time_scale);
}

void OceanApp::WaveAmplitudeChangedHandler(UISlider const & sender)
{
	float wave_amp = sender.GetValue() * 0.0001f;

	std::wostringstream stream;
	stream << L"Wave amplitude: " << wave_amp;
	dialog_params_->Control<UIStatic>(id_wave_amplitude_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->WaveAmplitude(wave_amp);
}

void OceanApp::WindSpeedXChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed X: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_x_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->WindSpeedX(wind_speed);
}

void OceanApp::WindSpeedYChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed Y: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_y_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->WindSpeedY(wind_speed);
}

void OceanApp::WindDependencyChangedHandler(UISlider const & sender)
{
	float dep = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"Wind dependency: " << dep;
	dialog_params_->Control<UIStatic>(id_wind_dependency_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->WindDependency(dep);
}

void OceanApp::ChoppyScaleChangedHandler(UISlider const & sender)
{
	float choppy = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Choppy scale: " << choppy;
	dialog_params_->Control<UIStatic>(id_choppy_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanRenderableComponent>(ocean_)->ChoppyScale(choppy);
}

void OceanApp::LightShaftHandler(UICheckBox const & sender)
{
	light_shaft_on_ = sender.GetChecked();
}

void OceanApp::FPSCameraHandler(UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpcController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpcController_.DetachCamera();
	}
}

void OceanApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Ocean", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	stream.str(L"");
	stream << deferred_rendering_->NumObjectsRendered() << " Scene objects "
		<< deferred_rendering_->NumRenderablesRendered() << " Renderables "
		<< deferred_rendering_->NumPrimitivesRendered() << " Primitives "
		<< deferred_rendering_->NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str(), 16);

	uint32_t const num_loading_res = ResLoader::Instance().NumLoadingResources();
	if (num_loading_res > 0)
	{
		stream.str(L"");
		stream << "Loading " << num_loading_res << " resources...";
		font_->RenderText(100, 300, Color(1, 0, 0, 1), stream.str(), 48);
	}
}

uint32_t OceanApp::DoUpdate(uint32_t pass)
{
	if (0 == deferred_rendering_->ActiveViewport())
	{
		ocean_->Enabled(false);

		float3 reflect_eye, reflect_at, reflect_up;
		checked_pointer_cast<OceanRenderableComponent>(ocean_)->ReflectViewParams(
			reflect_eye, reflect_at, reflect_up, screen_camera_->EyePos(), screen_camera_->LookAt(), screen_camera_->UpVec());
		reflection_fb_->Viewport()->Camera()->LookAtDist(MathLib::length(reflect_at - reflect_eye));
		reflection_fb_->Viewport()->Camera()->BoundSceneNode()->TransformToWorld(
			MathLib::inverse(MathLib::look_at_lh(reflect_eye, reflect_at, reflect_up)));
	}
	else
	{
		ocean_->Enabled(true);

		checked_pointer_cast<OceanRenderableComponent>(ocean_)->ReflectionTex(reflection_tex_);
	}

	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		if (light_shaft_on_)
		{
			light_shaft_pp_->SetParam(0, -sun_light_->Direction() * 10000.0f + this->ActiveCamera().EyePos());
			light_shaft_pp_->InputPin(0, deferred_rendering_->PrevFrameResolvedShadingSrv(deferred_rendering_->ActiveViewport()));
			light_shaft_pp_->InputPin(1, deferred_rendering_->PrevFrameResolvedDepthSrv(deferred_rendering_->ActiveViewport()));
			light_shaft_pp_->Apply();
		}
	}

	return ret;
}
