// CameraController.cpp
// KlayGE 摄像机控制器类 实现文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 初次建立 (2005.3.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/CameraController.hpp>

namespace KlayGE
{
	CameraController::CameraController()
		: rotationScaler_(0.05f), moveScaler_(1)
	{
	}

	CameraController::~CameraController()
	{
	}

	void CameraController::Scalers(float rotationScaler, float moveScaler)
	{
		rotationScaler_ = rotationScaler;
		moveScaler_ = moveScaler;
	}

	void CameraController::AttachCamera(Camera& camera)
	{
		camera_ = &camera;
	}

	void CameraController::DeattachCamera()
	{
		camera_ = NULL;
	}

	void CameraController::Update()
	{
	}


	FirstPersonCameraController::FirstPersonCameraController()
	{
	}

	void FirstPersonCameraController::Update()
	{
		assert(camera_ != NULL);

		MathLib::Inverse(world_, camera_->ViewMatrix());
	}

	void FirstPersonCameraController::Move(float x, float y, float z)
	{
		assert(camera_ != NULL);

		Vector3 movement(x, y, z);
		movement *= moveScaler_;

		Vector3 eyePos = this->EyePos();
		Vector3 viewVec = this->WorldAhead();
		Vector3 upVec = this->WorldUp();

		MathLib::TransformCoord(eyePos, movement, world_);

		camera_->ViewParams(eyePos, eyePos + viewVec, upVec);

		this->Update();
	}

	void FirstPersonCameraController::Rotate(float yaw, float pitch, float roll)
	{
		assert(camera_ != NULL);

		Vector3 eyePos = this->EyePos();
		Vector3 upVec = this->WorldUp();

		Quaternion rot;
		MathLib::RotationYawPitchRoll(rot, yaw * rotationScaler_, pitch * rotationScaler_, roll * rotationScaler_);

		Matrix4 rotMatrix;
		MathLib::ToMatrix(rotMatrix, rot);

		Vector3 viewVec;
		MathLib::TransformCoord(viewVec, Vector3(0, 0, 1), rotMatrix);
		MathLib::TransformCoord(viewVec, viewVec, world_);

		camera_->ViewParams(eyePos, viewVec, upVec);

		this->Update();
	}

	Vector3 FirstPersonCameraController::WorldRight() const
	{
		return Vector3(world_(0, 0), world_(0, 1), world_(0, 2));
	}

	Vector3 FirstPersonCameraController::WorldUp() const
	{
		return Vector3(world_(1, 0), world_(1, 1), world_(1, 2));
	}

	Vector3 FirstPersonCameraController::WorldAhead() const
	{
		return Vector3(world_(2, 0), world_(2, 1), world_(2, 2));
	}

    Vector3 FirstPersonCameraController::EyePos() const
	{
		return Vector3(world_(3, 0), world_(3, 1), world_(3, 2));
	}
}
