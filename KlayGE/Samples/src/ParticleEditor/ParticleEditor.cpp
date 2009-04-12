#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/GraphicsBuffer.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <fstream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#pragma warning(push)
#pragma warning(disable: 4127 4512)
#include <boost/random.hpp>
#pragma warning(pop)
#include <boost/typeof/typeof.hpp>
#pragma warning(push)
#pragma warning(disable: 4702)
#include <boost/lexical_cast.hpp>
#pragma warning(pop)

#include <rapidxml/rapidxml.hpp>
#pragma warning(push)
#pragma warning(disable: 4100)
#include <rapidxml/rapidxml_print.hpp>
#pragma warning(pop)

#include "ParticleEditor.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	bool use_gs = false;

	class TerrainRenderable : public RenderableHelper
	{
	public:
		TerrainRenderable()
			: RenderableHelper(L"Terrain")
		{
			BOOST_AUTO(grass, LoadTexture("grass.dds", EAH_GPU_Read));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("ParticleEditor.kfx")->TechniqueByName("Terrain");

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			float3 vertices[] =
			{
				float3(-10, 0, +10),
				float3(+10, 0, +10),
				float3(-10, 0, -10),
				float3(+10, 0, -10),
			};

			ElementInitData init_data;
			init_data.row_pitch = sizeof(vertices);
			init_data.slice_pitch = 0;
			init_data.data = &vertices[0];
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

			box_ = MathLib::compute_bounding_box<float>(vertices, vertices + sizeof(vertices) / sizeof(vertices[0]));

			*(technique_->Effect().ParameterByName("grass_tex")) = grass();
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 view = app.ActiveCamera().ViewMatrix();
			float4x4 proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();
			*(technique_->Effect().ParameterByName("depth_min")) = camera.NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (camera.FarPlane() - camera.NearPlane());
		}
	};

	class TerrainObject : public SceneObjectHelper
	{
	public:
		TerrainObject()
			: SceneObjectHelper(RenderablePtr(new TerrainRenderable), SOA_Cullable)
		{
		}
	};

	int const NUM_PARTICLE = 16384;

	class RenderParticles : public RenderableHelper
	{
	public:
		RenderParticles()
			: RenderableHelper(L"Particles")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

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

			rl_ = rf.MakeRenderLayout();
			if (use_gs)
			{
				rl_->TopologyType(RenderLayout::TT_PointList);

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
				rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

				technique_ = rf.LoadEffect("ParticleEditor.kfx")->TechniqueByName("ParticleWithGS");
			}
			else
			{
				rl_->TopologyType(RenderLayout::TT_TriangleStrip);

				ElementInitData init_data;
				init_data.row_pitch = sizeof(texs);
				init_data.slice_pitch = 0;
				init_data.data = texs;
				GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
				rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

				GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);
				rl_->BindVertexStream(pos_vb,
					boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_ABGR32F)),
					RenderLayout::ST_Instance);

				init_data.row_pitch = sizeof(indices);
				init_data.slice_pitch = 0;
				init_data.data = indices;
				GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
				rl_->BindIndexStream(ib, EF_R16UI);

				technique_ = rf.LoadEffect("ParticleEditor.kfx")->TechniqueByName("Particle");
			}

			*(technique_->Effect().ParameterByName("point_radius")) = 0.08f;
		}

		void SceneTexture(TexturePtr const tex, bool flip)
		{
			*(technique_->Effect().ParameterByName("scene_tex")) = tex;
			*(technique_->Effect().ParameterByName("flip")) = static_cast<int32_t>(flip ? -1 : 1);
		}

		void ParticleTexture(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("particle_tex")) = tex;
		}

		void SetInitLife(float life)
		{
			*(technique_->Effect().ParameterByName("init_life")) = life;
		}

		void SetSizeOverLife(std::vector<float2> const & size_over_life)
		{
			*(technique_->Effect().ParameterByName("size_over_life")) = size_over_life;
			*(technique_->Effect().ParameterByName("num_size_over_life")) = static_cast<int32_t>(size_over_life.size());
		}

		void SetWeightOverLife(std::vector<float2> const & weight_over_life)
		{
			*(technique_->Effect().ParameterByName("weight_over_life")) = weight_over_life;
			*(technique_->Effect().ParameterByName("num_weight_over_life")) = static_cast<int32_t>(weight_over_life.size());
		}

		void SetTransparencyOverLife(std::vector<float2> const & transparency_over_life)
		{
			*(technique_->Effect().ParameterByName("transparency_over_life")) = transparency_over_life;
			*(technique_->Effect().ParameterByName("num_transparency_over_life")) = static_cast<int32_t>(transparency_over_life.size());
		}

		void OnRenderBegin()
		{
			Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

			float4x4 const & view = camera.ViewMatrix();
			float4x4 const & proj = camera.ProjMatrix();
			float4x4 const inv_proj = MathLib::inverse(proj);

			*(technique_->Effect().ParameterByName("View")) = view;
			*(technique_->Effect().ParameterByName("Proj")) = proj;

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float4 const & texel_to_pixel = re.TexelToPixelOffset() * 2;
			float const x_offset = texel_to_pixel.x() / re.CurFrameBuffer()->Width();
			float const y_offset = texel_to_pixel.y() / re.CurFrameBuffer()->Height();
			*(technique_->Effect().ParameterByName("offset")) = float2(x_offset, y_offset);

			*(technique_->Effect().ParameterByName("upper_left")) = MathLib::transform_coord(float3(-1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("upper_right")) = MathLib::transform_coord(float3(1, 1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_left")) = MathLib::transform_coord(float3(-1, -1, 1), inv_proj);
			*(technique_->Effect().ParameterByName("lower_right")) = MathLib::transform_coord(float3(1, -1, 1), inv_proj);

			*(technique_->Effect().ParameterByName("depth_min")) = camera.NearPlane();
			*(technique_->Effect().ParameterByName("inv_depth_range")) = 1 / (camera.FarPlane() - camera.NearPlane());
		}
	};

	class ParticlesObject : public SceneObjectHelper
	{
	public:
		ParticlesObject()
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_.reset(new RenderParticles);
		}

		void ParticleTexture(TexturePtr const & tex)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->ParticleTexture(tex);
		}

		void SetInitLife(float life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetInitLife(life);
		}

		void SetSizeOverLife(std::vector<float2> const & size_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetSizeOverLife(size_over_life);
		}

		void SetWeightOverLife(std::vector<float2> const & weight_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetWeightOverLife(weight_over_life);
		}

		void SetTransparencyOverLife(std::vector<float2> const & transparency_over_life)
		{
			checked_pointer_cast<RenderParticles>(renderable_)->SetTransparencyOverLife(transparency_over_life);
		}
	};

	class RenderPolyline : public Renderable
	{
	public:
		RenderPolyline()
			: name_(L"Polyline")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = rf.LoadEffect("ParticleEditor.kfx");

			*(effect_->ParameterByName("active_pt")) = static_cast<int32_t>(-1);

			// background
			techniques_[BACKGROUND_INDEX] = effect_->TechniqueByName("BackgroundTech");
			rls_[BACKGROUND_INDEX] = rf.MakeRenderLayout();
			rls_[BACKGROUND_INDEX]->TopologyType(RenderLayout::TT_TriangleStrip);
			{
				float2 xy[] = 
				{
					float2(0, 0),
					float2(1, 0),
					float2(0, 1),
					float2(1, 1)
				};
				ElementInitData init_data;
				init_data.data = &xy[0];
				init_data.slice_pitch = init_data.row_pitch = sizeof(xy);
				pos_vbs_[BACKGROUND_INDEX] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read, &init_data);
			}
			rls_[BACKGROUND_INDEX]->BindVertexStream(pos_vbs_[BACKGROUND_INDEX], boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

			// coord line
			techniques_[COORDLINE_INDEX] = effect_->TechniqueByName("CoordLineTech");
			rls_[COORDLINE_INDEX] = rf.MakeRenderLayout();
			rls_[COORDLINE_INDEX]->TopologyType(RenderLayout::TT_TriangleList);
			{
				ElementInitData init_data;
				
				std::vector<float4> vdata(4 * (21 + 11));
				for (size_t i = 0; i < 21; ++ i)
				{
					vdata[i * 4 + 0] = float4(i / 20.0f, 0, -0.5f, 0);
					vdata[i * 4 + 1] = float4(i / 20.0f, 0, +0.5f, 0);
					vdata[i * 4 + 2] = float4(i / 20.0f, 1, +0.5f, 0);
					vdata[i * 4 + 3] = float4(i / 20.0f, 1, -0.5f, 0);
				}
				vdata[10 * 4 + 0].z() = -1;
				vdata[10 * 4 + 1].z() = +1;
				vdata[10 * 4 + 2].z() = +1;
				vdata[10 * 4 + 3].z() = -1;
				for (size_t i = 0; i < 11; ++ i)
				{
					vdata[(i + 21) * 4 + 0] = float4(0, i / 10.0f, 0, -0.5f);
					vdata[(i + 21) * 4 + 1] = float4(1, i / 10.0f, 0, -0.5f);
					vdata[(i + 21) * 4 + 2] = float4(1, i / 10.0f, 0, +0.5f);
					vdata[(i + 21) * 4 + 3] = float4(0, i / 10.0f, 0, +0.5f);
				}
				vdata[26 * 4 + 0].w() = -1;
				vdata[26 * 4 + 1].w() = -1;
				vdata[26 * 4 + 2].w() = +1;
				vdata[26 * 4 + 3].w() = +1;
				init_data.data = &vdata[0];
				init_data.slice_pitch = init_data.row_pitch = vdata.size() * sizeof(vdata[0]);
				pos_vbs_[COORDLINE_INDEX] = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read, &init_data);
				rls_[COORDLINE_INDEX]->BindVertexStream(pos_vbs_[COORDLINE_INDEX], boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));

				std::vector<uint16_t> idata(6 * (21 + 11));
				for (size_t i = 0; i < 21 + 11; ++ i)
				{
					idata[i * 6 + 0] = static_cast<uint16_t>(i * 4 + 0);
					idata[i * 6 + 1] = static_cast<uint16_t>(i * 4 + 1);
					idata[i * 6 + 2] = static_cast<uint16_t>(i * 4 + 2);
					idata[i * 6 + 3] = static_cast<uint16_t>(i * 4 + 2);
					idata[i * 6 + 4] = static_cast<uint16_t>(i * 4 + 3);
					idata[i * 6 + 5] = static_cast<uint16_t>(i * 4 + 0);
				}
				init_data.data = &idata[0];
				init_data.slice_pitch = init_data.row_pitch = idata.size() * sizeof(idata[0]);
				ibs_[COORDLINE_INDEX] = rf.MakeIndexBuffer(BU_Dynamic, EAH_GPU_Read, &init_data);
				rls_[COORDLINE_INDEX]->BindIndexStream(ibs_[COORDLINE_INDEX], EF_R16UI);
			}
			
			// polyline
			techniques_[POLYLINE_INDEX] = effect_->TechniqueByName("PolylineTech");
			rls_[POLYLINE_INDEX] = rf.MakeRenderLayout();
			rls_[POLYLINE_INDEX]->TopologyType(RenderLayout::TT_TriangleList);
			pos_vbs_[POLYLINE_INDEX] = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rls_[POLYLINE_INDEX]->BindVertexStream(pos_vbs_[POLYLINE_INDEX], boost::make_tuple(vertex_element(VEU_Position, 0, EF_ABGR32F)));
			ibs_[POLYLINE_INDEX] = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rls_[POLYLINE_INDEX]->BindIndexStream(ibs_[POLYLINE_INDEX], EF_R16UI);

			// control points
			techniques_[CTRLPOINTS_INDEX] = effect_->TechniqueByName("ControlPointTech");
			rls_[CTRLPOINTS_INDEX] = rf.MakeRenderLayout();
			rls_[CTRLPOINTS_INDEX]->TopologyType(RenderLayout::TT_TriangleList);
			pos_vbs_[CTRLPOINTS_INDEX] = rf.MakeVertexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rls_[CTRLPOINTS_INDEX]->BindVertexStream(pos_vbs_[CTRLPOINTS_INDEX], boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			ibs_[CTRLPOINTS_INDEX] = rf.MakeIndexBuffer(BU_Dynamic, EAH_CPU_Write | EAH_GPU_Read, NULL);
			rls_[CTRLPOINTS_INDEX]->BindIndexStream(ibs_[CTRLPOINTS_INDEX], EF_R16UI);
		}

		RenderTechniquePtr const & GetRenderTechnique() const
		{
			return techniques_[0];
		}

		RenderLayoutPtr const & GetRenderLayout() const
		{
			return rls_[0];
		}

		Box const & GetBound() const
		{
			return box_;
		}

		std::wstring const & Name() const
		{
			return name_;
		}

		void OnRenderBegin()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			float const half_width = re.CurFrameBuffer()->Width() / 2.0f;
			float const half_height = re.CurFrameBuffer()->Height() / 2.0f;

			*(effect_->ParameterByName("half_width_height")) = float2(half_width, half_height);

			float4 texel_to_pixel = re.TexelToPixelOffset();
			texel_to_pixel.x() /= half_width;
			texel_to_pixel.y() /= half_height;
			*(effect_->ParameterByName("texel_to_pixel_offset")) = texel_to_pixel;
		}

		void SetCtrlPoints(std::vector<float2> const & ctrl_points)
		{
			if (!ctrl_points.empty())
			{
				pos_vbs_[POLYLINE_INDEX]->Resize((ctrl_points.size() - 1) * 4 * sizeof(float4));
				{
					GraphicsBuffer::Mapper mapper(*pos_vbs_[POLYLINE_INDEX], BA_Write_Only);
					float4* p = mapper.Pointer<float4>();
					for (size_t i = 0; i < ctrl_points.size() - 1; ++ i)
					{
						float2 dir = ctrl_points[i + 1] - ctrl_points[i + 0];
						dir = MathLib::normalize(float2(dir.y(), -dir.x())) / 2;
						p[i * 4 + 0] = float4(ctrl_points[i + 0].x(), ctrl_points[i + 0].y(), -dir.x(), -dir.y());
						p[i * 4 + 1] = float4(ctrl_points[i + 1].x(), ctrl_points[i + 1].y(), -dir.x(), -dir.y());
						p[i * 4 + 2] = float4(ctrl_points[i + 1].x(), ctrl_points[i + 1].y(), dir.x(), dir.y());
						p[i * 4 + 3] = float4(ctrl_points[i + 0].x(), ctrl_points[i + 0].y(), dir.x(), dir.y());
					}
				}
				ibs_[POLYLINE_INDEX]->Resize((ctrl_points.size() - 1) * 6 * sizeof(uint16_t));
				{
					GraphicsBuffer::Mapper mapper(*ibs_[POLYLINE_INDEX], BA_Write_Only);
					uint16_t* p = mapper.Pointer<uint16_t>();
					for (size_t i = 0; i < ctrl_points.size() - 1; ++ i)
					{
						p[i * 6 + 0] = static_cast<uint16_t>(i * 4 + 0);
						p[i * 6 + 1] = static_cast<uint16_t>(i * 4 + 1);
						p[i * 6 + 2] = static_cast<uint16_t>(i * 4 + 2);
						p[i * 6 + 3] = static_cast<uint16_t>(i * 4 + 2);
						p[i * 6 + 4] = static_cast<uint16_t>(i * 4 + 3);
						p[i * 6 + 5] = static_cast<uint16_t>(i * 4 + 0);
					}
				}

				pos_vbs_[CTRLPOINTS_INDEX]->Resize(4 * ctrl_points.size() * sizeof(float3));
				{
					GraphicsBuffer::Mapper mapper(*pos_vbs_[CTRLPOINTS_INDEX], BA_Write_Only);
					float3* p = mapper.Pointer<float3>();
					for (size_t i = 0; i < ctrl_points.size(); ++ i)
					{
						p[i * 4 + 0] = float3(ctrl_points[i].x(), ctrl_points[i].y(), i * 4 + 0.0f);
						p[i * 4 + 1] = float3(ctrl_points[i].x(), ctrl_points[i].y(), i * 4 + 1.0f);
						p[i * 4 + 2] = float3(ctrl_points[i].x(), ctrl_points[i].y(), i * 4 + 2.0f);
						p[i * 4 + 3] = float3(ctrl_points[i].x(), ctrl_points[i].y(), i * 4 + 3.0f);
					}
				}
				ibs_[CTRLPOINTS_INDEX]->Resize(6 * ctrl_points.size() * sizeof(uint16_t));
				{
					GraphicsBuffer::Mapper mapper(*ibs_[CTRLPOINTS_INDEX], BA_Write_Only);
					uint16_t* p = mapper.Pointer<uint16_t>();
					for (size_t i = 0; i < ctrl_points.size(); ++ i)
					{
						p[i * 6 + 0] = static_cast<uint16_t>(i * 4 + 0);
						p[i * 6 + 1] = static_cast<uint16_t>(i * 4 + 1);
						p[i * 6 + 2] = static_cast<uint16_t>(i * 4 + 2);
						p[i * 6 + 3] = static_cast<uint16_t>(i * 4 + 2);
						p[i * 6 + 4] = static_cast<uint16_t>(i * 4 + 3);
						p[i * 6 + 5] = static_cast<uint16_t>(i * 4 + 0);
					}
				}
			}
		}

		void SetLocSize(float4 const & loc_size)
		{
			loc_size_ = loc_size;
			*(effect_->ParameterByName("loc_size")) = loc_size;
		}

		void SetColor(Color const & clr)
		{
			*(effect_->ParameterByName("polyline_clr")) = float4(&clr.r());
		}

		void ActivePoint(int32_t point_index)
		{
			*(effect_->ParameterByName("active_pt")) = point_index;
		}

		void Render()
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

			this->OnRenderBegin();
			for (int i = 0; i < 4; ++ i)
			{
				re.Render(*techniques_[i], *rls_[i]);
			}
			this->OnRenderEnd();
		}

	private:
		static const int BACKGROUND_INDEX = 0;
		static const int COORDLINE_INDEX = 1;
		static const int POLYLINE_INDEX = 2;
		static const int CTRLPOINTS_INDEX = 3;

	private:
		GraphicsBufferPtr pos_vbs_[4];
		GraphicsBufferPtr ibs_[4];

		float4 loc_size_;

		std::wstring name_;
		Box box_;

		RenderEffectPtr effect_;
		RenderLayoutPtr rls_[4];
		RenderTechniquePtr techniques_[4];
	};

	class PolylineObject : public SceneObjectHelper
	{
	public:
		PolylineObject()
			: SceneObjectHelper(RenderablePtr(new RenderPolyline), 0),
				move_point_(false)
		{
			active_pt_ = -1;
		}

		void ActivePoint(int index)
		{
			BOOST_ASSERT(index < static_cast<int>(ctrl_points_.size()));
			active_pt_ = index;
			checked_pointer_cast<RenderPolyline>(renderable_)->ActivePoint(index);
		}
		int ActivePoint() const
		{
			return active_pt_;
		}

		void ClearCtrlPoints()
		{
			active_pt_ = -1;
			ctrl_points_.clear();
			move_point_ = false;
		}

		int AddCtrlPoint(float pos, float value)
		{
			pos = std::max(std::min(pos, 1.0f), 0.0f);
			value = std::max(std::min(value, 1.0f), 0.0f);

			int index;
			if (ctrl_points_.size() >= 1)
			{
				std::vector<float2>::iterator iter = ctrl_points_.begin();
				while ((iter != ctrl_points_.end() - 1) && ((iter + 1)->x() < pos))
				{
					++ iter;
				}
				index = iter + 1 - ctrl_points_.begin();
				if ((iter + 1 == ctrl_points_.end()) || (abs((iter + 1)->x() - pos) > 0.05f))
				{
					ctrl_points_.insert(iter + 1, float2(pos, value));
					checked_pointer_cast<RenderPolyline>(renderable_)->SetCtrlPoints(ctrl_points_);
				}
			}
			else
			{
				index = 0;
				ctrl_points_.push_back(float2(pos, value));
			}
			this->ActivePoint(index);

			return index;
		}
		int AddCtrlPoint(float pos)
		{
			pos = std::max(std::min(pos, 1.0f), 0.0f);
			float value = this->GetValue(pos);

			return this->AddCtrlPoint(pos, value);
		}

		void DelCtrlPoint(int index)
		{
			if (active_pt_ == index)
			{
				active_pt_ = -1;
				checked_pointer_cast<RenderPolyline>(renderable_)->ActivePoint(active_pt_);
			}

			ctrl_points_.erase(ctrl_points_.begin() + index);
			checked_pointer_cast<RenderPolyline>(renderable_)->SetCtrlPoints(ctrl_points_);
		}

		void SetCtrlPoint(int index, float pos, float value)
		{
			ctrl_points_[index] = float2(pos, value);
			checked_pointer_cast<RenderPolyline>(renderable_)->SetCtrlPoints(ctrl_points_);
		}

		void SetCtrlPoints(std::vector<float2> const & ctrl_points)
		{
			ctrl_points_ = ctrl_points;
			checked_pointer_cast<RenderPolyline>(renderable_)->SetCtrlPoints(ctrl_points_);
		}

		void SetLoc(int x, int y)
		{
			loc_size_.x() = static_cast<float>(x);
			loc_size_.y() = static_cast<float>(y);
			checked_pointer_cast<RenderPolyline>(renderable_)->SetLocSize(loc_size_);
		}
		int GetX() const
		{
			return static_cast<int>(loc_size_.x());
		}
		int GetY() const
		{
			return static_cast<int>(loc_size_.y());
		}

		void SetSize(int width, int height)
		{
			loc_size_.z() = static_cast<float>(width);
			loc_size_.w() = static_cast<float>(height);
			checked_pointer_cast<RenderPolyline>(renderable_)->SetLocSize(loc_size_);
		}
		int GetWidth() const
		{
			return static_cast<int>(loc_size_.z());
		}
		int GetHeight() const
		{
			return static_cast<int>(loc_size_.w());
		}

		void SetColor(Color const & clr)
		{
			clr_ = clr;
			checked_pointer_cast<RenderPolyline>(renderable_)->SetColor(clr);
		}

		size_t NumCtrlPoints() const
		{
			return ctrl_points_.size();
		}
		float2 const & GetCtrlPoint(size_t i) const
		{
			return *(ctrl_points_.begin() + i);
		}
		std::vector<float2> const & GetCtrlPoints() const
		{
			return ctrl_points_;
		}

		float2 PtFromCoord(int x, int y) const
		{
			return float2(static_cast<float>(x - loc_size_.x()) / loc_size_.z(),
				static_cast<float>(y - loc_size_.y()) / loc_size_.w());
		}

		int SelectCtrlPoint(int x, int y)
		{
			active_pt_ = -1;
			float2 const fxy = this->PtFromCoord(x, y);
			for (std::vector<float2>::const_iterator iter = ctrl_points_.begin(); iter != ctrl_points_.end(); ++ iter)
			{
				float2 const dxy = *iter - fxy;
				if (MathLib::dot(dxy, dxy) < 0.01f)
				{
					active_pt_ = iter - ctrl_points_.begin();
					break;
				}
			}
			return active_pt_;
		}

		int ActivePointIndex() const
		{
			return active_pt_;
		}

		float GetValue(float pos) const
		{
			for (std::vector<float2>::const_iterator iter = ctrl_points_.begin(); iter != ctrl_points_.end() - 1; ++ iter)
			{
				if ((iter + 1)->x() >= pos)
				{
					float const s = (pos - iter->x()) / ((iter + 1)->x() - iter->x());
					return iter->y() + ((iter + 1)->y() - iter->y()) * s;
				}
			}
			return -1;
		}

		void LineSelected(bool sel)
		{
			if (sel)
			{
				checked_pointer_cast<RenderPolyline>(renderable_)->SetColor(Color(1, 0, 0, 1));
			}
			else
			{
				checked_pointer_cast<RenderPolyline>(renderable_)->SetColor(clr_);
			}
		}

		void MouseDownHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
		{
			if ((pt.x() >= this->GetX()) && (pt.y() >= this->GetY())
				&& (pt.x() < this->GetX() + this->GetWidth())
				&& (pt.y() < this->GetY() + this->GetHeight()))
			{
				if (MB_Left == buttons)
				{
					float2 p = this->PtFromCoord(pt.x(), pt.y());
					if (abs(this->GetValue(p.x()) - p.y()) < 0.1f)
					{
						bool found = false;
						for (size_t i = 0; i < this->NumCtrlPoints(); ++ i)
						{
							float2 cp = this->GetCtrlPoint(i);
							cp = p - cp;
							if (MathLib::dot(cp, cp) < 0.01f)
							{
								this->ActivePoint(i);
								move_point_ = true;
								found = true;
								break;
							}
						}

						if (!found)
						{
							this->AddCtrlPoint(p.x());
							move_point_ = true;
						}
					}
				}
			}
		}

		void MouseUpHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
		{
			if (move_point_ || ((pt.x() >= this->GetX()) && (pt.y() >= this->GetY())
				&& (pt.x() < this->GetX() + this->GetWidth())
				&& (pt.y() < this->GetY() + this->GetHeight())))
			{
				if (MB_Left == buttons)
				{
					if (move_point_)
					{
						if ((this->ActivePoint() != 0) && (this->ActivePoint() != static_cast<int>(this->NumCtrlPoints() - 1)))
						{
							float2 p = this->PtFromCoord(pt.x(), pt.y());
							if ((p.x() < 0) || (p.x() > 1) || (p.y() < 0) || (p.y() > 1))
							{
								this->DelCtrlPoint(this->ActivePoint());
							}
						}

						move_point_ = false;
					}
				}
			}
		}

		void MouseOverHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
		{
			if (move_point_ || ((pt.x() >= this->GetX()) && (pt.y() >= this->GetY())
				&& (pt.x() < this->GetX() + this->GetWidth())
				&& (pt.y() < this->GetY() + this->GetHeight())))
			{
				if (0 == buttons)
				{
					float2 p = this->PtFromCoord(pt.x(), pt.y());
					if (abs(this->GetValue(p.x()) - p.y()) < 0.1f)
					{
						this->LineSelected(true);
					}
					else
					{
						this->LineSelected(false);
					}
				}
				else
				{
					if (MB_Left == buttons)
					{
						if (move_point_)
						{
							float2 p = this->PtFromCoord(pt.x(), pt.y());
							if (0 == this->ActivePoint())
							{
								p.x() = 0;
								p.y() = std::min(std::max(p.y(), 0.0f), 1.0f);
							}
							else
							{
								if (static_cast<int>(this->NumCtrlPoints() - 1) == this->ActivePoint())
								{
									p.x() = 1;
									p.y() = std::min(std::max(p.y(), 0.0f), 1.0f);
								}
							}
							if (this->ActivePoint() < static_cast<int>(this->NumCtrlPoints() - 1))
							{
								float2 next_p = this->GetCtrlPoint(this->ActivePoint() + 1);
								if (next_p.x() <= p.x())
								{
									p.x() = next_p.x();
								}
							}
							this->SetCtrlPoint(this->ActivePoint(), p.x(), p.y());
						}
					}
				}
			}
		}

	private:
		std::vector<float2> ctrl_points_;
		int active_pt_;
		float4 loc_size_;
		Color clr_;

		bool move_point_;
	};

	boost::shared_ptr<PolylineObject> size_over_life_obj;
	boost::shared_ptr<PolylineObject> weight_over_life_obj;
	boost::shared_ptr<PolylineObject> transparency_over_life_obj;


	template <typename ParticleType>
	class GenParticle
	{
	public:
		GenParticle()
			: random_gen_(boost::lagged_fibonacci607(), boost::uniform_real<float>(-1, 1)),
				angle_(PI / 3), life_(3)
		{
		}

		void SetAngle(float angle)
		{
			angle_ = angle;
		}
		float GetAngle() const
		{
			return angle_;
		}

		void SetLife(float life)
		{
			life_ = life;
		}
		float GetLife() const
		{
			return life_;
		}

		void InitVelocity(float vel)
		{
			velocity_ = vel;
		}

		void operator()(ParticleType& par, float4x4 const & mat)
		{
			par.pos = MathLib::transform_coord(float3(0, 0, 0), mat);
			float theta = random_gen_() * PI;
			float phi = abs(random_gen_()) * angle_ / 2;
			float velocity = (random_gen_() + 1) * velocity_;
			float x = cos(theta) * sin(phi);
			float z = sin(theta) * sin(phi);
			float y = cos(phi);
			par.vel = MathLib::transform_normal(float3(x, y, z) * velocity, mat);
			par.life = life_;
		}

	private:
		boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_real<float> > random_gen_;
		float angle_;
		float life_;
		float velocity_;
	};

	GenParticle<Particle> gen_particle;


	template <typename ParticleType>
	class UpdateParticle
	{
	public:
		UpdateParticle()
			: gravity_(0.1f),
				force_(0, 0, 0),
				media_density_(0.0f)
		{
		}

		void SetInitLife(float life)
		{
			init_life_ = life;
		}

		void SetForce(float3 force)
		{
			force_ = force;
		}

		void MediaDensity(float density)
		{
			media_density_ = density;
		}

		void operator()(ParticleType& par, float elapse_time)
		{
			buoyancy_ = media_density_ * (size_over_life_obj->GetValue((init_life_ - par.life) / init_life_) * 2);
			par.vel += (force_ + float3(0, buoyancy_ - gravity_, 0)) * elapse_time;
			par.pos += par.vel * elapse_time;
			par.life -= elapse_time;

			if (par.pos.y() <= 0)
			{
				par.pos.y() += 0.01f;
				par.vel = MathLib::reflect(par.vel, float3(0, 1, 0)) * 0.8f;
			}
		}

	private:
		float init_life_;
		float gravity_;
		float buoyancy_;
		float3 force_;
		float media_density_;
	};

	UpdateParticle<Particle> update_particle;


	class CopyPostProcess : public PostProcess
	{
	public:
		CopyPostProcess()
			: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("ParticleEditor.kfx")->TechniqueByName("Copy"))
		{
		}
	};

	enum
	{
		Exit
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape)
	};

	bool ConfirmDevice()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();
		RenderDeviceCaps const & caps = re.DeviceCaps();
		if (caps.max_shader_model < 2)
		{
			return false;
		}

		try
		{
			TexturePtr temp_tex = rf.MakeTexture2D(800, 600, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
			rf.Make2DRenderView(*temp_tex, 0);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}


int main()
{
	ResLoader::Instance().AddPath("../Samples/media/Common");
	ResLoader::Instance().AddPath("../Samples/media/ParticleEditor");

	RenderSettings settings = Context::Instance().LoadCfg("KlayGE.cfg");
	settings.ConfirmDevice = ConfirmDevice;

	Context::Instance().SceneManagerInstance(SceneManager::NullObject());

	ParticleEditorApp app("Particle Editor", settings);
	app.Create();
	app.Run();

	return 0;
}

ParticleEditorApp::ParticleEditorApp(std::string const & name, RenderSettings const & settings)
					: App3DFramework(name, settings)
{
}

void ParticleEditorApp::InitObjects()
{
	use_gs = (Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps().max_shader_model >= 4);

	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	this->LookAt(float3(-1.2f, 0.5f, -1.2f), float3(0, 0.5f, 0));
	this->Proj(0.01f, 100);

	fpsController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(new input_signal);
	input_handler->connect(boost::bind(&ParticleEditorApp::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	particles_.reset(new ParticlesObject);
	particles_->AddToSceneManager();

	terrain_.reset(new TerrainObject);
	terrain_->AddToSceneManager();

	ps_.reset(new ParticleSystem<Particle>(NUM_PARTICLE, boost::bind(&GenParticle<Particle>::operator(), &gen_particle, _1, _2),
		boost::bind(&UpdateParticle<Particle>::operator(), &update_particle, _1, _2)));

	ps_->AutoEmit(256);

	size_over_life_obj.reset(new PolylineObject);
	size_over_life_obj->AddToSceneManager();
	checked_pointer_cast<ParticlesObject>(particles_)->SetSizeOverLife(size_over_life_obj->GetCtrlPoints());

	weight_over_life_obj.reset(new PolylineObject);
	weight_over_life_obj->AddToSceneManager();
	checked_pointer_cast<ParticlesObject>(particles_)->SetWeightOverLife(weight_over_life_obj->GetCtrlPoints());

	transparency_over_life_obj.reset(new PolylineObject);
	transparency_over_life_obj->AddToSceneManager();
	checked_pointer_cast<ParticlesObject>(particles_)->SetTransparencyOverLife(transparency_over_life_obj->GetCtrlPoints());

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	scene_buffer_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	scene_buffer_->GetViewport().camera = screen_buffer->GetViewport().camera;

	copy_pp_.reset(new CopyPostProcess);

	UIManager::Instance().Load(ResLoader::Instance().Load("ParticleEditor.kui"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	id_save_as_ = dialog_->IDFromName("SaveAs");
	id_angle_static_ = dialog_->IDFromName("AngleStatic");
	id_angle_slider_ = dialog_->IDFromName("AngleSlider");
	id_life_static_ = dialog_->IDFromName("LifeStatic");
	id_life_slider_ = dialog_->IDFromName("LifeSlider");
	id_density_static_ = dialog_->IDFromName("DensityStatic");
	id_density_slider_ = dialog_->IDFromName("DensitySlider");
	id_velocity_static_ = dialog_->IDFromName("VelocityStatic");
	id_velocity_slider_ = dialog_->IDFromName("VelocitySlider");
	id_fps_camera_ = dialog_->IDFromName("FPSCamera");
	id_particle_tex_button_ = dialog_->IDFromName("ParticleTexButton");

	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::OpenHandler, this, _1));
	dialog_->Control<UIButton>(id_save_as_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::SaveAsHandler, this, _1));

	dialog_->Control<UISlider>(id_angle_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::AngleChangedHandler, this, _1));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));
	dialog_->Control<UISlider>(id_life_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::LifeChangedHandler, this, _1));
	this->LifeChangedHandler(*dialog_->Control<UISlider>(id_life_slider_));
	dialog_->Control<UISlider>(id_density_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::DensityChangedHandler, this, _1));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));
	dialog_->Control<UISlider>(id_velocity_slider_)->OnValueChangedEvent().connect(boost::bind(&ParticleEditorApp::VelocityChangedHandler, this, _1));
	this->VelocityChangedHandler(*dialog_->Control<UISlider>(id_velocity_slider_));

	dialog_->Control<UICheckBox>(id_fps_camera_)->OnChangedEvent().connect(boost::bind(&ParticleEditorApp::FPSCameraHandler, this, _1));

	dialog_->Control<UITexButton>(id_particle_tex_button_)->OnClickedEvent().connect(boost::bind(&ParticleEditorApp::ChangeParticleTexHandler, this, _1));

	WindowPtr main_wnd = Context::Instance().AppInstance().MainWnd();
	main_wnd->OnMouseDown().connect(boost::bind(&ParticleEditorApp::MouseDownHandler, this, _2, _3));
	main_wnd->OnMouseUp().connect(boost::bind(&ParticleEditorApp::MouseUpHandler, this, _2, _3));
	main_wnd->OnMouseOver().connect(boost::bind(&ParticleEditorApp::MouseOverHandler, this, _2, _3));

	this->LoadParticleSystem(ResLoader::Instance().Locate("Fire.psml"));
}

void ParticleEditorApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	RenderViewPtr ds_view = rf.MakeDepthStencilRenderView(width, height, EF_D16, 1, 0);

	scene_tex_ = rf.MakeTexture2D(width, height, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
	scene_buffer_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*scene_tex_, 0));
	scene_buffer_->Attach(FrameBuffer::ATT_DepthStencil, ds_view);

	checked_pointer_cast<RenderParticles>(particles_->GetRenderable())->SceneTexture(scene_tex_, scene_buffer_->RequiresFlipping());

	copy_pp_->Source(scene_tex_, scene_buffer_->RequiresFlipping());
	copy_pp_->Destinate(FrameBufferPtr());

	size_over_life_obj->SetLoc(width - 240, 280);
	size_over_life_obj->SetSize(200, 120);
	size_over_life_obj->SetColor(Color(0, 1, 0, 1));
	//size_over_life_obj->AddCtrlPoint(0.0f, 0.5f);
	//size_over_life_obj->AddCtrlPoint(1.0f, 0.5f);

	weight_over_life_obj->SetLoc(width - 240, 440);
	weight_over_life_obj->SetSize(200, 120);
	weight_over_life_obj->SetColor(Color(0, 1, 0, 1));
	//weight_over_life_obj->AddCtrlPoint(0.0f, 0.5f);
	//weight_over_life_obj->AddCtrlPoint(1.0f, 0.5f);

	transparency_over_life_obj->SetLoc(width - 240, 600);
	transparency_over_life_obj->SetSize(200, 120);
	transparency_over_life_obj->SetColor(Color(0, 1, 0, 1));
	//transparency_over_life_obj->AddCtrlPoint(0.0f, 0.0f);
	//transparency_over_life_obj->AddCtrlPoint(1.0f, 1.0f);

	UIManager::Instance().SettleCtrls(width, height);
}

