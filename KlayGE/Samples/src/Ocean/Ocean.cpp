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
#include <KlayGE/SceneObjectHelper.hpp>
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
			gbuffer_alpha_blend_front_mrt_tech_ = deferred_effect_->TechniqueByName("OceanGBufferAlphaBlendFrontMRT");
			reflection_alpha_blend_front_tech_ = deferred_effect_->TechniqueByName("OceanReflectionAlphaBlendFront");
			special_shading_alpha_blend_front_tech_ = deferred_effect_->TechniqueByName("OceanSpecialShadingAlphaBlendFront");
			technique_ = gbuffer_alpha_blend_front_mrt_tech_;

			reflection_tex_param_ = effect_->ParameterByName("reflection_tex");

			this->SetStretch(strength);
			this->SetBaseLevel(base_level);

			model_mat_ = float4x4::Identity();
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
				*albedo_clr_param_ = float4(0.07f, 0.15f, 0.2f, 1);
				*albedo_map_enabled_param_ = static_cast<int32_t>(0);
				*normal_map_enabled_param_ = static_cast<int32_t>(0);
				*height_map_parallax_enabled_param_ = static_cast<int32_t>(0);
				*height_map_tess_enabled_param_ = static_cast<int32_t>(0);
				*metalness_clr_param_ = float2(1, 0);
				*glossiness_clr_param_ = float2(0.5f, 0);
				*opaque_depth_tex_param_ = drl->CurrFrameDepthTex(drl->ActiveViewport());
				break;

			case PT_OpaqueReflection:
			case PT_TransparencyBackReflection:
			case PT_TransparencyFrontReflection:
				*albedo_tex_param_ = TexturePtr();
				*albedo_map_enabled_param_ = static_cast<int32_t>(0);
				*emissive_tex_param_ = TexturePtr();
				*emissive_clr_param_ = float4(0, 0, 0, 0);
				*(effect_->ParameterByName("g_buffer_tex")) = drl->GBufferRT0Tex(drl->ActiveViewport());
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					*(effect_->ParameterByName("proj")) = camera.ProjMatrix();
					*(effect_->ParameterByName("inv_proj")) = camera.InverseProjMatrix();
					float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
					float3 near_q_far(camera.NearPlane() * q, q, camera.FarPlane());
					*(effect_->ParameterByName("near_q_far")) = near_q_far;
					*(effect_->ParameterByName("ray_length")) = camera.FarPlane() - camera.NearPlane();
					*(effect_->ParameterByName("min_samples")) = static_cast<int32_t>(20);
					*(effect_->ParameterByName("max_samples")) = static_cast<int32_t>(30);
					*(effect_->ParameterByName("view")) = camera.ViewMatrix();
					*(effect_->ParameterByName("inv_view")) = camera.InverseViewMatrix();
				}
				*(effect_->ParameterByName("front_side_depth_tex")) = drl->CurrFrameDepthTex(drl->ActiveViewport());
				*(effect_->ParameterByName("front_side_tex")) = drl->CurrFrameShadingTex(drl->ActiveViewport());
				break;

			case PT_OpaqueSpecialShading:
			case PT_TransparencyBackSpecialShading:
			case PT_TransparencyFrontSpecialShading:
				*albedo_tex_param_ = TexturePtr();
				*albedo_map_enabled_param_ = static_cast<int32_t>(0);
				*emissive_tex_param_ = TexturePtr();
				*emissive_clr_param_ = float4(0, 0, 0, 0);
				*(effect_->ParameterByName("opaque_shading_tex")) = drl->CurrFrameShadingTex(drl->ActiveViewport());
				*(effect_->ParameterByName("g_buffer_tex")) = drl->GBufferRT0Tex(drl->ActiveViewport());
				{
					App3DFramework const & app = Context::Instance().AppInstance();
					Camera const & camera = app.ActiveCamera();
					float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
					float3 near_q_far(camera.NearPlane() * q, q, camera.FarPlane());
					*(effect_->ParameterByName("near_q_far")) = near_q_far;
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

	class OceanObject : public InfTerrainSceneObject
	{
	public:
		OceanObject()
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderEngine& re = rf.RenderEngineInstance();

			base_level_ = 0;
			strength_ = 10;

			renderable_ = MakeSharedPtr<RenderOcean>(base_level_, strength_);

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

			checked_pointer_cast<RenderOcean>(renderable_)->PatchLength(ocean_param_.patch_length);

			Texture::TextureType disp_type;
			uint32_t disp_width = 0, disp_height = 0, disp_depth;
			uint32_t disp_num_mipmaps = 1;
			uint32_t disp_array_size = 0;
			ElementFormat disp_format;
			std::vector<ElementInitData> disp_init_data;
			std::vector<uint8_t> disp_data_block;
			if (!ResLoader::Instance().Locate("OceanDisplacement.dds").empty())
			{
				LoadTexture("OceanDisplacement.dds", disp_type, disp_width, disp_height, disp_depth, disp_num_mipmaps, disp_array_size,
					disp_format, disp_init_data, disp_data_block);
			}

			Texture::TextureType grad_type;
			uint32_t grad_width = 0, grad_height = 0, grad_depth;
			uint32_t grad_num_mipmaps = 1;
			uint32_t grad_array_size = 0;
			ElementFormat grad_format;
			std::vector<ElementInitData> grad_init_data;
			std::vector<uint8_t> grad_data_block;
			if (!ResLoader::Instance().Locate("OceanGradient.dds").empty())
			{
				LoadTexture("OceanGradient.dds", grad_type, grad_width, grad_height, grad_depth, grad_num_mipmaps, grad_array_size,
					grad_format, grad_init_data, grad_data_block);
			}
			
			Texture::TextureType disp_param_type;
			uint32_t disp_param_width = 0, disp_param_height = 0, disp_param_depth;
			uint32_t disp_param_num_mipmaps = 1;
			uint32_t disp_param_array_size = 0;
			ElementFormat disp_param_format;
			std::vector<ElementInitData> disp_param_init_data;
			std::vector<uint8_t> disp_param_data_block;
			if (!ResLoader::Instance().Locate("OceanDisplacementParam.dds").empty())
			{
				LoadTexture("OceanDisplacementParam.dds", disp_param_type, disp_param_width, disp_param_height, disp_param_depth, disp_param_num_mipmaps, disp_param_array_size,
					disp_param_format, disp_param_init_data, disp_param_data_block);
			}

			bool use_load_tex;
			if ((disp_array_size == ocean_param_.num_frames) && (disp_width == static_cast<uint32_t>(ocean_param_.dmap_dim)) && (disp_height == disp_width)
				&& (grad_array_size == ocean_param_.num_frames) && (grad_width == static_cast<uint32_t>(ocean_param_.dmap_dim)) && (grad_height == grad_width)
				&& (disp_param_width == ocean_param_.num_frames) && (disp_param_height >= 2))
			{
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
				ArrayRef<ElementInitData> did;
				ArrayRef<ElementInitData> gid;
				if (use_load_tex)
				{
					did = disp_init_data;
					gid = grad_init_data;
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

				checked_pointer_cast<RenderOcean>(renderable_)->DisplacementMapArray(displacement_tex_array_);
				checked_pointer_cast<RenderOcean>(renderable_)->GradientMapArray(gradient_tex_array_);
			}
			else
			{
				displacement_tex_.resize(ocean_param_.num_frames);
				gradient_tex_.resize(ocean_param_.num_frames);
				for (uint32_t i = 0; i < ocean_param_.num_frames; ++ i)
				{
					ElementFormat fmt;
					if (re.DeviceCaps().texture_format_support(EF_GR8))
					{
						fmt = EF_GR8;
					}
					else
					{
						BOOST_ASSERT(re.DeviceCaps().texture_format_support(EF_ABGR8));

						fmt = EF_ABGR8;
					}

					ArrayRef<ElementInitData> did;
					if (use_load_tex)
					{
						did = disp_init_data[i * disp_num_mipmaps];
					}

					gradient_tex_[i] = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						0, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write | EAH_Generate_Mips);
					displacement_tex_[i] = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
						1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write, did);

					if (use_load_tex)
					{
						TexturePtr gta;
						if (EF_GR8 == fmt)
						{
							gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
								1, 1, fmt, 1, 0, EAH_CPU_Read | EAH_CPU_Write, grad_init_data[i * grad_num_mipmaps]);
						}
						else
						{
							gta = rf.MakeTexture2D(ocean_param_.dmap_dim, ocean_param_.dmap_dim,
								1, 1, fmt, 1, 0, EAH_CPU_Read | EAH_CPU_Write);

							Texture::Mapper mapper(*gta, 0, 0, TMA_Write_Only, 0, 0, ocean_param_.dmap_dim, ocean_param_.dmap_dim);
							ElementInitData* gid = &grad_init_data[i * grad_num_mipmaps];
							uint8_t const * src = static_cast<uint8_t const *>(gid->data);
							uint8_t* dst = mapper.Pointer<uint8_t>();
							uint32_t const src_pitch = gid->row_pitch;
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
		}

		Plane const & OceanPlane() const
		{
			return ocean_plane_;
		}

		virtual bool MainThreadUpdate(float app_time, float elapsed_time) override
		{
			if (dirty_)
			{
				this->GenWaveTextures();

				dirty_ = false;
			}

			InfTerrainSceneObject::MainThreadUpdate(app_time, elapsed_time);

			float t = app_time * ocean_param_.time_scale / ocean_param_.time_peroid;
			float frame = (t - floor(t)) * ocean_param_.num_frames;
			int frame0 = static_cast<int>(frame);
			int frame1 = frame0 + 1;
			checked_pointer_cast<RenderOcean>(renderable_)->InterpolateFrac(frame - frame0);
			frame0 %= ocean_param_.num_frames;
			frame1 %= ocean_param_.num_frames;
			if (use_tex_array_)
			{
				checked_pointer_cast<RenderOcean>(renderable_)->Frames(int2(frame0, frame1));
			}
			else
			{
				checked_pointer_cast<RenderOcean>(renderable_)->DisplacementMap(displacement_tex_[frame0], displacement_tex_[frame1]);
				checked_pointer_cast<RenderOcean>(renderable_)->GradientMap(gradient_tex_[frame0], gradient_tex_[frame1]);
			}
			checked_pointer_cast<RenderOcean>(renderable_)->DisplacementParam(displacement_params_[frame0], displacement_params_[frame1],
				displacement_params_[ocean_param_.num_frames + frame0], displacement_params_[ocean_param_.num_frames + frame1]);

			return false;
		}

		void ReflectionTex(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->ReflectionTex(tex);
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

		void SkylightTex(TexturePtr const & y_cube, TexturePtr const & c_cube)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->SkylightTex(y_cube, c_cube);
		}

		void FogColor(Color const & fog_color)
		{
			checked_pointer_cast<RenderOcean>(renderable_)->FogColor(fog_color);
		}

	private:
		void GenWaveTextures()
		{
			checked_pointer_cast<RenderOcean>(renderable_)->PatchLength(ocean_param_.patch_length);

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
					1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read | EAH_CPU_Write, init_data);

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
			SaveTexture(PREFIX + "OceanDisplacementParam.dds", Texture::TT_2D, ocean_param_.num_frames, 2, 1, 1, 1, EF_ABGR32F, param_init_data);

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

					SaveTexture(PREFIX + "OceanDisplacement.dds", Texture::TT_2D,
						ocean_param_.dmap_dim, ocean_param_.dmap_dim, 1, 1, ocean_param_.num_frames,
						displacement_tex_[0]->Format(), disp_init_data);
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

					SaveTexture(PREFIX + "OceanGradient.dds", Texture::TT_2D,
						ocean_param_.dmap_dim, ocean_param_.dmap_dim, 1, 1, ocean_param_.num_frames,
						gradient_tex_[0]->Format(), grad_init_data);
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

	class SceneObjectFoggySkyBox : public SceneObjectSkyBox
	{
	public:
		explicit SceneObjectFoggySkyBox(uint32_t attrib = 0)
			: SceneObjectSkyBox(attrib)
		{
			renderable_ = MakeSharedPtr<RenderableFoggySkyBox>();
		}

		void FogColor(Color const & clr)
		{
			checked_pointer_cast<RenderableFoggySkyBox>(renderable_)->FogColor(clr);
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
	deferred_rendering_->SSVOEnabled(0, false);

	sun_light_ = MakeSharedPtr<DirectionalLightSource>();
	// TODO: Fix the shadow flicking
	sun_light_->Attrib(LightSource::LSA_NoShadow);
	sun_light_->Direction(float3(0.267835f, -0.0517653f, -0.960315f));
	sun_light_->Color(float3(1, 0.7f, 0.5f));
	sun_light_->AddToSceneManager();
	
	Color fog_color(0.61f, 0.52f, 0.62f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		fog_color.r() = MathLib::srgb_to_linear(fog_color.r());
		fog_color.g() = MathLib::srgb_to_linear(fog_color.g());
		fog_color.b() = MathLib::srgb_to_linear(fog_color.b());
	}

	HQTerrainSceneObjectPtr terrain = MakeSharedPtr<HQTerrainSceneObject>(MakeSharedPtr<ProceduralTerrain>());
	terrain->TextureLayer(0, ASyncLoadTexture("RealSand40BoH.dds", EAH_GPU_Read | EAH_Immutable));
	terrain->TextureLayer(1, ASyncLoadTexture("snow_DM.dds", EAH_GPU_Read | EAH_Immutable));
	terrain->TextureLayer(2, ASyncLoadTexture("GrassGreenTexture0002.dds", EAH_GPU_Read | EAH_Immutable));
	terrain->TextureLayer(3, ASyncLoadTexture("Dirt.dds", EAH_GPU_Read | EAH_Immutable));
	terrain->TextureScale(0, float2(7, 7));
	terrain->TextureScale(1, float2(1, 1));
	terrain->TextureScale(2, float2(3, 3));
	terrain->TextureScale(3, float2(1, 1));
	terrain_ = terrain;
	terrain_->AddToSceneManager();

	ocean_ = MakeSharedPtr<OceanObject>();
	ocean_->AddToSceneManager();
	checked_pointer_cast<OceanObject>(ocean_)->SkylightTex(y_cube, c_cube);
	checked_pointer_cast<OceanObject>(ocean_)->FogColor(fog_color);

	checked_pointer_cast<ProceduralTerrain>(terrain_->GetRenderable())
		->ReflectionPlane(checked_pointer_cast<OceanObject>(ocean_)->OceanPlane());

	sky_box_ = MakeSharedPtr<SceneObjectFoggySkyBox>();
	checked_pointer_cast<SceneObjectFoggySkyBox>(sky_box_)->CompressedCubeMap(y_cube, c_cube);
	checked_pointer_cast<SceneObjectFoggySkyBox>(sky_box_)->FogColor(fog_color);
	sky_box_->AddToSceneManager();

	sun_flare_ = MakeSharedPtr<LensFlareSceneObject>();
	checked_pointer_cast<LensFlareSceneObject>(sun_flare_)->Direction(float3(-0.267835f, 0.0517653f, 0.960315f));
	sun_flare_->AddToSceneManager();

	fog_pp_ = SyncLoadPostProcess("Fog.ppml", "fog");
	fog_pp_->SetParam(1, float3(fog_color.r(), fog_color.g(), fog_color.b()));
	fog_pp_->SetParam(2, 1.0f / 5000);
	fog_pp_->SetParam(3, this->ActiveCamera().FarPlane());
	deferred_rendering_->AtmosphericPostProcess(fog_pp_);

	light_shaft_pp_ = MakeSharedPtr<LightShaftPostProcess>();
	light_shaft_pp_->SetParam(1, sun_light_->Color());

	Camera& scene_camera = this->ActiveCamera();
	reflection_fb_ = Context::Instance().RenderFactoryInstance().MakeFrameBuffer();
	reflection_fb_->GetViewport()->camera->ProjParams(scene_camera.FOV(), scene_camera.Aspect(),
		scene_camera.NearPlane(), scene_camera.FarPlane());

	fpcController_.Scalers(0.05f, 1.0f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&OceanApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
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

	dialog_params_->Control<UISlider>(id_dmap_dim_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::DMapDimChangedHandler, this, std::placeholders::_1));
	this->DMapDimChangedHandler(*dialog_params_->Control<UISlider>(id_dmap_dim_slider_));

	dialog_params_->Control<UISlider>(id_patch_length_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::PatchLengthChangedHandler, this, std::placeholders::_1));
	this->PatchLengthChangedHandler(*dialog_params_->Control<UISlider>(id_patch_length_slider_));

	dialog_params_->Control<UISlider>(id_time_scale_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::TimeScaleChangedHandler, this, std::placeholders::_1));
	this->TimeScaleChangedHandler(*dialog_params_->Control<UISlider>(id_time_scale_slider_));

	dialog_params_->Control<UISlider>(id_wave_amplitude_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::WaveAmplitudeChangedHandler, this, std::placeholders::_1));
	this->WaveAmplitudeChangedHandler(*dialog_params_->Control<UISlider>(id_wave_amplitude_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_x_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::WindSpeedXChangedHandler, this, std::placeholders::_1));
	this->WindSpeedXChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_x_slider_));

	dialog_params_->Control<UISlider>(id_wind_speed_y_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::WindSpeedYChangedHandler, this, std::placeholders::_1));
	this->WindSpeedYChangedHandler(*dialog_params_->Control<UISlider>(id_wind_speed_y_slider_));

	dialog_params_->Control<UISlider>(id_wind_dependency_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::WindDependencyChangedHandler, this, std::placeholders::_1));
	this->WindDependencyChangedHandler(*dialog_params_->Control<UISlider>(id_wind_dependency_slider_));

	dialog_params_->Control<UISlider>(id_choppy_scale_slider_)->OnValueChangedEvent().connect(std::bind(&OceanApp::ChoppyScaleChangedHandler, this, std::placeholders::_1));
	this->ChoppyScaleChangedHandler(*dialog_params_->Control<UISlider>(id_choppy_scale_slider_));

	dialog_params_->Control<UICheckBox>(id_light_shaft_)->OnChangedEvent().connect(std::bind(&OceanApp::LightShaftHandler, this, std::placeholders::_1));

	dialog_params_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(std::bind(&OceanApp::FPSCameraHandler, this, std::placeholders::_1));
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
	reflection_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*reflection_tex_, 0, 1, 0));
	reflection_fb_->Attach(FrameBuffer::ATT_DepthStencil, rf.Make2DDepthStencilRenderView(*reflection_ds_tex_, 0, 1, 0));

	deferred_rendering_->SetupViewport(0, reflection_fb_,
		VPAM_NoTransparencyBack | VPAM_NoTransparencyFront | VPAM_NoSimpleForward | VPAM_NoGI | VPAM_NoSSVO);

	screen_camera_ = re.CurFrameBuffer()->GetViewport()->camera;
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

	checked_pointer_cast<OceanObject>(ocean_)->DMapDim(dmap_dim);
}

