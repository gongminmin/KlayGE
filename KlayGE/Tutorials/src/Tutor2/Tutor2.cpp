#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"

class TutorFramework : public KlayGE::App3DFramework
{
public:
	TutorFramework();

protected:
	virtual void OnCreate();

private:
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

private:
	KlayGE::TrackballCameraController tbController_;

	KlayGE::FontPtr font_;

	KlayGE::SceneNodePtr renderableBox_;
	KlayGE::SceneNodePtr renderableFile_;
	KlayGE::SceneNodePtr renderableMesh_;
};

class RenderPolygon : public KlayGE::StaticMesh
{
public:
	explicit RenderPolygon(std::wstring_view name);

	void DoBuildMeshInfo(KlayGE::RenderModel const & model) override;
};

int SampleMain()
{
	TutorFramework app;
	app.Create();
	app.Run();

	return 0;
}

TutorFramework::TutorFramework()
	: App3DFramework("Tutor2")
{
}

void TutorFramework::OnCreate()
{
	font_ = KlayGE::SyncLoadFont("gkai00mp.kfont");

	auto& root_node = KlayGE::Context::Instance().SceneManagerInstance().SceneRootNode();

	KlayGE::OBBox boxRange(
		KlayGE::MathLib::convert_to_obbox(KlayGE::AABBox(KlayGE::float3(-1.0f, -0.25f, -0.25f), KlayGE::float3(-0.5f, 0.25f, 0.25f))));
	KlayGE::Color boxColor(1.0f, 0.0f, 0.0f, 1.0f);

	renderableBox_ = KlayGE::MakeSharedPtr<KlayGE::SceneNode>(
		KlayGE::MakeSharedPtr<KlayGE::RenderableComponent>(KlayGE::MakeSharedPtr<KlayGE::RenderableTriBox>(boxRange, boxColor)),
		KlayGE::SceneNode::SOA_Cullable);
	root_node.AddChild(renderableBox_);

	KlayGE::RenderModelPtr loadedModel = KlayGE::SyncLoadModel("teapot.glb", KlayGE::EAH_GPU_Read,
		KlayGE::SceneNode::SOA_Cullable, KlayGE::AddToSceneRootHelper,
		KlayGE::CreateModelFactory<KlayGE::RenderModel>, KlayGE::CreateMeshFactory<RenderPolygon>);

	renderableFile_ = loadedModel->RootNode();
	renderableFile_->TransformToParent(KlayGE::MathLib::translation(0.0f, 0.5f, 0.0f));

	std::vector<KlayGE::float3> vertices;
	vertices.push_back(KlayGE::float3(0.5f, -0.25f,  0.25f));
	vertices.push_back(KlayGE::float3(1.0f, -0.25f,  0.25f));
	vertices.push_back(KlayGE::float3(1.0f, -0.25f, -0.25f));
	vertices.push_back(KlayGE::float3(0.5f, -0.25f, -0.25f));
	vertices.push_back(KlayGE::float3(0.5f,  0.25f,  0.25f));
	vertices.push_back(KlayGE::float3(1.0f,  0.25f,  0.25f));
	vertices.push_back(KlayGE::float3(1.0f,  0.25f, -0.25f));
	vertices.push_back(KlayGE::float3(0.5f,  0.25f, -0.25f));

	KlayGE::RenderModelPtr model = KlayGE::MakeSharedPtr<KlayGE::RenderModel>(L"model", KlayGE::SceneNode::SOA_Cullable);

	std::vector<KlayGE::StaticMeshPtr> meshes(2);

	std::vector<KlayGE::uint16_t> indices1;
	indices1.push_back(0); indices1.push_back(4); indices1.push_back(1); indices1.push_back(5);
	indices1.push_back(2); indices1.push_back(6); indices1.push_back(3); indices1.push_back(7);
	indices1.push_back(0); indices1.push_back(4);

	meshes[0] = KlayGE::MakeSharedPtr<RenderPolygon>(L"side_mesh");
	meshes[0]->NumLods(1);
	meshes[0]->AddVertexStream(0, &vertices[0], static_cast<KlayGE::uint32_t>(sizeof(vertices[0]) * vertices.size()),
		KlayGE::VertexElement(KlayGE::VEU_Position, 0, KlayGE::EF_BGR32F), KlayGE::EAH_GPU_Read);
	meshes[0]->AddIndexStream(0, &indices1[0], static_cast<KlayGE::uint32_t>(sizeof(indices1[0]) * indices1.size()),
		KlayGE::EF_R16UI, KlayGE::EAH_GPU_Read);
	meshes[0]->GetRenderLayout().TopologyType(KlayGE::RenderLayout::TT_TriangleStrip);
	meshes[0]->PosBound(KlayGE::AABBox(KlayGE::float3(-1, -1, -1), KlayGE::float3(1, 1, 1)));

	std::vector<KlayGE::uint16_t> indices2;
	indices2.push_back(0); indices2.push_back(1); indices2.push_back(2);
	indices2.push_back(0); indices2.push_back(2); indices2.push_back(3);
	indices2.push_back(7); indices2.push_back(6); indices2.push_back(5);
	indices2.push_back(7); indices2.push_back(5); indices2.push_back(4);

	meshes[1] = KlayGE::MakeSharedPtr<RenderPolygon>(L"cap_mesh");
	meshes[1]->NumLods(1);
	meshes[1]->AddVertexStream(0, &vertices[0], static_cast<KlayGE::uint32_t>(sizeof(vertices[0]) * vertices.size()),
		KlayGE::VertexElement(KlayGE::VEU_Position, 0, KlayGE::EF_BGR32F), KlayGE::EAH_GPU_Read);
	meshes[1]->AddIndexStream(0, &indices2[0], static_cast<KlayGE::uint32_t>(sizeof(indices2[0]) * indices2.size()),
		KlayGE::EF_R16UI, KlayGE::EAH_GPU_Read);
	meshes[1]->GetRenderLayout().TopologyType(KlayGE::RenderLayout::TT_TriangleList);
	meshes[1]->PosBound(KlayGE::AABBox(KlayGE::float3(-1, -1, -1), KlayGE::float3(1, 1, 1)));

	for (size_t i = 0; i < meshes.size(); ++ i)
	{
		meshes[i]->BuildMeshInfo(*model);
	}
	model->AssignMeshes(meshes.begin(), meshes.end());
	model->BuildModelInfo();

	renderableMesh_ = model->RootNode();
	for (size_t i = 0; i < meshes.size(); ++ i)
	{
		renderableMesh_->AddComponent(KlayGE::MakeSharedPtr<KlayGE::RenderableComponent>(meshes[i]));
	}
	root_node.AddChild(renderableMesh_);

	this->LookAt(KlayGE::float3(0, 0, -4.0f), KlayGE::float3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	tbController_.AttachCamera(this->ActiveCamera());
	tbController_.Scalers(0.01f, 0.05f);
}

void TutorFramework::DoUpdateOverlay()
{
	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, KlayGE::Color(1, 1, 0, 1), L"Tutorial 2", 16);
	font_->RenderText(0, 18, KlayGE::Color(1, 1, 0, 1), stream.str(), 16);
}