void ParticleEditorApp::MouseDownHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
{
	size_over_life_obj->MouseDownHandler(buttons, pt);
	weight_over_life_obj->MouseDownHandler(buttons, pt);
	transparency_over_life_obj->MouseDownHandler(buttons, pt);
}

void ParticleEditorApp::MouseUpHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
{
	size_over_life_obj->MouseUpHandler(buttons, pt);
	weight_over_life_obj->MouseUpHandler(buttons, pt);
	transparency_over_life_obj->MouseUpHandler(buttons, pt);
}

void ParticleEditorApp::MouseOverHandler(uint32_t buttons, Vector_T<int32_t, 2> const & pt)
{
	size_over_life_obj->MouseOverHandler(buttons, pt);
	weight_over_life_obj->MouseOverHandler(buttons, pt);
	transparency_over_life_obj->MouseOverHandler(buttons, pt);
}

void ParticleEditorApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ParticleEditorApp::OpenHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		this->LoadParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::SaveAsHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "PSML File\0*.psml\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = ".psml";

	if (GetSaveFileNameA(&ofn))
	{
		this->SaveParticleSystem(fn);
	}
#endif
}

void ParticleEditorApp::AngleChangedHandler(KlayGE::UISlider const & sender)
{
	float angle = static_cast<float>(sender.GetValue());
	gen_particle.SetAngle(angle * DEG2RAD);

	std::wostringstream stream;
	stream << angle;
	dialog_->Control<UIStatic>(id_angle_static_)->SetText(stream.str());
}

