/**
 * @file ParticleSystem.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KFL/Hash.hpp>

#include <fstream>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/algorithm/string/split.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#include <boost/algorithm/string/trim.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/ParticleSystem.hpp>

namespace
{
	using namespace KlayGE;

	uint32_t const NUM_PARTICLES = 4096;

	class ParticleSystemLoadingDesc : public ResLoadingDesc
	{
	private:
		struct ParticleSystemDesc
		{
			std::string res_name;

			struct ParticleSystemData
			{
				std::wstring name;

				std::string particle_alpha_from_tex;
				std::string particle_alpha_to_tex;
				Color particle_color_from;
				Color particle_color_to;

				std::string emitter_type;
				float frequency;
				float angle;
				float3 min_pos;
				float3 max_pos;
				float min_vel;
				float max_vel;
				float min_life;
				float max_life;

				std::string updater_type;
				std::vector<KlayGE::float2> size_over_life_ctrl_pts;
				std::vector<KlayGE::float2> mass_over_life_ctrl_pts;
				std::vector<KlayGE::float2> opacity_over_life_ctrl_pts;
			};
			std::shared_ptr<ParticleSystemData> ps_data;

			std::shared_ptr<ParticleSystemPtr> ps;
		};

	public:
		explicit ParticleSystemLoadingDesc(std::string const & res_name)
		{
			ps_desc_.res_name = res_name;
			ps_desc_.ps_data = MakeSharedPtr<ParticleSystemDesc::ParticleSystemData>();
			ps_desc_.ps = MakeSharedPtr<ParticleSystemPtr>();
		}

		uint64_t Type() const override
		{
			static uint64_t const type = CT_HASH("ParticleSystemLoadingDesc");
			return type;
		}

		bool StateLess() const override
		{
			return false;
		}

		void SubThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);

			if (*ps_desc_.ps)
			{
				return;
			}

			ResIdentifierPtr psmm_input = ResLoader::Instance().Open(ps_desc_.res_name);

			KlayGE::XMLDocument doc;
			XMLNodePtr root = doc.Parse(psmm_input);

			{
				XMLNodePtr particle_node = root->FirstNode("particle");
				{
					XMLNodePtr alpha_node = particle_node->FirstNode("alpha");
					ps_desc_.ps_data->particle_alpha_from_tex = alpha_node->Attrib("from")->ValueString();
					ps_desc_.ps_data->particle_alpha_to_tex = alpha_node->Attrib("to")->ValueString();
				}
				{
					XMLNodePtr color_node = particle_node->FirstNode("color");
					{
						Color from;
						XMLAttributePtr attr = color_node->Attrib("from");
						if (attr)
						{
							std::string_view const value_str = attr->ValueString();
							std::vector<std::string> strs;
							boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
							for (size_t i = 0; i < 3; ++ i)
							{
								if (i < strs.size())
								{
									boost::algorithm::trim(strs[i]);
									from[i] = static_cast<float>(atof(strs[i].c_str()));
								}
								else
								{
									from[i] = 0;
								}
							}
						}
						from.a() = 1;
						ps_desc_.ps_data->particle_color_from = from;

						Color to;
						attr = color_node->Attrib("to");
						if (attr)
						{
							std::string_view const value_str = attr->ValueString();
							std::vector<std::string> strs;
							boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
							for (size_t i = 0; i < 3; ++ i)
							{
								if (i < strs.size())
								{
									boost::algorithm::trim(strs[i]);
									to[i] = static_cast<float>(atof(strs[i].c_str()));
								}
								else
								{
									to[i] = 0;
								}
							}
						}
						to.a() = 1;
						ps_desc_.ps_data->particle_color_to = to;
					}
				}
			}

			{
				XMLNodePtr emitter_node = root->FirstNode("emitter");

				XMLAttributePtr type_attr = emitter_node->Attrib("type");
				if (type_attr)
				{
					ps_desc_.ps_data->emitter_type = type_attr->ValueString();
				}
				else
				{
					ps_desc_.ps_data->emitter_type = "point";
				}

				XMLNodePtr freq_node = emitter_node->FirstNode("frequency");
				if (freq_node)
				{
					XMLAttributePtr attr = freq_node->Attrib("value");
					ps_desc_.ps_data->frequency = attr->ValueFloat();
				}

				XMLNodePtr angle_node = emitter_node->FirstNode("angle");
				if (angle_node)
				{
					XMLAttributePtr attr = angle_node->Attrib("value");
					ps_desc_.ps_data->angle = attr->ValueInt() * DEG2RAD;
				}

				XMLNodePtr pos_node = emitter_node->FirstNode("pos");
				if (pos_node)
				{
					float3 min_pos(0, 0, 0);
					XMLAttributePtr attr = pos_node->Attrib("min");
					if (attr)
					{
						std::string_view const value_str = attr->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
						for (size_t i = 0; i < 3; ++ i)
						{
							if (i < strs.size())
							{
								boost::algorithm::trim(strs[i]);
								min_pos[i] = static_cast<float>(atof(strs[i].c_str()));
							}
							else
							{
								min_pos[i] = 0;
							}
						}
					}
					ps_desc_.ps_data->min_pos = min_pos;
			
					float3 max_pos(0, 0, 0);
					attr = pos_node->Attrib("max");
					if (attr)
					{
						std::string_view const value_str = attr->ValueString();
						std::vector<std::string> strs;
						boost::algorithm::split(strs, value_str, boost::is_any_of(" "));
						for (size_t i = 0; i < 3; ++ i)
						{
							if (i < strs.size())
							{
								boost::algorithm::trim(strs[i]);
								max_pos[i] = static_cast<float>(atof(strs[i].c_str()));
							}
							else
							{
								max_pos[i] = 0;
							}
						}
					}			
					ps_desc_.ps_data->max_pos = max_pos;
				}

				XMLNodePtr vel_node = emitter_node->FirstNode("vel");
				if (vel_node)
				{
					XMLAttributePtr attr = vel_node->Attrib("min");
					ps_desc_.ps_data->min_vel = attr->ValueFloat();

					attr = vel_node->Attrib("max");
					ps_desc_.ps_data->max_vel = attr->ValueFloat();
				}

				XMLNodePtr life_node = emitter_node->FirstNode("life");
				if (life_node)
				{
					XMLAttributePtr attr = life_node->Attrib("min");
					ps_desc_.ps_data->min_life = attr->ValueFloat();

					attr = life_node->Attrib("max");
					ps_desc_.ps_data->max_life = attr->ValueFloat();
				}
			}

			{
				XMLNodePtr updater_node = root->FirstNode("updater");

				XMLAttributePtr type_attr = updater_node->Attrib("type");
				if (type_attr)
				{
					ps_desc_.ps_data->updater_type = type_attr->ValueString();
				}
				else
				{
					ps_desc_.ps_data->updater_type = "polyline";
				}

				if ("polyline" == ps_desc_.ps_data->updater_type)
				{
					for (XMLNodePtr node = updater_node->FirstNode("curve"); node; node = node->NextSibling("curve"))
					{
						std::vector<float2> xys;
						for (XMLNodePtr ctrl_point_node = node->FirstNode("ctrl_point"); ctrl_point_node; ctrl_point_node = ctrl_point_node->NextSibling("ctrl_point"))
						{
							XMLAttributePtr attr_x = ctrl_point_node->Attrib("x");
							XMLAttributePtr attr_y = ctrl_point_node->Attrib("y");

							xys.push_back(float2(attr_x->ValueFloat(), attr_y->ValueFloat()));
						}

						XMLAttributePtr attr = node->Attrib("name");
						std::string_view const name = attr->ValueString();
						size_t const name_hash = HashRange(name.begin(), name.end());
						if (CT_HASH("size_over_life") == name_hash)
						{
							ps_desc_.ps_data->size_over_life_ctrl_pts = xys;
						}
						else if (CT_HASH("mass_over_life") == name_hash)
						{
							ps_desc_.ps_data->mass_over_life_ctrl_pts = xys;
						}
						else if (CT_HASH("opacity_over_life") == name_hash)
						{
							ps_desc_.ps_data->opacity_over_life_ctrl_pts = xys;
						}
					}
				}
			}

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();
			RenderDeviceCaps const & caps = rf.RenderEngineInstance().DeviceCaps();
			if (caps.multithread_res_creating_support)
			{
				this->MainThreadStageNoLock();
			}
		}

		void MainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(main_thread_stage_mutex_);
			this->MainThreadStageNoLock();
		}

		bool HasSubThreadStage() const override
		{
			return true;
		}

		bool Match(ResLoadingDesc const & rhs) const override
		{
			if (this->Type() == rhs.Type())
			{
				ParticleSystemLoadingDesc const & psld = static_cast<ParticleSystemLoadingDesc const &>(rhs);
				return (ps_desc_.res_name == psld.ps_desc_.res_name);
			}
			return false;
		}

		void CopyDataFrom(ResLoadingDesc const & rhs) override
		{
			BOOST_ASSERT(this->Type() == rhs.Type());

			ParticleSystemLoadingDesc const & psld = static_cast<ParticleSystemLoadingDesc const &>(rhs);
			ps_desc_.res_name = psld.ps_desc_.res_name;
			ps_desc_.ps_data = psld.ps_desc_.ps_data;
			ps_desc_.ps = psld.ps_desc_.ps;
		}

		std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
		{
			ParticleSystemPtr rhs_pp = std::static_pointer_cast<ParticleSystem>(resource);
			return std::static_pointer_cast<void>(rhs_pp->Clone());
		}

		std::shared_ptr<void> Resource() const override
		{
			return *ps_desc_.ps;
		}

	private:
		void MainThreadStageNoLock()
		{
			if (!*ps_desc_.ps)
			{
				ParticleSystemPtr ps = MakeSharedPtr<ParticleSystem>(NUM_PARTICLES);

				ps->ParticleAlphaFromTex(ps_desc_.ps_data->particle_alpha_from_tex);
				ps->ParticleAlphaToTex(ps_desc_.ps_data->particle_alpha_to_tex);
				ps->ParticleColorFrom(ps_desc_.ps_data->particle_color_from);
				ps->ParticleColorTo(ps_desc_.ps_data->particle_color_to);

				ParticleEmitterPtr emitter = ps->MakeEmitter(ps_desc_.ps_data->emitter_type);
				ps->AddEmitter(emitter);

				emitter->Frequency(ps_desc_.ps_data->frequency);
				emitter->EmitAngle(ps_desc_.ps_data->angle);
				emitter->MinPosition(ps_desc_.ps_data->min_pos);
				emitter->MaxPosition(ps_desc_.ps_data->max_pos);
				emitter->MinVelocity(ps_desc_.ps_data->min_vel);
				emitter->MaxVelocity(ps_desc_.ps_data->max_vel);
				emitter->MinLife(ps_desc_.ps_data->min_life);
				emitter->MaxLife(ps_desc_.ps_data->max_life);

				ParticleUpdaterPtr updater = ps->MakeUpdater(ps_desc_.ps_data->updater_type);
				ps->AddUpdater(updater);
				checked_pointer_cast<PolylineParticleUpdater>(updater)->SizeOverLife(ps_desc_.ps_data->size_over_life_ctrl_pts);
				checked_pointer_cast<PolylineParticleUpdater>(updater)->MassOverLife(ps_desc_.ps_data->mass_over_life_ctrl_pts);
				checked_pointer_cast<PolylineParticleUpdater>(updater)->OpacityOverLife(ps_desc_.ps_data->opacity_over_life_ctrl_pts);

				*ps_desc_.ps = ps;
			}
		}

	private:
		ParticleSystemDesc ps_desc_;
		std::mutex main_thread_stage_mutex_;
	};
	
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct ParticleInstance
	{
		float3 pos;
		float life;
		float spin;
		float size;
		float life_factor;
		float alpha;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class RenderParticles : public RenderableHelper
	{
	public:
		explicit RenderParticles(bool gs_support)
			: RenderableHelper(L"Particles")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("Particle.fxml");

			rl_ = rf.MakeRenderLayout();
			if (gs_support)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write,
					sizeof(ParticleInstance), nullptr);
				rl_->BindVertexStream(pos_vb,
					{ VertexElement(VEU_Position, 0, EF_ABGR32F), VertexElement(VEU_TextureCoord, 0, EF_ABGR32F) });

				simple_forward_tech_ = effect_->TechniqueByName("ParticleWithGS");
				vdm_tech_ = effect_->TechniqueByName("ParticleWithGSVDM");
			}
			else
			{
				float2 texs[] =
				{
					float2(-1.0f, 1.0f),
					float2(1.0f, 1.0f),
					float2(-1.0f, -1.0f),
					float2(1.0f, -1.0f)
				};

				uint16_t indices[] =
				{
					0, 1, 2, 3
				};

				rl_->TopologyType(RenderLayout::TT_TriangleStrip);

				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					sizeof(texs), texs);
				rl_->BindVertexStream(tex_vb, VertexElement(VEU_Position, 0, EF_GR32F),
					RenderLayout::ST_Geometry, 0);

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write,
					sizeof(ParticleInstance), nullptr);
				rl_->BindVertexStream(pos_vb,
					{ VertexElement(VEU_TextureCoord, 0, EF_ABGR32F), VertexElement(VEU_TextureCoord, 1, EF_ABGR32F) },
					RenderLayout::ST_Instance);

				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
					sizeof(indices), indices);
				rl_->BindIndexStream(ib, EF_R16UI);

				simple_forward_tech_ = effect_->TechniqueByName("Particle");
				vdm_tech_ = effect_->TechniqueByName("ParticleVDM");
			}
			technique_ = simple_forward_tech_;

			effect_attrs_ |= EA_VDM;
		}

		void SceneDepthTexture(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("depth_tex")) = tex;
		}

		void ParticleColorFrom(Color const & clr)
		{
			*(effect_->ParameterByName("particle_color_from")) = float3(clr.r(), clr.g(), clr.b());
		}

		void ParticleColorTo(Color const & clr)
		{
			*(effect_->ParameterByName("particle_color_to")) = float3(clr.r(), clr.g(), clr.b());
		}

		void ParticleAlphaFrom(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("particle_alpha_from_tex")) = tex;
		}

		void ParticleAlphaTo(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("particle_alpha_to_tex")) = tex;
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();

			*(effect_->ParameterByName("model_view")) = model_mat_ * view;
			*(effect_->ParameterByName("proj")) = proj;
			*(effect_->ParameterByName("far_plane")) = camera.FarPlane();

			float scale_x = sqrt(model_mat_(0, 0) * model_mat_(0, 0) + model_mat_(0, 1) * model_mat_(0, 1) + model_mat_(0, 2) * model_mat_(0, 2));
			float scale_y = sqrt(model_mat_(1, 0) * model_mat_(1, 0) + model_mat_(1, 1) * model_mat_(1, 1) + model_mat_(1, 2) * model_mat_(1, 2));
			*(effect_->ParameterByName("point_radius")) = 0.08f * std::max(scale_x, scale_y);

			auto drl = Context::Instance().DeferredRenderingLayerInstance();
			if (drl)
			{
				*(effect_->ParameterByName("depth_tex")) = drl->CurrFrameDepthTex(drl->ActiveViewport());
			}
		}

		void PosBound(AABBox const & pos_aabb)
		{
			pos_aabb_ = pos_aabb;
		}

		using RenderableHelper::PosBound;
	};
}

namespace KlayGE
{
	ParticleEmitter::ParticleEmitter(SceneObjectPtr const & ps)
			: ps_(checked_pointer_cast<ParticleSystem>(ps)),
				model_mat_(float4x4::Identity()),
				min_spin_(-PI / 2), max_spin_(+PI / 2)
	{
	}

	uint32_t ParticleEmitter::Update(float elapsed_time)
	{
		return static_cast<uint32_t>(elapsed_time * emit_freq_ + 0.5f);
	}

	void ParticleEmitter::DoClone(ParticleEmitterPtr const & rhs)
	{
		rhs->ps_ = ps_;

		rhs->emit_freq_ = emit_freq_;

		rhs->model_mat_ = model_mat_;
		rhs->emit_angle_ = emit_angle_;

		rhs->min_pos_ = min_pos_;
		rhs->max_pos_ = max_pos_;
		rhs->min_vel_ = min_vel_;
		rhs->max_vel_ = max_vel_;
		rhs->min_life_ = min_life_;
		rhs->max_life_ = max_life_;
		rhs->min_spin_ = min_spin_;
		rhs->max_spin_ = max_spin_;
		rhs->min_size_ = min_size_;
		rhs->max_size_ = max_size_;
	}


	ParticleUpdater::ParticleUpdater(SceneObjectPtr const & ps)
		: ps_(checked_pointer_cast<ParticleSystem>(ps))
	{
	}

	void ParticleUpdater::DoClone(ParticleUpdaterPtr const & rhs)
	{
		rhs->ps_ = ps_;
	}


	ParticleSystem::ParticleSystem(uint32_t max_num_particles)
		: SceneObjectHelper(SOA_Moveable | SOA_NotCastShadow),
			particles_(max_num_particles),
			gravity_(0.5f), force_(0, 0, 0), media_density_(0.0f)
	{
		this->ClearParticles();

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		gs_support_ = rf.RenderEngineInstance().DeviceCaps().gs_support;
		renderable_ = MakeSharedPtr<RenderParticles>(gs_support_);
	}

	ParticleSystemPtr ParticleSystem::Clone()
	{
		ParticleSystemPtr ret = MakeSharedPtr<ParticleSystem>(NUM_PARTICLES);

		ret->emitters_.resize(emitters_.size());
		for (size_t i = 0; i < emitters_.size(); ++ i)
		{
			ret->emitters_[i] = emitters_[i]->Clone();
		}

		ret->updaters_.resize(updaters_.size());
		for (size_t i = 0; i < updaters_.size(); ++ i)
		{
			ret->updaters_[i] = updaters_[i]->Clone();
		}

		ret->gravity_ = gravity_;
		ret->force_ = force_;
		ret->media_density_ = media_density_;

		ret->particle_alpha_from_tex_name_ = particle_alpha_from_tex_name_;
		ret->particle_alpha_to_tex_name_ = particle_alpha_to_tex_name_;
		ret->particle_color_from_ = particle_color_from_;
		ret->particle_color_to_ = particle_color_to_;

		return ret;
	}

	ParticleEmitterPtr ParticleSystem::MakeEmitter(std::string_view name)
	{
		ParticleEmitterPtr ret;
		if ("point" == name)
		{
			ret = MakeSharedPtr<PointParticleEmitter>(this->shared_from_this());
		}
		else
		{
			KFL_UNREACHABLE("Unsupported emitter type");
		}

		return ret;
	}

	ParticleUpdaterPtr ParticleSystem::MakeUpdater(std::string_view name)
	{
		ParticleUpdaterPtr ret;
		if ("polyline" == name)
		{
			ret = MakeSharedPtr<PolylineParticleUpdater>(this->shared_from_this());
		}
		else
		{
			KFL_UNREACHABLE("Unsupported updater type");
		}

		return ret;
	}

	void ParticleSystem::AddEmitter(ParticleEmitterPtr const & emitter)
	{
		emitters_.push_back(emitter);
	}

	void ParticleSystem::DelEmitter(ParticleEmitterPtr const & emitter)
	{
		auto iter = std::find(emitters_.begin(), emitters_.end(), emitter);
		if (iter != emitters_.end())
		{
			emitters_.erase(iter);
		}
	}

	void ParticleSystem::ClearEmitters()
	{
		emitters_.clear();
	}

	void ParticleSystem::AddUpdater(ParticleUpdaterPtr const & updater)
	{
		updaters_.push_back(updater);
	}

	void ParticleSystem::DelUpdater(ParticleUpdaterPtr const & updater)
	{
		auto iter = std::find(updaters_.begin(), updaters_.end(), updater);
		if (iter != updaters_.end())
		{
			updaters_.erase(iter);
		}
	}

	void ParticleSystem::ClearUpdaters()
	{
		updaters_.clear();
	}

	uint32_t ParticleSystem::NumActiveParticles() const
	{
		std::lock_guard<std::mutex> lock(actived_particles_mutex_);
		return static_cast<uint32_t>(actived_particles_.size());
	}

	uint32_t ParticleSystem::GetActiveParticleIndex(uint32_t i) const
	{
		std::lock_guard<std::mutex> lock(actived_particles_mutex_);
		return actived_particles_[i].first;
	}

	void ParticleSystem::ClearParticles()
	{
		for (auto& particle : particles_)
		{
			particle.life = 0;
		}
	}

	void ParticleSystem::UpdateParticlesNoLock(float elapsed_time, std::vector<std::pair<uint32_t, float>>& actived_particles)
	{
		auto emitter_iter = emitters_.begin();
		uint32_t new_particle = (*emitter_iter)->Update(elapsed_time);

		float4x4 const & view_mat = Context::Instance().AppInstance().ActiveCamera().ViewMatrix();

		actived_particles.clear();

		float3 min_bb(+1e10f, +1e10f, +1e10f);
		float3 max_bb(-1e10f, -1e10f, -1e10f);

		for (uint32_t i = 0; i < particles_.size(); ++ i)
		{
			Particle& particle = particles_[i];

			if (particle.life > 0)
			{
				for (auto const & updater : updaters_)
				{
					updater->Update(particle, elapsed_time);
				}
			}
			else
			{
				if (new_particle > 0)
				{
					(*emitter_iter)->Emit(particle);
					for (auto const & updater : updaters_)
					{
						updater->Update(particle, 0);
					}
					-- new_particle;
				}
				else
				{
					if (emitter_iter != emitters_.end())
					{
						++ emitter_iter;
						if (emitter_iter != emitters_.end())
						{
							new_particle = (*emitter_iter)->Update(elapsed_time);
						}
					}
				}
			}

			if (particle.life > 0)
			{
				float3 const & pos = particle.pos;
				float p_to_v = (pos.x() * view_mat(0, 2) + pos.y() * view_mat(1, 2) + pos.z() * view_mat(2, 2) + view_mat(3, 2))
					/ (pos.x() * view_mat(0, 3) + pos.y() * view_mat(1, 3) + pos.z() * view_mat(2, 3) + view_mat(3, 3));

				actived_particles.emplace_back(i, p_to_v);

				min_bb = MathLib::minimize(min_bb, pos);
				max_bb = MathLib::maximize(min_bb, pos);
			}
		}

		if (!actived_particles.empty())
		{
			std::sort(actived_particles.begin(), actived_particles.end(),
				[](std::pair<uint32_t, float> const & lhs, std::pair<uint32_t, float> const & rhs)
				{
					return lhs.second > rhs.second;
				});

			checked_pointer_cast<RenderParticles>(renderable_)->PosBound(AABBox(min_bb, max_bb));
		}
	}

	void ParticleSystem::UpdateParticleBufferNoLock(std::vector<std::pair<uint32_t, float>> const & actived_particles)
	{
		if (!actived_particles.empty())
		{
			RenderLayout& rl = renderable_->GetRenderLayout();

			GraphicsBufferPtr instance_gb;
			if (gs_support_)
			{
				instance_gb = rl.GetVertexStream(0);
			}
			else
			{
				instance_gb = rl.InstanceStream();
			}

			uint32_t const num_active_particles = static_cast<uint32_t>(actived_particles.size());
			uint32_t const new_instance_size = num_active_particles * sizeof(ParticleInstance);
			if (!instance_gb || (instance_gb->Size() < new_instance_size))
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				instance_gb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write,
					new_instance_size, nullptr);

				if (gs_support_)
				{
					rl.SetVertexStream(0, instance_gb);
				}
				else
				{
					rl.InstanceStream(instance_gb);
				}
			}

			if (gs_support_)
			{
				rl.NumVertices(num_active_particles);
			}
			else
			{
				for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
				{
					rl.VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, num_active_particles);
				}
			}

			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				ParticleInstance* instance_data = mapper.Pointer<ParticleInstance>();
				for (uint32_t i = 0; i < num_active_particles; ++ i, ++ instance_data)
				{
					Particle const & par = particles_[actived_particles[i].first];
					instance_data->pos = par.pos;
					instance_data->life = par.life;
					instance_data->spin = par.spin;
					instance_data->size = par.size;
					instance_data->life_factor = (par.init_life - par.life) / par.init_life;
					instance_data->alpha = par.alpha;
				}
			}
		}
	}

	void ParticleSystem::SubThreadUpdate(float app_time, float elapsed_time)
	{
		KFL_UNUSED(app_time);

		std::lock_guard<std::mutex> lock(actived_particles_mutex_);

		this->UpdateParticlesNoLock(elapsed_time, actived_particles_);

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const & caps = rf.RenderEngineInstance().DeviceCaps();
		if (caps.arbitrary_multithread_rendering_support)
		{
			this->UpdateParticleBufferNoLock(actived_particles_);
		}
	}

	bool ParticleSystem::MainThreadUpdate(float app_time, float elapsed_time)
	{
		KFL_UNUSED(app_time);
		KFL_UNUSED(elapsed_time);

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto const & caps = rf.RenderEngineInstance().DeviceCaps();
		if (!caps.arbitrary_multithread_rendering_support)
		{
			std::lock_guard<std::mutex> lock(actived_particles_mutex_);
			this->UpdateParticleBufferNoLock(actived_particles_);
		}

		return false;
	}

	void ParticleSystem::ParticleAlphaFromTex(std::string const & tex_name)
	{
		particle_alpha_from_tex_name_ = tex_name;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaFrom(
			SyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable));
	}

	void ParticleSystem::ParticleAlphaToTex(std::string const & tex_name)
	{
		particle_alpha_to_tex_name_ = tex_name;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleAlphaTo(
			SyncLoadTexture(tex_name, EAH_GPU_Read | EAH_Immutable));
	}

	void ParticleSystem::ParticleColorFrom(Color const & clr)
	{
		particle_color_from_ = clr;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorFrom(clr);
	}

	void ParticleSystem::ParticleColorTo(Color const & clr)
	{
		particle_color_to_ = clr;
		checked_pointer_cast<RenderParticles>(renderable_)->ParticleColorTo(clr);
	}

	void ParticleSystem::SceneDepthTexture(TexturePtr const & depth_tex)
	{
		checked_pointer_cast<RenderParticles>(renderable_)->SceneDepthTexture(depth_tex);
	}


	ParticleSystemPtr SyncLoadParticleSystem(std::string const & psml_name)
	{
		return ResLoader::Instance().SyncQueryT<ParticleSystem>(MakeSharedPtr<ParticleSystemLoadingDesc>(psml_name));
	}

	ParticleSystemPtr ASyncLoadParticleSystem(std::string const & psml_name)
	{
		// TODO: Make it really async
		return ResLoader::Instance().SyncQueryT<ParticleSystem>(MakeSharedPtr<ParticleSystemLoadingDesc>(psml_name));
	}

	void SaveParticleSystem(ParticleSystemPtr const & ps, std::string const & psml_name)
	{
		KlayGE::XMLDocument doc;

		XMLNodePtr root = doc.AllocNode(XNT_Element, "particle_system");
		doc.RootNode(root);

		{
			XMLNodePtr particle_node = doc.AllocNode(XNT_Element, "particle");
			{
				XMLNodePtr alpha_node = doc.AllocNode(XNT_Element, "alpha");
				alpha_node->AppendAttrib(doc.AllocAttribString("from", ps->ParticleAlphaFromTex()));
				alpha_node->AppendAttrib(doc.AllocAttribString("to", ps->ParticleAlphaToTex()));
				particle_node->AppendNode(alpha_node);
			}
			{
				XMLNodePtr color_node = doc.AllocNode(XNT_Element, "color");
				{
					Color const & from = ps->ParticleColorFrom();
					{
						std::string from_str = boost::lexical_cast<std::string>(from.r())
							+ ' ' + boost::lexical_cast<std::string>(from.g())
							+ ' ' + boost::lexical_cast<std::string>(from.b());
						color_node->AppendAttrib(doc.AllocAttribString("from", from_str));
					}
					Color const & to = ps->ParticleColorTo();
					{
						std::string to_str = boost::lexical_cast<std::string>(to.r())
							+ ' ' + boost::lexical_cast<std::string>(to.g())
							+ ' ' + boost::lexical_cast<std::string>(to.b());
						color_node->AppendAttrib(doc.AllocAttribString("to", to_str));
					}
				}
				particle_node->AppendNode(color_node);
			}
			root->AppendNode(particle_node);
		}

		for (uint32_t i = 0; i < ps->NumEmitters(); ++ i)
		{
			ParticleEmitterPtr const & particle_emitter = ps->Emitter(i);

			XMLNodePtr emitter_node = doc.AllocNode(XNT_Element, "emitter");
			emitter_node->AppendAttrib(doc.AllocAttribString("type", particle_emitter->Type()));

			{
				XMLNodePtr freq_node = doc.AllocNode(XNT_Element, "frequency");
				freq_node->AppendAttrib(doc.AllocAttribFloat("value", particle_emitter->Frequency()));
				emitter_node->AppendNode(freq_node);
			}
			{
				XMLNodePtr angle_node = doc.AllocNode(XNT_Element, "angle");
				angle_node->AppendAttrib(doc.AllocAttribInt("value", static_cast<int>(particle_emitter->EmitAngle() * RAD2DEG + 0.5f)));
				emitter_node->AppendNode(angle_node);
			}
			{
				XMLNodePtr pos_node = doc.AllocNode(XNT_Element, "pos");
				{
					std::string min_str = boost::lexical_cast<std::string>(particle_emitter->MinPosition().x())
						+ ' ' + boost::lexical_cast<std::string>(particle_emitter->MinPosition().y())
						+ ' ' + boost::lexical_cast<std::string>(particle_emitter->MinPosition().z());
					pos_node->AppendAttrib(doc.AllocAttribString("min", min_str));
				}
				{
					std::string max_str = boost::lexical_cast<std::string>(particle_emitter->MaxPosition().x())
						+ ' ' + boost::lexical_cast<std::string>(particle_emitter->MaxPosition().y())
						+ ' ' + boost::lexical_cast<std::string>(particle_emitter->MaxPosition().z());
					pos_node->AppendAttrib(doc.AllocAttribString("max", max_str));
				}
				emitter_node->AppendNode(pos_node);
			}		
			{
				XMLNodePtr vel_node = doc.AllocNode(XNT_Element, "vel");
				vel_node->AppendAttrib(doc.AllocAttribFloat("min", particle_emitter->MinVelocity()));
				vel_node->AppendAttrib(doc.AllocAttribFloat("max", particle_emitter->MaxVelocity()));
				emitter_node->AppendNode(vel_node);
			}
			{
				XMLNodePtr life_node = doc.AllocNode(XNT_Element, "life");
				life_node->AppendAttrib(doc.AllocAttribFloat("min", particle_emitter->MinLife()));
				life_node->AppendAttrib(doc.AllocAttribFloat("max", particle_emitter->MaxLife()));
				emitter_node->AppendNode(life_node);
			}
			root->AppendNode(emitter_node);
		}

		for (uint32_t i = 0; i < ps->NumUpdaters(); ++ i)
		{
			ParticleUpdaterPtr const & particle_updater = ps->Updater(i);

			XMLNodePtr updater_node = doc.AllocNode(XNT_Element, "updater");
			updater_node->AppendAttrib(doc.AllocAttribString("type", particle_updater->Type()));

			if ("polyline" == particle_updater->Type())
			{
				std::shared_ptr<PolylineParticleUpdater> polyline_updater = checked_pointer_cast<PolylineParticleUpdater>(particle_updater);

				XMLNodePtr size_over_life_node = doc.AllocNode(XNT_Element, "curve");
				size_over_life_node->AppendAttrib(doc.AllocAttribString("name", "size_over_life"));
				std::vector<float2> const & size_over_life = polyline_updater->SizeOverLife();
				for (size_t j = 0; j < size_over_life.size(); ++ j)
				{
					float2 const & pt = size_over_life[j];

					XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

					size_over_life_node->AppendNode(ctrl_point_node);
				}
				updater_node->AppendNode(size_over_life_node);

				XMLNodePtr mass_over_life_node = doc.AllocNode(XNT_Element, "curve");
				mass_over_life_node->AppendAttrib(doc.AllocAttribString("name", "mass_over_life"));
				std::vector<float2> const & mass_over_life = polyline_updater->MassOverLife();
				for (size_t j = 0; j < mass_over_life.size(); ++ j)
				{
					float2 const & pt = mass_over_life[j];

					XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

					mass_over_life_node->AppendNode(ctrl_point_node);
				}
				updater_node->AppendNode(mass_over_life_node);

				XMLNodePtr opacity_over_life_node = doc.AllocNode(XNT_Element, "curve");
				opacity_over_life_node->AppendAttrib(doc.AllocAttribString("name", "opacity_over_life"));
				std::vector<float2> const & opacity_over_life = polyline_updater->OpacityOverLife();
				for (size_t j = 0; j < opacity_over_life.size(); ++ j)
				{
					float2 const & pt = opacity_over_life[j];

					XMLNodePtr ctrl_point_node = doc.AllocNode(XNT_Element, "ctrl_point");
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("x", pt.x()));
					ctrl_point_node->AppendAttrib(doc.AllocAttribFloat("y", pt.y()));

					opacity_over_life_node->AppendNode(ctrl_point_node);
				}
				updater_node->AppendNode(opacity_over_life_node);
			}

			root->AppendNode(updater_node);
		}

		std::ofstream ofs(psml_name.c_str());
		if (!ofs)
		{
			ofs.open((ResLoader::Instance().LocalFolder() + psml_name).c_str());
		}
		doc.Print(ofs);
	}


	PointParticleEmitter::PointParticleEmitter(SceneObjectPtr const & ps)
		: ParticleEmitter(ps),
			random_dis_(0, 10000)
	{
	}

	std::string const & PointParticleEmitter::Type() const
	{
		static std::string const type("point");
		return type;
	}

	ParticleEmitterPtr PointParticleEmitter::Clone()
	{
		std::shared_ptr<PointParticleEmitter> ret = MakeSharedPtr<PointParticleEmitter>(ps_.lock());
		this->DoClone(ret);
		return ret;
	}

	void PointParticleEmitter::Emit(Particle& par)
	{
		par.pos.x() = MathLib::lerp(min_pos_.x(), max_pos_.x(), this->RandomGen());
		par.pos.y() = MathLib::lerp(min_pos_.y(), max_pos_.y(), this->RandomGen());
		par.pos.z() = MathLib::lerp(min_pos_.z(), max_pos_.z(), this->RandomGen());
		par.pos = MathLib::transform_coord(par.pos, model_mat_);
		float theta = (this->RandomGen() * 2 - 1) * PI;
		float phi = this->RandomGen() * emit_angle_ / 2;
		float velocity = MathLib::lerp(min_vel_, max_vel_, this->RandomGen());
		float vx = cos(theta) * sin(phi);
		float vz = sin(theta) * sin(phi);
		float vy = cos(phi);
		par.vel = MathLib::transform_normal(float3(vx, vy, vz) * velocity, model_mat_);
		par.life = MathLib::lerp(min_life_, max_life_, this->RandomGen());
		par.spin = MathLib::lerp(min_spin_, max_spin_, this->RandomGen());
		par.size = MathLib::lerp(min_size_, max_size_, this->RandomGen());
		par.init_life = par.life;
	}

	float PointParticleEmitter::RandomGen()
	{
		return MathLib::clamp(random_dis_(gen_) * 0.0001f, 0.0f, 1.0f);
	}


	PolylineParticleUpdater::PolylineParticleUpdater(SceneObjectPtr const & ps)
		: ParticleUpdater(ps)
	{
	}

	std::string const & PolylineParticleUpdater::Type() const
	{
		static std::string const type("polyline");
		return type;
	}

	ParticleUpdaterPtr PolylineParticleUpdater::Clone()
	{
		std::shared_ptr<PolylineParticleUpdater> ret = MakeSharedPtr<PolylineParticleUpdater>(ps_.lock());
		this->DoClone(ret);
		ret->size_over_life_ = size_over_life_;
		ret->mass_over_life_ = mass_over_life_;
		ret->opacity_over_life_ = opacity_over_life_;
		return ret;
	}

	void PolylineParticleUpdater::Update(Particle& par, float elapse_time)
	{
		std::lock_guard<std::mutex> lock(update_mutex_);

		BOOST_ASSERT(!size_over_life_.empty());
		BOOST_ASSERT(!mass_over_life_.empty());
		BOOST_ASSERT(!opacity_over_life_.empty());

		float pos = (par.init_life - par.life) / par.init_life;

		float cur_size = size_over_life_.back().y();
		for (auto iter = size_over_life_.begin(); iter != size_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_size = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		float cur_mass = mass_over_life_.back().y();
		for (auto iter = mass_over_life_.begin(); iter != mass_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_mass = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		float cur_alpha = opacity_over_life_.back().y();
		for (auto iter = opacity_over_life_.begin(); iter != opacity_over_life_.end() - 1; ++ iter)
		{
			if ((iter + 1)->x() >= pos)
			{
				float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
				cur_alpha = MathLib::lerp(iter->y(), (iter + 1)->y(), s);
				break;
			}
		}

		ParticleSystemPtr ps = ps_.lock();
		float buoyancy = 4.0f / 3 * PI * MathLib::cube(cur_size) * ps->MediaDensity() * ps->Gravity();
		float3 accel = (ps->Force() + float3(0, buoyancy, 0)) / cur_mass - float3(0, ps->Gravity(), 0);
		par.vel += accel * elapse_time;
		par.pos += par.vel * elapse_time;
		par.life -= elapse_time;
		par.spin += 0.001f;
		par.size = cur_size;
		par.alpha = cur_alpha;
	}
}
