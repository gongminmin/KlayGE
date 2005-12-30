// CameraController.cpp
// KlayGE 摄像机控制器类 实现文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了timer (2005.8.2)
//
// 2.5.0
// AttachCamera内增加了Update (2005.3.30)
// 自动处理输入 (2005.4.3)
//
// 2.4.0
// 初次建立 (2005.3.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/InputFactory.hpp>

#pragma warning(disable: 4512)

#include <boost/assign.hpp>
#include <boost/assign/std/vector.hpp>

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
		timer_.restart();
		camera_ = &camera;
		this->Update();
	}

	void CameraController::DetachCamera()
	{
		camera_ = NULL;
	}

	void CameraController::Update()
	{
	}


	FirstPersonCameraController::FirstPersonCameraController()
	{
		using namespace boost::assign;

		std::vector<InputActionDefine> actions;
		actions += InputActionDefine(TurnLeftRight, MS_X),
					InputActionDefine(TurnUpDown, MS_Y),
					InputActionDefine(Forward, KS_W),
					InputActionDefine(Backward, KS_S),
					InputActionDefine(MoveLeft, KS_A),
					InputActionDefine(MoveRight, KS_D),
					InputActionDefine(Forward, KS_UpArrow),
					InputActionDefine(Backward, KS_DownArrow),
					InputActionDefine(MoveLeft, KS_LeftArrow),
					InputActionDefine(MoveRight, KS_RightArrow);

		InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
		KlayGE::InputActionMap actionMap;
		actionMap.AddActions(actions.begin(), actions.end());

		action_handler_t input_handler(inputEngine);
		input_handler += boost::bind(&FirstPersonCameraController::InputHandler, this, _1, _2);
		inputEngine.ActionMap(actionMap, input_handler, true);
	}

	void FirstPersonCameraController::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
	{
		float const scaler = elapsed_time_ * 10;

		switch (action.first)
		{
		case TurnLeftRight:
			this->Rotate(action.second * scaler, 0, 0);
			break;

		case TurnUpDown:
			this->Rotate(0, action.second * scaler, 0);
			break;

		case Forward:
			this->Move(0, 0, scaler);
			break;

		case Backward:
			this->Move(0, 0, -scaler);
			break;

		case MoveLeft:
			this->Move(-scaler, 0, 0);
			break;

		case MoveRight:
			this->Move(scaler, 0, 0);
			break;
		}
	}

	void FirstPersonCameraController::Update()
	{
		BOOST_ASSERT(camera_ != NULL);

		world_ = MathLib::Inverse(camera_->ViewMatrix());

		elapsed_time_ = static_cast<float>(timer_.elapsed());
		timer_.restart();
	}

	void FirstPersonCameraController::Move(float x, float y, float z)
	{
		BOOST_ASSERT(camera_ != NULL);

		Vector3 movement(x, y, z);
		movement *= moveScaler_;

		Vector3 const & viewVec = camera_->ViewVec();
		Vector3 const eyePos = MathLib::TransformCoord(movement, world_);

		camera_->ViewParams(eyePos, eyePos + viewVec, camera_->UpVec());
	}

	void FirstPersonCameraController::Rotate(float yaw, float pitch, float roll)
	{
		BOOST_ASSERT(camera_ != NULL);

		Matrix4 rot(MathLib::RotationMatrixYawPitchRoll(yaw * rotationScaler_,
			pitch * rotationScaler_, roll * rotationScaler_));

		Vector3 const viewVec = MathLib::TransformCoord(Vector3(rot(2, 0), rot(2, 1), rot(2, 2)), world_);
		camera_->ViewParams(camera_->EyePos(), viewVec, camera_->UpVec());
	}
}