void ParticleEditorApp::LifeChangedHandler(KlayGE::UISlider const & sender)
{
	init_life_ = static_cast<float>(sender.GetValue());
	gen_particle.SetLife(init_life_);
	update_particle.SetInitLife(init_life_);
	checked_pointer_cast<ParticlesObject>(particles_)->SetInitLife(init_life_);

	std::wostringstream stream;
	stream << init_life_;
	dialog_->Control<UIStatic>(id_life_static_)->SetText(stream.str());
}

void ParticleEditorApp::DensityChangedHandler(KlayGE::UISlider const & sender)
{
	float density = sender.GetValue() / 100.0f;
	update_particle.MediaDensity(density);

	std::wostringstream stream;
	stream << density;
	dialog_->Control<UIStatic>(id_density_static_)->SetText(stream.str());
}

void ParticleEditorApp::VelocityChangedHandler(KlayGE::UISlider const & sender)
{
	float velocity = sender.GetValue() / 100.0f;
	gen_particle.InitVelocity(velocity);

	std::wostringstream stream;
	stream << velocity;
	dialog_->Control<UIStatic>(id_velocity_static_)->SetText(stream.str());
}

void ParticleEditorApp::FPSCameraHandler(KlayGE::UICheckBox const & sender)
{
	if (sender.GetChecked())
	{
		fpsController_.AttachCamera(this->ActiveCamera());
	}
	else
	{
		fpsController_.DetachCamera();
	}
}

