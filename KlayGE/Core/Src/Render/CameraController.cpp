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

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4512)
#endif
#include <boost/assign.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include <KlayGE/CameraController.hpp>

#include <iostream>

namespace KlayGE
{
	CameraController::CameraController()
		: rotationScaler_(0.05f), moveScaler_(1),
			camera_(NULL)
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
					InputActionDefine(RollLeft, KS_Q),
					InputActionDefine(RollRight, KS_E),
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

		action_handler_t input_handler(new input_signal);
		input_handler->connect(boost::bind(&FirstPersonCameraController::InputHandler, this, _1, _2));
		inputEngine.ActionMap(actionMap, input_handler, true);
	}

	void FirstPersonCameraController::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
	{
		if (camera_)
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

			case RollLeft:
				this->Rotate(0, 0, -scaler);
				break;

			case RollRight:
				this->Rotate(0, 0, scaler);
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
	}

	void FirstPersonCameraController::AttachCamera(Camera& camera)
	{
		Quaternion quat = MathLib::to_quaternion(camera.ViewMatrix());
		float sqx = quat.x() * quat.x();
		float sqy = quat.y() * quat.y();
		float sqz = quat.z() * quat.z();
		float sqw = quat.w() * quat.w();
		float unit = sqx + sqy + sqz + sqw;
		float test = quat.w() * quat.x() + quat.y() * quat.z();
		float yaw, pitch, roll;
		if (test > 0.499f * unit)
		{
			// singularity at north pole
			yaw = 2 * atan2(quat.z(), quat.w());
			pitch = PI / 2;
			roll = 0;
		}
		else
		{
			if (test < -0.499f * unit)
			{
				// singularity at south pole
				yaw = -2 * atan2(quat.z(), quat.w());
				pitch = -PI / 2;
				roll = 0;
			}
			else
			{
				yaw = atan2(2 * (quat.y() * quat.w() - quat.x() * quat.z()), -sqx - sqy + sqz + sqw);
				pitch = asin(2 * test / unit);
				roll = atan2(2 * (quat.z() * quat.w() - quat.x() * quat.y()), -sqx + sqy - sqz + sqw);
			}
		}

		MathLib::sin_cos(pitch / 2, rot_x_.x(), rot_x_.y());
		MathLib::sin_cos(yaw / 2, rot_y_.x(), rot_y_.y());
		MathLib::sin_cos(roll / 2, rot_z_.x(), rot_z_.y());

		CameraController::AttachCamera(camera);
	}

	void FirstPersonCameraController::Update()
	{
		if (camera_)
		{
			float4x4 mat = camera_->ViewMatrix();

			world_ = MathLib::inverse(mat);

			elapsed_time_ = static_cast<float>(timer_.elapsed());
			if (elapsed_time_ > 0.01f)
			{
				timer_.restart();
			}
		}
	}

	void FirstPersonCameraController::Move(float x, float y, float z)
	{
		BOOST_ASSERT(camera_ != NULL);

		float3 movement(x, y, z);
		movement *= moveScaler_;

		float3 eyePos = camera_->EyePos();
		float3 viewVec = camera_->ViewVec();

		eyePos = MathLib::transform_coord(movement, world_);

		camera_->ViewParams(eyePos, eyePos + viewVec, camera_->UpVec());
	}

	void FirstPersonCameraController::Rotate(float yaw, float pitch, float roll)
	{
		BOOST_ASSERT(camera_ != NULL);

		pitch *= -rotationScaler_ / 2;
		yaw *= -rotationScaler_ / 2;
		roll *= -rotationScaler_ / 2;

		float2 delta_x, delta_y, delta_z;
		MathLib::sin_cos(pitch, delta_x.x(), delta_x.y());
		MathLib::sin_cos(yaw, delta_y.x(), delta_y.y());
		MathLib::sin_cos(roll, delta_z.x(), delta_z.y());

		Quaternion quat_x = Quaternion(rot_x_.x() * delta_x.y() + rot_x_.y() * delta_x.x(), 0, 0, rot_x_.y() * delta_x.y() - rot_x_.x() * delta_x.x());
		Quaternion quat_y = Quaternion(0, rot_y_.x() * delta_y.y() + rot_y_.y() * delta_y.x(), 0, rot_y_.y() * delta_y.y() - rot_y_.x() * delta_y.x());
		Quaternion quat_z = Quaternion(0, 0, rot_z_.x() * delta_z.y() + rot_z_.y() * delta_z.x(), rot_z_.y() * delta_z.y() - rot_z_.x() * delta_z.x());

		rot_x_ = float2(quat_x.x(), quat_x.w());
		rot_y_ = float2(quat_y.y(), quat_y.w());
		rot_z_ = float2(quat_z.z(), quat_z.w());

		float4x4 rot = MathLib::to_matrix(quat_y * quat_x * quat_z);
		float4x4 inv_rot = MathLib::inverse(rot);
		float3 view_vec = MathLib::transform_normal(float3(0, 0, 1), inv_rot);
		float3 up_vec = MathLib::transform_normal(float3(0, 1, 0), inv_rot);

		camera_->ViewParams(camera_->EyePos(), camera_->EyePos() + view_vec, up_vec);
	}
}