void OceanApp::PatchLengthChangedHandler(UISlider const & sender)
{
	float patch_length = sender.GetValue() * 0.5f;

	std::wostringstream stream;
	stream << L"Patch length: " << patch_length;
	dialog_params_->Control<UIStatic>(id_patch_length_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->PatchLength(patch_length);
}

void OceanApp::TimeScaleChangedHandler(UISlider const & sender)
{
	float time_scale = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Time scale: " << time_scale;
	dialog_params_->Control<UIStatic>(id_time_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->TimeScale(time_scale);
}

void OceanApp::WaveAmplitudeChangedHandler(UISlider const & sender)
{
	float wave_amp = sender.GetValue() * 0.0001f;

	std::wostringstream stream;
	stream << L"Wave amplitude: " << wave_amp;
	dialog_params_->Control<UIStatic>(id_wave_amplitude_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WaveAmplitude(wave_amp);
}

void OceanApp::WindSpeedXChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed X: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_x_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindSpeedX(wind_speed);
}

void OceanApp::WindSpeedYChangedHandler(UISlider const & sender)
{
	float wind_speed = sender.GetValue() * 0.05f;

	std::wostringstream stream;
	stream << L"Wind speed Y: " << wind_speed;
	dialog_params_->Control<UIStatic>(id_wind_speed_y_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindSpeedY(wind_speed);
}

void OceanApp::WindDependencyChangedHandler(UISlider const & sender)
{
	float dep = sender.GetValue() * 0.01f;

	std::wostringstream stream;
	stream << L"Wind dependency: " << dep;
	dialog_params_->Control<UIStatic>(id_wind_dependency_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->WindDependency(dep);
}

void OceanApp::ChoppyScaleChangedHandler(UISlider const & sender)
{
	float choppy = sender.GetValue() * 0.1f;

	std::wostringstream stream;
	stream << L"Choppy scale: " << choppy;
	dialog_params_->Control<UIStatic>(id_choppy_scale_static_)->SetText(stream.str());

	checked_pointer_cast<OceanObject>(ocean_)->ChoppyScale(choppy);
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
}

uint32_t OceanApp::DoUpdate(uint32_t pass)
{
	if (0 == deferred_rendering_->ActiveViewport())
	{
		ocean_->Visible(false);

		float3 reflect_eye, reflect_at, reflect_up;
		checked_pointer_cast<OceanObject>(ocean_)->ReflectViewParams(reflect_eye, reflect_at, reflect_up,
			screen_camera_->EyePos(), screen_camera_->LookAt(), screen_camera_->UpVec());
		reflection_fb_->GetViewport()->camera->ViewParams(reflect_eye, reflect_at, reflect_up);
	}
	else
	{
		ocean_->Visible(true);

		checked_pointer_cast<OceanObject>(ocean_)->ReflectionTex(reflection_tex_);
	}

	uint32_t ret = deferred_rendering_->Update(pass);
	if (ret & App3DFramework::URV_Finished)
	{
		if (light_shaft_on_)
		{
			light_shaft_pp_->SetParam(0, -sun_light_->Direction() * 10000.0f + this->ActiveCamera().EyePos());
			light_shaft_pp_->InputPin(0, deferred_rendering_->PrevFrameShadingTex(deferred_rendering_->ActiveViewport()));
			light_shaft_pp_->InputPin(1, deferred_rendering_->PrevFrameDepthTex(deferred_rendering_->ActiveViewport()));
			light_shaft_pp_->Apply();
		}
	}

	return ret;
}