void ParticleEditorApp::ChangeParticleTexHandler(KlayGE::UITexButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "DDS File\0*.dds\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		particle_tex_ = fn;
		this->LoadParticleTex(fn);
	}
#endif
}

void ParticleEditorApp::LoadParticleTex(std::string const & name)
{
	TexturePtr tex = LoadTexture(name, EAH_GPU_Read)();
	dialog_->Control<UITexButton>(id_particle_tex_button_)->SetTexture(tex);
	checked_pointer_cast<ParticlesObject>(particles_)->ParticleTexture(tex);
}

void ParticleEditorApp::LoadParticleSystem(std::string const & name)
{
	size_over_life_obj->ClearCtrlPoints();
	weight_over_life_obj->ClearCtrlPoints();
	transparency_over_life_obj->ClearCtrlPoints();

	using boost::lexical_cast;

	std::ifstream ifs(name.c_str());
	BOOST_ASSERT(ifs);

	ifs.seekg(0, std::ios_base::end);
	int len = ifs.tellg();
	ifs.seekg(0, std::ios_base::beg);
	std::vector<char> str(len + 1, 0);
	ifs.read(&str[0], len);

	using namespace rapidxml;
	xml_document<> doc;
	doc.parse<0>(&str[0]);
	
	xml_node<>* root = doc.first_node("particle_system");

	xml_attribute<>* attr = root->first_attribute("particle_tex");
	particle_tex_ = attr->value();
	this->LoadParticleTex(ResLoader::Instance().Locate(attr->value()));

	attr = root->first_attribute("emit_angle");
	dialog_->Control<UISlider>(id_angle_slider_)->SetValue(lexical_cast<int>(attr->value()));
	this->AngleChangedHandler(*dialog_->Control<UISlider>(id_angle_slider_));

	attr = root->first_attribute("life");
	dialog_->Control<UISlider>(id_life_slider_)->SetValue(lexical_cast<int>(attr->value()));
	this->LifeChangedHandler(*dialog_->Control<UISlider>(id_life_slider_));

	attr = root->first_attribute("media_density");
	dialog_->Control<UISlider>(id_density_slider_)->SetValue(static_cast<int>(lexical_cast<float>(attr->value()) * 100.0f));
	this->DensityChangedHandler(*dialog_->Control<UISlider>(id_density_slider_));

	attr = root->first_attribute("velocity");
	dialog_->Control<UISlider>(id_velocity_slider_)->SetValue(static_cast<int>(lexical_cast<float>(attr->value()) * 100.0f));
	this->VelocityChangedHandler(*dialog_->Control<UISlider>(id_velocity_slider_));

	for (xml_node<>* node = root->first_node("curve"); node; node = node->next_sibling())
	{
		std::vector<float2> xys;
		for (xml_node<>* ctrl_point_node = node->first_node("ctrl_point"); ctrl_point_node; ctrl_point_node = ctrl_point_node->next_sibling())
		{
			xml_attribute<>* attr_x = ctrl_point_node->first_attribute("x");
			xml_attribute<>* attr_y = ctrl_point_node->first_attribute("y");

			xys.push_back(float2(lexical_cast<float>(attr_x->value()), lexical_cast<float>(attr_y->value())));
		}

		attr = node->first_attribute("name");
		if (std::string("size_over_life") == attr->value())
		{
			size_over_life_obj->SetCtrlPoints(xys);
		}
		if (std::string("weight_over_life") == attr->value())
		{
			weight_over_life_obj->SetCtrlPoints(xys);
		}
		if (std::string("transparency_over_life") == attr->value())
		{
			transparency_over_life_obj->SetCtrlPoints(xys);
		}
	}
}

