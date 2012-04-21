#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/CameraController.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Camera.hpp>

#include <vector>
#include <sstream>

class TutorFramework : public KlayGE::App3DFramework
{
public:
	TutorFramework();

protected:
	virtual void InitObjects();

private:
	virtual void DoUpdateOverlay();
	virtual KlayGE::uint32_t DoUpdate(KlayGE::uint32_t pass);

private:
	KlayGE::TrackballCameraController tbController_;

	KlayGE::FontPtr font_;

	KlayGE::SceneObjectHelperPtr renderableBox_;
	KlayGE::SceneObjectHelperPtr renderableFile_;
	KlayGE::SceneObjectHelperPtr renderableMesh_;
};

class RenderPolygon : public KlayGE::StaticMesh
{
public:
	RenderPolygon(KlayGE::RenderModelPtr const & model, std::wstring const& name);

	virtual void OnRenderBegin();
};

int main()
{
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");

	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");

	TutorFramework app;
	app.Create();
	app.Run();

	return 0;
}

TutorFramework::TutorFramework()
	: App3DFramework("Tutor2")
{
}

void TutorFramework::InitObjects()
{
	font_ = KlayGE::Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	KlayGE::AABBox boxRange(KlayGE::float3(-1.0f,-0.25f,-0.25f), KlayGE::float3(-0.5f, 0.25f, 0.25f));
	KlayGE::Color boxColor(1.0f, 0.0f, 0.0f, 1.0f);

	renderableBox_ = KlayGE::MakeSharedPtr<KlayGE::SceneObjectHelper>(
		KlayGE::MakeSharedPtr<KlayGE::RenderableTriBox>(boxRange, boxColor), KlayGE::SceneObject::SOA_Cullable);
	renderableBox_->AddToSceneManager();

	KlayGE::RenderModelPtr loadedModel = KlayGE::SyncLoadModel("teapot.meshml", KlayGE::EAH_GPU_Read,
		KlayGE::CreateModelFactory<KlayGE::RenderModel>(), KlayGE::CreateMeshFactory<RenderPolygon>());

	renderableFile_ = KlayGE::MakeSharedPtr<KlayGE::SceneObjectHelper>(loadedModel, KlayGE::SceneObject::SOA_Cullable);
	renderableFile_->AddToSceneManager();

	std::vector<KlayGE::float3> vertices;
	vertices.push_back(KlayGE::float3(0.5f,-0.25f, 0.25f));
	vertices.push_back(KlayGE::float3(1.0f,-0.25f, 0.25f));
	vertices.push_back(KlayGE::float3(1.0f,-0.25f,-0.25f));
	vertices.push_back(KlayGE::float3(0.5f,-0.25f,-0.25f));
	vertices.push_back(KlayGE::float3(0.5f, 0.25f, 0.25f));
	vertices.push_back(KlayGE::float3(1.0f, 0.25f, 0.25f));
	vertices.push_back(KlayGE::float3(1.0f, 0.25f,-0.25f));
	vertices.push_back(KlayGE::float3(0.5f, 0.25f,-0.25f));

	KlayGE::RenderModelPtr model = KlayGE::MakeSharedPtr<KlayGE::RenderModel>(L"model");

	std::vector<KlayGE::StaticMeshPtr> meshes(2);

	std::vector<KlayGE::uint16_t> indices1;
	indices1.push_back(0); indices1.push_back(4); indices1.push_back(1); indices1.push_back(5);
	indices1.push_back(2); indices1.push_back(6); indices1.push_back(3); indices1.push_back(7);
	indices1.push_back(0); indices1.push_back(4);

	meshes[0] = KlayGE::MakeSharedPtr<RenderPolygon>(model, L"side_mesh");

	meshes[0]->AddVertexStream(&vertices[0], static_cast<KlayGE::uint32_t>(sizeof(vertices[0]) * vertices.size()),
		KlayGE::vertex_element(KlayGE::VEU_Position, 0, KlayGE::EF_BGR32F), KlayGE::EAH_GPU_Read);

	meshes[0]->AddIndexStream(&indices1[0], static_cast<KlayGE::uint32_t>(sizeof(indices1[0]) * indices1.size()),
		KlayGE::EF_R16UI, KlayGE::EAH_GPU_Read);

	meshes[0]->GetRenderLayout()->TopologyType(KlayGE::RenderLayout::TT_TriangleStrip);

	std::vector<KlayGE::uint16_t> indices2;
	indices2.push_back(0); indices2.push_back(1); indices2.push_back(2);
	indices2.push_back(0); indices2.push_back(2); indices2.push_back(3);
	indices2.push_back(7); indices2.push_back(6); indices2.push_back(5);
	indices2.push_back(7); indices2.push_back(5); indices2.push_back(4);

	meshes[1] = KlayGE::MakeSharedPtr<RenderPolygon>(model, L"cap_mesh");
	meshes[1]->AddVertexStream(&vertices[0], static_cast<KlayGE::uint32_t>(sizeof(vertices[0]) * vertices.size()),
		KlayGE::vertex_element(KlayGE::VEU_Position, 0, KlayGE::EF_BGR32F), KlayGE::EAH_GPU_Read);
	meshes[1]->AddIndexStream(&indices2[0], static_cast<KlayGE::uint32_t>(sizeof(indices2[0]) * indices2.size()),
		KlayGE::EF_R16UI, KlayGE::EAH_GPU_Read);
	meshes[1]->GetRenderLayout()->TopologyType(KlayGE::RenderLayout::TT_TriangleList);

	model->AssignMeshes(meshes.begin(), meshes.end());

	renderableMesh_ = KlayGE::MakeSharedPtr<KlayGE::SceneObjectHelper>(model, KlayGE::SceneObject::SOA_Cullable);
	renderableMesh_->AddToSceneManager();

	this->LookAt(KlayGE::float3(0, 0,-4.0f), KlayGE::float3(0, 0, 0));
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

uint32_t TutorFramework::DoUpdate(uint32_t /*pass*/)
{
	KlayGE::RenderEngine& re = KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	re.CurFrameBuffer()->Clear(KlayGE::FrameBuffer::CBM_Color | KlayGE::FrameBuffer::CBM_Depth,
		KlayGE::Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	return KlayGE::App3DFramework::URV_Need_Flush | KlayGE::App3DFramework::URV_Finished;
}


RenderPolygon::RenderPolygon(KlayGE::RenderModelPtr const & model, std::wstring const& name)
	: KlayGE::StaticMesh(model, name)
{
	KlayGE::RenderFactory& rf = KlayGE::Context::Instance().RenderFactoryInstance();
	KlayGE::RenderEffectPtr effect = rf.LoadEffect("RenderableHelper.fxml");

	this->SetRenderTechnique(effect->TechniqueByName("TriangleTec"));

	*(effect->ParameterByName("color")) = KlayGE::float4(1.0f, 0.0f, 0.0f, 1.0f);
}

void RenderPolygon::OnRenderBegin()
{
	KlayGE::App3DFramework const & app = KlayGE::Context::Instance().AppInstance();

	KlayGE::float4x4 view_proj = app.ActiveCamera().ViewMatrix() * app.ActiveCamera().ProjMatrix();
	*(this->GetRenderTechnique()->Effect().ParameterByName("matViewProj")) = view_proj;
}