KlayGE::uint32_t TutorFramework::DoUpdate(KlayGE::uint32_t /*pass*/)
{
	KlayGE::RenderEngine& re = KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	KlayGE::Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (KlayGE::Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	re.CurFrameBuffer()->Clear(KlayGE::FrameBuffer::CBM_Color | KlayGE::FrameBuffer::CBM_Depth,
		clear_clr, 1.0f, 0);

	return KlayGE::App3DFramework::URV_NeedFlush | KlayGE::App3DFramework::URV_Finished;
}


RenderPolygon::RenderPolygon(std::wstring_view name)
	: KlayGE::StaticMesh(name)
{
	KlayGE::RenderEffectPtr effect = KlayGE::SyncLoadRenderEffect("RenderableHelper.fxml");

	this->Technique(effect, effect->TechniqueByName("HelperTec"));

	*(effect->ParameterByName("color")) = KlayGE::float4(1.0f, 0.0f, 0.0f, 1.0f);
}

void RenderPolygon::DoBuildMeshInfo(KlayGE::RenderModel const & model)
{
	KFL_UNUSED(model);

	KlayGE::AABBox const & pos_bb = this->PosBound();
	*(effect_->ParameterByName("pos_center")) = pos_bb.Center();
	*(effect_->ParameterByName("pos_extent")) = pos_bb.HalfSize();
}