void ParticleEditorApp::SaveParticleSystem(std::string const & name)
{
	using boost::lexical_cast;

	using namespace rapidxml;
	xml_document<> doc;

	xml_node<>* root = doc.allocate_node(node_element, "particle_system");
	doc.append_node(root);

	xml_attribute<>* attr = doc.allocate_attribute("particle_tex", particle_tex_.c_str());
	root->append_attribute(attr);

	float angle = static_cast<float>(dialog_->Control<UISlider>(id_angle_slider_)->GetValue());
	std::string emit_angle_str = lexical_cast<std::string>(angle);
	attr = doc.allocate_attribute("emit_angle", emit_angle_str.c_str());
	root->append_attribute(attr);

	float life = static_cast<float>(dialog_->Control<UISlider>(id_life_slider_)->GetValue());
	std::string life_str = lexical_cast<std::string>(life);
	attr = doc.allocate_attribute("life", life_str.c_str());
	root->append_attribute(attr);

	float density = dialog_->Control<UISlider>(id_density_slider_)->GetValue() / 100.0f;
	std::string density_str = lexical_cast<std::string>(density);
	attr = doc.allocate_attribute("media_density", density_str.c_str());
	root->append_attribute(attr);

	float velocity = dialog_->Control<UISlider>(id_velocity_slider_)->GetValue() / 100.0f;
	std::string velocity_str = lexical_cast<std::string>(velocity);
	attr = doc.allocate_attribute("velocity", velocity_str.c_str());
	root->append_attribute(attr);

	xml_node<>* size_over_life_node = doc.allocate_node(node_element, "curve");
	attr = doc.allocate_attribute("name", "size_over_life");
	size_over_life_node->append_attribute(attr);
	std::vector<std::string> size_over_life_xs(size_over_life_obj->NumCtrlPoints());
	std::vector<std::string> size_over_life_ys(size_over_life_obj->NumCtrlPoints());
	for (size_t i = 0; i < size_over_life_obj->NumCtrlPoints(); ++ i)
	{
		float2 const & pt = size_over_life_obj->GetCtrlPoint(i);
		size_over_life_xs[i] = lexical_cast<std::string>(pt.x());
		size_over_life_ys[i] = lexical_cast<std::string>(pt.y());
		
		xml_node<>* ctrl_point_node = doc.allocate_node(node_element, "ctrl_point");
		attr = doc.allocate_attribute("x", size_over_life_xs[i].c_str());
		ctrl_point_node->append_attribute(attr);
		attr = doc.allocate_attribute("y", size_over_life_ys[i].c_str());
		ctrl_point_node->append_attribute(attr);

		size_over_life_node->append_node(ctrl_point_node);
	}
	root->append_node(size_over_life_node);

	xml_node<>* weight_over_life_node = doc.allocate_node(node_element, "curve");
	attr = doc.allocate_attribute("name", "weight_over_life");
	weight_over_life_node->append_attribute(attr);
	std::vector<std::string> weight_over_life_xs(weight_over_life_obj->NumCtrlPoints());
	std::vector<std::string> weight_over_life_ys(weight_over_life_obj->NumCtrlPoints());
	for (size_t i = 0; i < weight_over_life_obj->NumCtrlPoints(); ++ i)
	{
		float2 const & pt = weight_over_life_obj->GetCtrlPoint(i);
		weight_over_life_xs[i] = lexical_cast<std::string>(pt.x());
		weight_over_life_ys[i] = lexical_cast<std::string>(pt.y());
		
		xml_node<>* ctrl_point_node = doc.allocate_node(node_element, "ctrl_point");
		attr = doc.allocate_attribute("x", weight_over_life_xs[i].c_str());
		ctrl_point_node->append_attribute(attr);
		attr = doc.allocate_attribute("y", weight_over_life_ys[i].c_str());
		ctrl_point_node->append_attribute(attr);

		weight_over_life_node->append_node(ctrl_point_node);
	}
	root->append_node(weight_over_life_node);

	xml_node<>* transparency_over_life_node = doc.allocate_node(node_element, "curve");
	attr = doc.allocate_attribute("name", "transparency_over_life");
	transparency_over_life_node->append_attribute(attr);
	std::vector<std::string> transparency_over_life_xs(transparency_over_life_obj->NumCtrlPoints());
	std::vector<std::string> transparency_over_life_ys(transparency_over_life_obj->NumCtrlPoints());
	for (size_t i = 0; i < transparency_over_life_obj->NumCtrlPoints(); ++ i)
	{
		float2 const & pt = transparency_over_life_obj->GetCtrlPoint(i);
		transparency_over_life_xs[i] = lexical_cast<std::string>(pt.x());
		transparency_over_life_ys[i] = lexical_cast<std::string>(pt.y());
		
		xml_node<>* ctrl_point_node = doc.allocate_node(node_element, "ctrl_point");
		attr = doc.allocate_attribute("x", transparency_over_life_xs[i].c_str());
		ctrl_point_node->append_attribute(attr);
		attr = doc.allocate_attribute("y", transparency_over_life_ys[i].c_str());
		ctrl_point_node->append_attribute(attr);

		transparency_over_life_node->append_node(ctrl_point_node);
	}
	root->append_node(transparency_over_life_node);

	std::ofstream ofs(name.c_str());
	ofs << doc;
}

