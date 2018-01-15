#include <KlayGE/KlayGE.hpp>

#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include "../KGEditorCore/KGEditorCore.hpp"

#include "KGEditorCoreWrapper.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace KlayGE
{
	std::string StringToStd(String^ str)
	{
		using namespace System::Runtime::InteropServices;
		char const * char_str = static_cast<char const *>((Marshal::StringToHGlobalAnsi(str)).ToPointer());
		std::string std_str = char_str;
		Marshal::FreeHGlobal(IntPtr(static_cast<void*>(const_cast<char*>(char_str))));
		return std_str;
	}

	String^ StdToString(std::string const & str)
	{
		return gcnew String(str.c_str());
	}


	KGEditorCoreWrapper::KGEditorCoreWrapper(IntPtr native_wnd)
	{
		Context::Instance().LoadCfg("KlayGE.cfg");

		ContextCfg cfg = Context::Instance().Config();
		cfg.deferred_rendering = true;
		Context::Instance().Config(cfg);

		core_ = new KGEditorCore(native_wnd.ToPointer());
		core_->Create();
	}

	KGEditorCoreWrapper::~KGEditorCoreWrapper()
	{
		delete core_;
	}

	void KGEditorCoreWrapper::Refresh()
	{
		core_->Refresh();
	}

	void KGEditorCoreWrapper::Resize(uint32_t width, uint32_t height)
	{
		core_->Resize(width, height);
	}

	void KGEditorCoreWrapper::LoadScene(String^ name)
	{
		core_->LoadScene(StringToStd(name));
	}

	void KGEditorCoreWrapper::SaveScene(String^ name)
	{
		core_->SaveScene(StringToStd(name));
	}

	void KGEditorCoreWrapper::CloseScene()
	{
		core_->CloseScene();
	}

	System::String^ KGEditorCoreWrapper::SceneName()
	{
		return StdToString(core_->SceneName());
	}

	void KGEditorCoreWrapper::SceneName(String^ name)
	{
		core_->SceneName(StringToStd(name));
	}

	System::String^ KGEditorCoreWrapper::SkyboxName()
	{
		return StdToString(core_->SkyboxName());
	}

	void KGEditorCoreWrapper::SkyboxName(String^ name)
	{
		core_->SkyboxName(StringToStd(name));
	}

	void KGEditorCoreWrapper::DisplaySSVO(bool ssvo)
	{
		core_->DisplaySSVO(ssvo);
	}

	void KGEditorCoreWrapper::DisplayHDR(bool hdr)
	{
		core_->DisplayHDR(hdr);
	}

	void KGEditorCoreWrapper::DisplayAA(bool aa)
	{
		core_->DisplayAA(aa);
	}

	void KGEditorCoreWrapper::DisplayGamma(bool gamma)
	{
		core_->DisplayGamma(gamma);
	}

	void KGEditorCoreWrapper::DisplayColorGrading(bool cg)
	{
		core_->DisplayColorGrading(cg);
	}

	KGEditorCoreWrapper::ControlMode KGEditorCoreWrapper::GetControlMode()
	{
		return static_cast<KGEditorCoreWrapper::ControlMode>(core_->GetControlMode());
	}

	void KGEditorCoreWrapper::SetControlMode(KGEditorCoreWrapper::ControlMode mode)
	{
		core_->SetControlMode(static_cast<KGEditorCore::ControlMode>(mode));
	}

	uint32_t KGEditorCoreWrapper::AddModel(String^ meshml_name)
	{
		return core_->AddModel(StringToStd(meshml_name));
	}

	void KGEditorCoreWrapper::ClearModels()
	{
		core_->ClearModels();
	}

	uint32_t KGEditorCoreWrapper::AddLight(LightType type, String^ name)
	{
		return core_->AddLight(static_cast<LightSource::LightType>(type), StringToStd(name));
	}

	void KGEditorCoreWrapper::ClearLights()
	{
		core_->ClearLights();
	}

	uint32_t KGEditorCoreWrapper::AddCamera(System::String^ name)
	{
		return core_->AddCamera(StringToStd(name));
	}

	void KGEditorCoreWrapper::ClearCameras()
	{
		core_->ClearCameras();
	}

	uint32_t KGEditorCoreWrapper::NumEntities()
	{
		return core_->NumEntities();
	}

	uint32_t KGEditorCoreWrapper::EntityIDByIndex(uint32_t index)
	{
		return core_->EntityIDByIndex(index);
	}

	void KGEditorCoreWrapper::RemoveEntity(uint32_t id)
	{
		core_->RemoveEntity(id);
	}

	void KGEditorCoreWrapper::SelectEntity(uint32_t id)
	{
		core_->SelectEntity(id);
	}

	uint32_t KGEditorCoreWrapper::SelectedEntity()
	{
		return core_->SelectedEntity();
	}

	System::String^ KGEditorCoreWrapper::EntityName(uint32_t id)
	{
		return StdToString(core_->EntityName(id));
	}

	void KGEditorCoreWrapper::EntityName(uint32_t id, System::String^ name)
	{
		core_->EntityName(id, StringToStd(name));
	}

	bool KGEditorCoreWrapper::HideEntity(uint32_t id)
	{
		return core_->HideEntity(id);
	}

	void KGEditorCoreWrapper::HideEntity(uint32_t id, bool hide)
	{
		return core_->HideEntity(id, hide);
	}

	KGEditorCoreWrapper::EntityType KGEditorCoreWrapper::GetEntityType(uint32_t id)
	{
		return static_cast<KGEditorCoreWrapper::EntityType>(core_->GetEntityType(id));
	}

	KGEditorCoreWrapper::LightType KGEditorCoreWrapper::GetLightType(uint32_t id)
	{
		return static_cast<KGEditorCoreWrapper::LightType>(core_->GetLight(id)->Type());
	}

	bool KGEditorCoreWrapper::LightEnabled(uint32_t id)
	{
		return core_->GetLight(id)->Enabled();
	}

	void KGEditorCoreWrapper::LightEnabled(uint32_t id, bool enabled)
	{
		core_->GetLight(id)->Enabled(enabled);
	}

	int32_t KGEditorCoreWrapper::LightAttrib(uint32_t id)
	{
		return core_->GetLight(id)->Attrib();
	}

	void KGEditorCoreWrapper::LightAttrib(uint32_t id, int32_t attrib)
	{
		core_->GetLight(id)->Attrib(attrib);
	}

	array<float>^ KGEditorCoreWrapper::LightColor(uint32_t id)
	{
		auto const & clr = core_->GetLight(id)->Color();
		array<float>^ ret = { clr.x(), clr.y(), clr.z() };
		return ret;
	}

	void KGEditorCoreWrapper::LightColor(uint32_t id, array<float>^ color)
	{
		float r = color[0];
		float g = color[1];
		float b = color[2];
		core_->GetLight(id)->Color(float3(r, g, b));
	}

	array<float>^ KGEditorCoreWrapper::LightFalloff(uint32_t id)
	{
		auto const & falloff = core_->GetLight(id)->Falloff();
		array<float>^ ret = { falloff.x(), falloff.y(), falloff.z() };
		return ret;
	}

	void KGEditorCoreWrapper::LightFalloff(uint32_t id, array<float>^ falloff)
	{
		float x = falloff[0];
		float y = falloff[1];
		float z = falloff[2];
		core_->GetLight(id)->Falloff(float3(x, y, z));
	}

	float KGEditorCoreWrapper::LightInnerAngle(uint32_t id)
	{
		return acos(core_->GetLight(id)->CosInnerAngle());
	}

	void KGEditorCoreWrapper::LightInnerAngle(uint32_t id, float angle)
	{
		core_->GetLight(id)->InnerAngle(angle);
	}

	float KGEditorCoreWrapper::LightOuterAngle(uint32_t id)
	{
		return acos(core_->GetLight(id)->CosOuterAngle());
	}

	void KGEditorCoreWrapper::LightOuterAngle(uint32_t id, float angle)
	{
		core_->GetLight(id)->OuterAngle(angle);
	}

	System::String^ KGEditorCoreWrapper::LightProjectiveTex(uint32_t id)
	{
		return StdToString(core_->LightProjectiveTexName(id));
	}

	void KGEditorCoreWrapper::LightProjectiveTex(uint32_t id, System::String^ name)
	{
		core_->LightProjectiveTexName(id, StringToStd(name));
	}

	array<float>^ KGEditorCoreWrapper::CameraLookAt(uint32_t id)
	{
		auto const & look_at = core_->GetCamera(id)->LookAt();
		array<float>^ ret = { look_at.x(), look_at.y(), look_at.z() };
		return ret;
	}

	void KGEditorCoreWrapper::CameraLookAt(uint32_t id, array<float>^ look_at)
	{
		auto const & camera = core_->GetCamera(id);

		float x = look_at[0];
		float y = look_at[1];
		float z = look_at[2];
		camera->ViewParams(camera->EyePos(), float3(x, y, z), camera->UpVec());
	}
	
	array<float>^ KGEditorCoreWrapper::CameraUpVec(uint32_t id)
	{
		auto const & up_vec = core_->GetCamera(id)->UpVec();
		array<float>^ ret = { up_vec.x(), up_vec.y(), up_vec.z() };
		return ret;
	}

	void KGEditorCoreWrapper::CameraUpVec(uint32_t id, array<float>^ up_vec)
	{
		auto const & camera = core_->GetCamera(id);

		float x = up_vec[0];
		float y = up_vec[1];
		float z = up_vec[2];
		camera->ViewParams(camera->EyePos(), camera->LookAt(), float3(x, y, z));
	}

	float KGEditorCoreWrapper::CameraFoV(uint32_t id)
	{
		return core_->GetCamera(id)->FOV();
	}

	void KGEditorCoreWrapper::CameraFoV(uint32_t id, float fov)
	{
		auto const & camera = core_->GetCamera(id);
		camera->ProjParams(fov, camera->Aspect(), camera->NearPlane(), camera->FarPlane());
	}

	float KGEditorCoreWrapper::CameraAspect(uint32_t id)
	{
		return core_->GetCamera(id)->Aspect();
	}

	void KGEditorCoreWrapper::CameraAspect(uint32_t id, float aspect)
	{
		auto const & camera = core_->GetCamera(id);
		camera->ProjParams(camera->FOV(), aspect, camera->NearPlane(), camera->FarPlane());
	}

	float KGEditorCoreWrapper::CameraNearPlane(uint32_t id)
	{
		return core_->GetCamera(id)->NearPlane();
	}

	void KGEditorCoreWrapper::CameraNearPlane(uint32_t id, float near_plane)
	{
		auto const & camera = core_->GetCamera(id);
		camera->ProjParams(camera->FOV(), camera->Aspect(), near_plane, camera->FarPlane());
	}

	float KGEditorCoreWrapper::CameraFarPlane(uint32_t id)
	{
		return core_->GetCamera(id)->FarPlane();
	}

	void KGEditorCoreWrapper::CameraFarPlane(uint32_t id, float far_plane)
	{
		auto const & camera = core_->GetCamera(id);
		camera->ProjParams(camera->FOV(), camera->Aspect(), camera->NearPlane(), far_plane);
	}

	array<float>^ KGEditorCoreWrapper::EntityScaling(uint32_t id)
	{
		auto const & scaling = core_->EntityScaling(id);
		array<float>^ ret = { scaling.x(), scaling.y(), scaling.z() };
		return ret;
	}

	void KGEditorCoreWrapper::EntityScaling(uint32_t id, array<float>^ s)
	{
		float x = s[0];
		float y = s[1];
		float z = s[2];
		core_->EntityScaling(id, float3(x, y, z));
	}

	array<float>^ KGEditorCoreWrapper::EntityRotation(uint32_t id)
	{
		auto const & quat = core_->EntityRotation(id);
		array<float>^ ret = { quat.x(), quat.y(), quat.z(), quat.w() };
		return ret;
	}

	void KGEditorCoreWrapper::EntityRotation(uint32_t id, array<float>^ r)
	{
		core_->EntityRotation(id, Quaternion(r[0], r[1], r[2], r[3]));
	}

	array<float>^ KGEditorCoreWrapper::EntityTranslation(uint32_t id)
	{
		auto const & trans = core_->EntityTranslation(id);
		array<float>^ ret = { trans.x(), trans.y(), trans.z() };
		return ret;
	}

	void KGEditorCoreWrapper::EntityTranslation(uint32_t id, array<float>^ t)
	{
		float x = t[0];
		float y = t[1];
		float z = t[2];
		core_->EntityTranslation(id, float3(x, y, z));
	}

	uint32_t KGEditorCoreWrapper::ActiveCamera()
	{
		return core_->ActiveCameraID();
	}

	void KGEditorCoreWrapper::ActiveCamera(uint32_t id)
	{
		core_->ActiveCameraID(id);
	}

	uint32_t KGEditorCoreWrapper::BackupEntityInfo(uint32_t id)
	{
		return core_->BackupEntityInfo(id);
	}

	void KGEditorCoreWrapper::RestoreEntityInfo(uint32_t id, uint32_t backup_id)
	{
		core_->RestoreEntityInfo(id, backup_id);
	}

	void KGEditorCoreWrapper::MouseMove(int x, int y, uint32_t button)
	{
		core_->MouseMove(x, y, button);
	}

	void KGEditorCoreWrapper::MouseUp(int x, int y, uint32_t button)
	{
		core_->MouseUp(x, y, button);
	}

	void KGEditorCoreWrapper::MouseDown(int x, int y, uint32_t button)
	{
		core_->MouseDown(x, y, button);
	}

	void KGEditorCoreWrapper::UpdatePropertyCallback(UpdatePropertyDelegate^ callback)
	{
		update_property_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdatePropertyCallback(static_cast<KGEditorCore::UpdatePropertyEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::UpdateSelectEntityCallback(UpdateSelectEntityDelegate^ callback)
	{
		update_select_entity_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdateSelectEntityCallback(static_cast<KGEditorCore::UpdateSelectEntityEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::UpdateAddEntityCallback(UpdateAddEntityDelegate^ callback)
	{
		update_add_entity_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdateAddEntityCallback(static_cast<KGEditorCore::UpdateAddEntityEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::UpdateRemoveEntityCallback(UpdateRemoveEntityDelegate^ callback)
	{
		update_remove_entity_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->UpdateRemoveEntityCallback(static_cast<KGEditorCore::UpdateRemoveEntityEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::AddModelCallback(AddModelDelegate^ callback)
	{
		add_model_delegate_ = callback;
		add_model_delegate_with_c_string_ = gcnew AddModelDelegateWithCString(this,
			&KGEditorCoreWrapper::AddModelDelegateWithCStringFunc);

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(add_model_delegate_with_c_string_);
		core_->AddModelCallback(static_cast<KGEditorCore::AddModelEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::AddLightCallback(AddLightDelegate^ callback)
	{
		add_light_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->AddLightCallback(static_cast<KGEditorCore::AddLightEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::AddCameraCallback(AddCameraDelegate^ callback)
	{
		add_camera_delegate_ = callback;

		IntPtr ip = Marshal::GetFunctionPointerForDelegate(callback);
		core_->AddCameraCallback(static_cast<KGEditorCore::AddCameraEvent>(ip.ToPointer()));
	}

	void KGEditorCoreWrapper::AddModelDelegateWithCStringFunc(char const * name)
	{
		add_model_delegate_(StdToString(name));
	}
}
