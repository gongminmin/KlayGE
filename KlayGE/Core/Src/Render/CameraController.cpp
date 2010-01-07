// CameraController.cpp
// KlayGE 摄像机控制器类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 增加了TrackballCameraController (2009.3.25)
//
// 3.7.0
// FirstPersonCameraController去掉了死角的限制 (2008.1.25)
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
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Color.hpp>
#include <KlayGE/UI.hpp>

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
		camera_ = &camera;
	}

	void CameraController::DetachCamera()
	{
		camera_ = NULL;
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

		action_handler_t input_handler = MakeSharedPtr<input_signal>();
		input_handler->connect(boost::bind(&FirstPersonCameraController::InputHandler, this, _1, _2));
		inputEngine.ActionMap(actionMap, input_handler, true);
	}

	void FirstPersonCameraController::InputHandler(InputEngine const & ie, InputAction const & action)
	{
		float elapsed_time = ie.ElapsedTime();
		if (camera_)
		{
			float const scaler = elapsed_time * 10;

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
		float3 scale, trans;
		Quaternion quat;
		MathLib::decompose(scale, quat, trans, camera.ViewMatrix());
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

		inv_rot_ = MathLib::inverse(quat);

		CameraController::AttachCamera(camera);
	}

	void FirstPersonCameraController::Move(float x, float y, float z)
	{
		if (camera_)
		{
			float3 movement(x, y, z);
			movement *= moveScaler_;

			float3 new_eye_pos = camera_->EyePos() + MathLib::transform_quat(movement, inv_rot_);

			camera_->ViewParams(new_eye_pos, new_eye_pos + camera_->ViewVec(), camera_->UpVec());
		}
	}

	void FirstPersonCameraController::Rotate(float yaw, float pitch, float roll)
	{
		if (camera_)
		{
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

			inv_rot_ = MathLib::inverse(quat_y * quat_x * quat_z);
			float3 view_vec = MathLib::transform_quat(float3(0, 0, 1), inv_rot_);
			float3 up_vec = MathLib::transform_quat(float3(0, 1, 0), inv_rot_);

			camera_->ViewParams(camera_->EyePos(), camera_->EyePos() + view_vec, up_vec);
		}
	}


	TrackballCameraController::TrackballCameraController()
	{
		using namespace boost::assign;

		std::vector<InputActionDefine> actions;
		actions += InputActionDefine(Turn, MS_X),
					InputActionDefine(Turn, MS_Y);

		InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
		KlayGE::InputActionMap actionMap;
		actionMap.AddActions(actions.begin(), actions.end());

		action_handler_t input_handler = MakeSharedPtr<input_signal>();
		input_handler->connect(boost::bind(&TrackballCameraController::InputHandler, this, _1, _2));
		inputEngine.ActionMap(actionMap, input_handler, true);
	}

	void TrackballCameraController::InputHandler(InputEngine const & ie, InputAction const & /*action*/)
	{
		if (camera_)
		{
			InputMousePtr mouse;
			for (uint32_t i = 0; i < ie.NumDevices(); ++ i)
			{
				InputMousePtr m = boost::dynamic_pointer_cast<InputMouse>(ie.Device(i));
				if (m)
				{
					mouse = m;
					break;
				}
			}

			if (mouse)
			{
				float xd = static_cast<float>(mouse->X());
				float yd = static_cast<float>(mouse->Y());

				bool mouse_on_ui = false;
				std::vector<UIDialogPtr> const & dlgs = UIManager::Instance().GetDialogs();
				for (size_t i = 0; i < dlgs.size(); ++ i)
				{
					if (dlgs[i]->GetVisible() && dlgs[i]->ContainsPoint(Vector_T<int32_t, 2>(mouse->AbsX(), mouse->AbsY())))
					{
						mouse_on_ui = true;
						break;
					}
				}

				if (!mouse_on_ui)
				{
					if (mouse->LeftButton())
					{
						Quaternion q = MathLib::rotation_axis(right_, yd * rotationScaler_);
						float4x4 mat = MathLib::transformation<float>(NULL, NULL, NULL, &target_, &q, NULL);
						float3 pos = MathLib::transform_coord(camera_->EyePos(), mat);

						q = MathLib::rotation_axis(float3(0.0f, MathLib::dot(camera_->UpVec(), float3(0, 1, 0)) < 0 ? -1.0f : 1.0f, 0.0f), xd * rotationScaler_);
						mat = MathLib::transformation<float>(NULL, NULL, NULL, &target_, &q, NULL);
						pos = MathLib::transform_coord(pos, mat);

						right_ = MathLib::transform_quat(right_, q);

						float3 dir;
						if (reverse_target_)
						{
							dir = pos - target_;
						}
						else
						{
							dir = target_ - pos;
						}
						dir = MathLib::normalize(dir);
						float3 up = MathLib::cross(dir, right_);

						camera_->ViewParams(pos, pos + dir, up);
					}
					else
					{
						if (mouse->MiddleButton())
						{
							float3 offset = right_ * (-xd * rotationScaler_ * 2);
							float3 pos = camera_->EyePos() + offset;
							target_ += offset;

							offset = camera_->UpVec() * (yd * rotationScaler_ * 2);
							pos += offset;
							target_ += offset;

							camera_->ViewParams(pos, target_, camera_->UpVec());

						}
						else
						{
							if (mouse->RightButton())
							{
								float3 offset = camera_->ViewVec() * ((xd + yd) * rotationScaler_ * 2);
								float3 pos = camera_->EyePos() + offset;

								if (MathLib::dot(target_ - pos, camera_->ViewVec()) <= 0)
								{
									reverse_target_ = true;
								}
								else
								{
									reverse_target_ = false;
								}

								camera_->ViewParams(pos, pos + camera_->ViewVec(), camera_->UpVec());
							}
						}
					}
				}
			}
		}
	}

	void TrackballCameraController::AttachCamera(Camera& camera)
	{
		CameraController::AttachCamera(camera);

		reverse_target_ = false;
		target_ = camera_->LookAt();
		right_ = MathLib::normalize(MathLib::cross(float3(0, 1, 0), -camera_->EyePos()));
	}
}