class particle_cmp
{
public:
	bool operator()(std::pair<int, float> const & lhs, std::pair<int, float> const & rhs) const
	{
		return lhs.second > rhs.second;
	}
};

uint32_t ParticleEditorApp::DoUpdate(uint32_t pass)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	switch (pass)
	{
	case 0:
		re.BindFrameBuffer(scene_buffer_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

		checked_pointer_cast<ParticlesObject>(particles_)->SetSizeOverLife(size_over_life_obj->GetCtrlPoints());
		checked_pointer_cast<ParticlesObject>(particles_)->SetWeightOverLife(weight_over_life_obj->GetCtrlPoints());
		checked_pointer_cast<ParticlesObject>(particles_)->SetTransparencyOverLife(transparency_over_life_obj->GetCtrlPoints());

		terrain_->Visible(true);
		particles_->Visible(false);
		return App3DFramework::URV_Need_Flush;

	default:
		re.BindFrameBuffer(FrameBufferPtr());
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 1, 0);

		copy_pp_->Apply();

		float4x4 mat = MathLib::translation(0.0f, 0.1f, 0.0f);
		ps_->ModelMatrix(mat);

		ps_->Update(static_cast<float>(timer_.elapsed()));
		timer_.restart();

		float4x4 view_mat = Context::Instance().AppInstance().ActiveCamera().ViewMatrix();
		std::vector<std::pair<int, float> > active_particles;
		for (uint32_t i = 0; i < ps_->NumParticles(); ++ i)
		{
			if (ps_->GetParticle(i).life > 0)
			{
				float3 pos = ps_->GetParticle(i).pos;
				float p_to_v = (pos.x() * view_mat(0, 2) + pos.y() * view_mat(1, 2) + pos.z() * view_mat(2, 2) + view_mat(3, 2))
					/ (pos.x() * view_mat(0, 3) + pos.y() * view_mat(1, 3) + pos.z() * view_mat(2, 3) + view_mat(3, 3));

				active_particles.push_back(std::make_pair(i, p_to_v));
			}
		}
		if (!active_particles.empty())
		{
			std::sort(active_particles.begin(), active_particles.end(), particle_cmp());

			uint32_t const num_pars = static_cast<uint32_t>(active_particles.size());
			RenderLayoutPtr const & rl = particles_->GetRenderable()->GetRenderLayout();
			GraphicsBufferPtr instance_gb;
			if (use_gs)
			{
				instance_gb = rl->GetVertexStream(0);
			}
			else
			{
				instance_gb = rl->InstanceStream();

				for (uint32_t i = 0; i < rl->NumVertexStreams(); ++ i)
				{
					rl->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, num_pars);
				}
			}

			instance_gb->Resize(sizeof(float4) * num_pars);
			{
				GraphicsBuffer::Mapper mapper(*instance_gb, BA_Write_Only);
				float4* instance_data = mapper.Pointer<float4>();
				for (uint32_t i = 0; i < num_pars; ++ i, ++ instance_data)
				{
					instance_data->x() = ps_->GetParticle(active_particles[i].first).pos.x();
					instance_data->y() = ps_->GetParticle(active_particles[i].first).pos.y();
					instance_data->z() = ps_->GetParticle(active_particles[i].first).pos.z();
					instance_data->w() = ps_->GetParticle(active_particles[i].first).life;
				}
			}

			particles_->Visible(true);
		}
		terrain_->Visible(false);

		UIManager::Instance().Render();

		std::wostringstream stream;
		stream << this->FPS();

		font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Particle System", 16);
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

		return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
	}
}
