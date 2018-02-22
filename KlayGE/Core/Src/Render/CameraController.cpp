// CameraController.cpp
// KlayGE 摄像机控制器类 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2005-2009
// Homepage: http://www.klayge.org
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
#include <KFL/CXX17/iterator.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/InputFactory.hpp>
#include <KFL/Math.hpp>
#include <KFL/Color.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Camera.hpp>
#include <KFL/XMLDom.hpp>
#include <KlayGE/App3D.hpp>
#include <KFL/Hash.hpp>

#include <sstream>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore unused variable (mpl_assertion_in_line_xxx) in boost
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <KlayGE/CameraController.hpp>

namespace KlayGE
{
	CameraController::CameraController()
		: rotationScaler_(0.05f), moveScaler_(1),
			camera_(nullptr)
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
		camera_ = nullptr;
	}


	FirstPersonCameraController::FirstPersonCameraController(bool use_input_engine)
		: left_button_down_(false)
	{
		if (use_input_engine)
		{
			InputActionDefine actions[] =
			{
				InputActionDefine(TurnLeftRight, MS_X),
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
				InputActionDefine(MoveRight, KS_RightArrow),
				InputActionDefine(Turn, TS_Pan),
				InputActionDefine(Turn, SS_OrientationQuat)
			};

			InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
			InputActionMap actionMap;
			actionMap.AddActions(actions, actions + std::size(actions));

			action_handler_t input_handler = MakeSharedPtr<input_signal>();
			input_handler->connect(
				[this](InputEngine const & ie, InputAction const & action)
				{
					this->InputHandler(ie, action);
				});
			inputEngine.ActionMap(actionMap, input_handler);
		}
	}

	void FirstPersonCameraController::RequiresLeftButtonDown(bool lbd)
	{
		left_button_down_ = lbd;
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
				{
					InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
					if ((left_button_down_ && (param->buttons_state & MB_Left)) || !left_button_down_)
					{
						this->RotateRel(param->move_vec.x() * scaler, 0, 0);
					}
				}
				break;

			case TurnUpDown:
				{
					InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
					if ((left_button_down_ && (param->buttons_state & MB_Left)) || !left_button_down_)
					{
						this->RotateRel(0, param->move_vec.y() * scaler, 0);
					}
				}
				break;

			case Turn:
				switch (action.second->type)
				{
				case InputEngine::IDT_Touch:
					{
						InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
						this->RotateRel(param->move_vec.x() * scaler, param->move_vec.y() * scaler, 0);
					}
					break;

				case InputEngine::IDT_Sensor:
					{
						InputSensorActionParamPtr param = checked_pointer_cast<InputSensorActionParam>(action.second);
						this->RotateAbs(param->orientation_quat);
					}
					break;

				default:
					KFL_UNREACHABLE("Invalid input type");
				}
				break;

			case RollLeft:
				this->RotateRel(0, 0, -scaler);
				break;

			case RollRight:
				this->RotateRel(0, 0, scaler);
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

		float yaw, pitch, roll;
		MathLib::to_yaw_pitch_roll(yaw, pitch, roll, quat);

		MathLib::sincos(pitch / 2, rot_x_.x(), rot_x_.y());
		MathLib::sincos(yaw / 2, rot_y_.x(), rot_y_.y());
		MathLib::sincos(roll / 2, rot_z_.x(), rot_z_.y());

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

			camera_->ViewParams(new_eye_pos, new_eye_pos + camera_->ForwardVec() * camera_->LookAtDist(),
				camera_->UpVec());
		}
	}

	void FirstPersonCameraController::RotateRel(float yaw, float pitch, float roll)
	{
		if (camera_)
		{
			pitch *= -rotationScaler_ / 2;
			yaw *= -rotationScaler_ / 2;
			roll *= -rotationScaler_ / 2;

			float2 delta_x, delta_y, delta_z;
			MathLib::sincos(pitch, delta_x.x(), delta_x.y());
			MathLib::sincos(yaw, delta_y.x(), delta_y.y());
			MathLib::sincos(roll, delta_z.x(), delta_z.y());

			Quaternion quat_x(rot_x_.x() * delta_x.y() + rot_x_.y() * delta_x.x(), 0, 0, rot_x_.y() * delta_x.y() - rot_x_.x() * delta_x.x());
			Quaternion quat_y(0, rot_y_.x() * delta_y.y() + rot_y_.y() * delta_y.x(), 0, rot_y_.y() * delta_y.y() - rot_y_.x() * delta_y.x());
			Quaternion quat_z(0, 0, rot_z_.x() * delta_z.y() + rot_z_.y() * delta_z.x(), rot_z_.y() * delta_z.y() - rot_z_.x() * delta_z.x());

			rot_x_ = float2(quat_x.x(), quat_x.w());
			rot_y_ = float2(quat_y.y(), quat_y.w());
			rot_z_ = float2(quat_z.z(), quat_z.w());

			inv_rot_ = MathLib::inverse(quat_y * quat_x * quat_z);
			float3 view_vec = MathLib::transform_quat(float3(0, 0, 1), inv_rot_);
			float3 up_vec = MathLib::transform_quat(float3(0, 1, 0), inv_rot_);

			camera_->ViewParams(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec);
		}
	}

	void FirstPersonCameraController::RotateAbs(Quaternion const & quat)
	{
		if (camera_)
		{
			float yaw, pitch, roll;
			MathLib::to_yaw_pitch_roll(yaw, pitch, roll, quat);

			MathLib::sincos(pitch / 2, rot_x_.x(), rot_x_.y());
			MathLib::sincos(yaw / 2, rot_y_.x(), rot_y_.y());
			MathLib::sincos(roll / 2, rot_z_.x(), rot_z_.y());

			inv_rot_ = MathLib::inverse(quat);
			float3 view_vec = MathLib::transform_quat(float3(0, 0, 1), inv_rot_);
			float3 up_vec = MathLib::transform_quat(float3(0, 1, 0), inv_rot_);

			camera_->ViewParams(camera_->EyePos(), camera_->EyePos() + view_vec * camera_->LookAtDist(), up_vec);
		}
	}


	TrackballCameraController::TrackballCameraController(bool use_input_engine, uint32_t rotate_button,
		uint32_t zoom_button, uint32_t move_button)
			: move_button_(move_button), rotate_button_(rotate_button), zoom_button_(zoom_button)
	{
		if (use_input_engine)
		{
			InputActionDefine actions[] =
			{
				InputActionDefine(Turn, MS_X),
				InputActionDefine(Turn, MS_Y),
				InputActionDefine(Turn, TS_Pan),
				InputActionDefine(Turn, TS_Pan),
				InputActionDefine(ZoomInOut, TS_Zoom)
			};

			InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
			InputActionMap actionMap;
			actionMap.AddActions(actions, actions + std::size(actions));

			action_handler_t input_handler = MakeSharedPtr<input_signal>();
			input_handler->connect(
				[this](InputEngine const & ie, InputAction const & action)
				{
					this->InputHandler(ie, action);
				});
			inputEngine.ActionMap(actionMap, input_handler);
		}
	}

	void TrackballCameraController::InputHandler(InputEngine const & /*ie*/, InputAction const & action)
	{
		if (camera_ && !UIManager::Instance().MouseOnUI())
		{
			switch (action.first)
			{
			case Turn:
				switch (action.second->type)
				{
				case InputEngine::IDT_Mouse:
					{
						InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);

						float xd = static_cast<float>(param->move_vec.x());
						float yd = static_cast<float>(param->move_vec.y());

						if (param->buttons_state & rotate_button_)
						{
							this->Rotate(xd, yd);
						}
						else if (param->buttons_state & zoom_button_)
						{
							this->Zoom(xd, yd);
						}
						else if (param->buttons_state & move_button_)
						{
							this->Move(xd, yd);
						}
					}
					break;

				case InputEngine::IDT_Touch:
					{
						InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);

						float xd = static_cast<float>(param->move_vec.x());
						float yd = static_cast<float>(param->move_vec.y());

						this->Rotate(xd, yd);
					}
					break;

				default:
					break;
				}
				break;

			case ZoomInOut:
				switch (action.second->type)
				{
				case InputEngine::IDT_Touch:
					{
						InputTouchActionParamPtr param = checked_pointer_cast<InputTouchActionParam>(action.second);
						this->Zoom(param->zoom, 0);
					}
					break;

				default:
					break;
				}
				break;
			}
		}
	}

	void TrackballCameraController::AttachCamera(Camera& camera)
	{
		CameraController::AttachCamera(camera);

		reverse_target_ = false;
		target_ = camera_->LookAt();
		right_ = camera_->RightVec();
	}

	void TrackballCameraController::Move(float offset_x, float offset_y)
	{
		float3 offset = right_ * (-offset_x * moveScaler_ * 2);
		float3 pos = camera_->EyePos() + offset;
		target_ += offset;

		offset = camera_->UpVec() * (offset_y * moveScaler_ * 2);
		pos += offset;
		target_ += offset;

		camera_->ViewParams(pos, target_, camera_->UpVec());
	}

	void TrackballCameraController::Rotate(float offset_x, float offset_y)
	{
		Quaternion q = MathLib::rotation_axis(right_, offset_y * rotationScaler_);
		float4x4 mat = MathLib::transformation<float>(nullptr, nullptr, nullptr, &target_, &q, nullptr);
		float3 pos = MathLib::transform_coord(camera_->EyePos(), mat);

		q = MathLib::rotation_axis(float3(0.0f, MathLib::sgn(camera_->UpVec().y()), 0.0f), offset_x * rotationScaler_);
		mat = MathLib::transformation<float>(nullptr, nullptr, nullptr, &target_, &q, nullptr);
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
		float dist = MathLib::length(dir);
		dir /= dist;
		float3 up = MathLib::cross(dir, right_);

		camera_->ViewParams(pos, pos + dir * dist, up);
	}

	void TrackballCameraController::Zoom(float offset_x, float offset_y)
	{
		float3 offset = camera_->ForwardVec() * ((offset_x + offset_y) * moveScaler_ * 2);
		float3 pos = camera_->EyePos() + offset;

		if (MathLib::dot(target_ - pos, camera_->ForwardVec()) <= 0)
		{
			reverse_target_ = true;
		}
		else
		{
			reverse_target_ = false;
		}

		camera_->ViewParams(pos, pos + camera_->ForwardVec() * camera_->LookAtDist(), camera_->UpVec());
	}


	CameraPathController::CameraPathController()
		: curr_frame_(0), frame_rate_(30)
	{
	}

	uint32_t CameraPathController::NumCurves() const
	{
		return static_cast<uint32_t>(curves_.size());
	}

	uint32_t CameraPathController::AddCurve(CameraPathController::InterpolateType type, uint32_t num_frames)
	{
		uint32_t curve_id = static_cast<uint32_t>(curves_.size());
		CameraCurve curve;
		curve.type = type;
		curve.num_frames = num_frames;
		curves_.push_back(curve);
		return curve_id;
	}

	void CameraPathController::DelCurve(uint32_t curve_id)
	{
		BOOST_ASSERT(curve_id < curves_.size());

		curves_.erase(curves_.begin() + curve_id);
	}

	CameraPathController::InterpolateType CameraPathController::CurveType(uint32_t curve_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());

		return curves_[curve_id].type;
	}

	uint32_t CameraPathController::NumCurveFrames(uint32_t curve_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());

		return curves_[curve_id].num_frames;
	}

	uint32_t CameraPathController::NumControlPoints(uint32_t curve_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());

		return static_cast<uint32_t>(curves_[curve_id].eye_ctrl_pts.size());
	}

	uint32_t CameraPathController::AddControlPoint(uint32_t curve_id, float frame_id,
		float3 const & eye_ctrl_pt, float3 const & target_ctrl_pt,
		float3 const & up_ctrl_pt, bool corner)
	{
		BOOST_ASSERT(curve_id < curves_.size());

		CameraCurve& curve = curves_[curve_id];

		uint32_t pt_id = static_cast<uint32_t>(curve.eye_ctrl_pts.size());
		curve.frame_ids.push_back(frame_id);
		curve.eye_ctrl_pts.push_back(eye_ctrl_pt);
		curve.target_ctrl_pts.push_back(target_ctrl_pt);
		curve.up_ctrl_pts.push_back(up_ctrl_pt);
		curve.corners.push_back(corner);
		return pt_id;
	}

	void CameraPathController::DelControlPoint(uint32_t curve_id, uint32_t pt_id)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		curves_[curve_id].eye_ctrl_pts.erase(curves_[curve_id].eye_ctrl_pts.begin() + pt_id);
		curves_[curve_id].target_ctrl_pts.erase(curves_[curve_id].target_ctrl_pts.begin() + pt_id);
		curves_[curve_id].up_ctrl_pts.erase(curves_[curve_id].up_ctrl_pts.begin() + pt_id);
	}

	float CameraPathController::FrameID(uint32_t curve_id, uint32_t pt_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		return curves_[curve_id].frame_ids[pt_id];
	}

	float3 const & CameraPathController::EyeControlPoint(uint32_t curve_id, uint32_t pt_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		return curves_[curve_id].eye_ctrl_pts[pt_id];
	}

	float3 const & CameraPathController::TargetControlPoint(uint32_t curve_id, uint32_t pt_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].target_ctrl_pts.size());

		return curves_[curve_id].target_ctrl_pts[pt_id];
	}

	float3 const & CameraPathController::UpControlPoint(uint32_t curve_id, uint32_t pt_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].up_ctrl_pts.size());

		return curves_[curve_id].up_ctrl_pts[pt_id];
	}	

	bool CameraPathController::Corner(uint32_t curve_id, uint32_t pt_id) const
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		return curves_[curve_id].corners[pt_id];
	}

	void CameraPathController::FrameID(uint32_t curve_id, uint32_t pt_id, float frame_id)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		curves_[curve_id].frame_ids[pt_id] = frame_id;
	}

	void CameraPathController::EyeControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		curves_[curve_id].eye_ctrl_pts[pt_id] = pt;
	}

	void CameraPathController::TargetControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].target_ctrl_pts.size());

		curves_[curve_id].target_ctrl_pts[pt_id] = pt;
	}

	void CameraPathController::UpControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].up_ctrl_pts.size());

		curves_[curve_id].up_ctrl_pts[pt_id] = pt;
	}
	
	void CameraPathController::Corner(uint32_t curve_id, uint32_t pt_id, bool corner)
	{
		BOOST_ASSERT(curve_id < curves_.size());
		BOOST_ASSERT(pt_id < curves_[curve_id].eye_ctrl_pts.size());

		curves_[curve_id].corners[pt_id] = corner;
	}

	uint32_t CameraPathController::FrameRate() const
	{
		return frame_rate_;
	}

	void CameraPathController::FrameRate(uint32_t frame_rate)
	{
		frame_rate_ = frame_rate;
	}

	float CameraPathController::Frame() const
	{
		return curr_frame_;
	}

	void CameraPathController::Frame(float frame)
	{
		curr_frame_ = frame;
		this->UpdateCamera();
	}

	void CameraPathController::AttachCamera(Camera& camera)
	{
		CameraController::AttachCamera(camera);

		start_time_ = Context::Instance().AppInstance().AppTime();
		camera.BindUpdateFunc(
			[this](Camera& camera, float app_time, float elapsed_time)
			{
				this->UpdateCameraFunc(camera, app_time, elapsed_time);
			});
	}

	void CameraPathController::DetachCamera()
	{
		camera_->BindUpdateFunc(std::function<void(Camera&, float, float)>());

		CameraController::DetachCamera();
	}

	void CameraPathController::UpdateCameraFunc(Camera& /*camera*/, float app_time, float /*elapsed_time*/)
	{
		curr_frame_ = frame_rate_ * (app_time - start_time_);
		this->UpdateCamera();
	}

	void CameraPathController::UpdateCamera()
	{
		uint32_t total_frames = 0;
		for (size_t i = 0; i < curves_.size(); ++ i)
		{
			total_frames += curves_[i].num_frames;
		}

		float frame = MathLib::mod(curr_frame_, static_cast<float>(total_frames));
		for (size_t i = 0; i < curves_.size(); ++ i)
		{
			CameraCurve const & curve = curves_[i];
			if (frame < curve.num_frames)
			{
				switch (curve.type)
				{
				case IT_Linear:
					for (size_t j = 0; j < curve.eye_ctrl_pts.size() - 1; ++ j)
					{
						if ((frame >= curve.frame_ids[j + 0])
							&& (frame < curve.frame_ids[j + 1]))
						{
							float factor = (frame - curve.frame_ids[j + 0])
								/ (curve.frame_ids[j + 1] - curve.frame_ids[j + 0]);
							float3 eye = MathLib::lerp(curve.eye_ctrl_pts[j + 0],
								curve.eye_ctrl_pts[j + 1], factor);
							float3 target = MathLib::lerp(curve.target_ctrl_pts[j + 0],
								curve.target_ctrl_pts[j + 1], factor);
							float3 up = MathLib::lerp(curve.up_ctrl_pts[j + 0],
								curve.up_ctrl_pts[j + 1], factor);

							camera_->ViewParams(eye, target, up);

							break;
						}
					}
					break;

				case IT_CatmullRom:
					for (size_t j = 0; j < curve.eye_ctrl_pts.size() - 3; j += 3)
					{
						if ((frame >= curve.frame_ids[j + 0])
							&& (frame < curve.frame_ids[j + 3]))
						{
							float factor = (frame - curve.frame_ids[j + 0])
								/ (curve.frame_ids[j + 3] - curve.frame_ids[j + 0]);
							float3 eye = MathLib::catmull_rom(curve.eye_ctrl_pts[j + 0],
								curve.eye_ctrl_pts[j + 1], curve.eye_ctrl_pts[j + 2],
								curve.eye_ctrl_pts[j + 3], factor);
							float3 target = MathLib::catmull_rom(curve.target_ctrl_pts[j + 0],
								curve.target_ctrl_pts[j + 1], curve.target_ctrl_pts[j + 2],
								curve.target_ctrl_pts[j + 3], factor);
							float3 up = MathLib::catmull_rom(curve.up_ctrl_pts[j + 0],
								curve.up_ctrl_pts[j + 1], curve.up_ctrl_pts[j + 2], 
								curve.up_ctrl_pts[j + 3], factor);

							camera_->ViewParams(eye, target, up);

							break;
						}
					}
					break;

				case IT_BSpline:
					for (size_t j = 0; j < curve.eye_ctrl_pts.size() - 3; j += 3)
					{
						if ((frame >= curve.frame_ids[j + 0])
							&& (frame < curve.frame_ids[j + 3]))
						{
							float factor = (frame - curve.frame_ids[j + 0])
								/ (curve.frame_ids[j + 3] - curve.frame_ids[j + 0]);
							float3 eye = MathLib::cubic_b_spline(curve.eye_ctrl_pts[j + 0],
								curve.eye_ctrl_pts[j + 1], curve.eye_ctrl_pts[j + 2],
								curve.eye_ctrl_pts[j + 3], factor);
							float3 target = MathLib::cubic_b_spline(curve.target_ctrl_pts[j + 0],
								curve.target_ctrl_pts[j + 1], curve.target_ctrl_pts[j + 2],
								curve.target_ctrl_pts[j + 3], factor);
							float3 up = MathLib::cubic_b_spline(curve.up_ctrl_pts[j + 0],
								curve.up_ctrl_pts[j + 1], curve.up_ctrl_pts[j + 2], 
								curve.up_ctrl_pts[j + 3], factor);

							camera_->ViewParams(eye, target, up);

							break;
						}
					}
					break;

				case IT_Bezier:
					for (size_t j = 0; j < curve.eye_ctrl_pts.size() - 3; j += 3)
					{
						if ((frame >= curve.frame_ids[j + 0])
							&& (frame < curve.frame_ids[j + 3]))
						{
							float factor = (frame - curve.frame_ids[j + 0])
								/ (curve.frame_ids[j + 3] - curve.frame_ids[j + 0]);
							float3 eye = MathLib::cubic_bezier(curve.eye_ctrl_pts[j + 0],
								curve.eye_ctrl_pts[j + 1], curve.eye_ctrl_pts[j + 2],
								curve.eye_ctrl_pts[j + 3], factor);
							float3 target = MathLib::cubic_bezier(curve.target_ctrl_pts[j + 0],
								curve.target_ctrl_pts[j + 1], curve.target_ctrl_pts[j + 2],
								curve.target_ctrl_pts[j + 3], factor);
							float3 up = MathLib::cubic_bezier(curve.up_ctrl_pts[j + 0],
								curve.up_ctrl_pts[j + 1], curve.up_ctrl_pts[j + 2], 
								curve.up_ctrl_pts[j + 3], factor);

							camera_->ViewParams(eye, target, up);

							break;
						}
					}
					break;

				default:
					KFL_UNREACHABLE("Invalid interpolate type");
				}

				break;
			}
			else
			{
				frame -= curve.num_frames;
			}
		}
	}

	CameraPathControllerPtr LoadCameraPath(ResIdentifierPtr const & res)
	{
		CameraPathControllerPtr ret = MakeSharedPtr<CameraPathController>();

		XMLDocument doc;
		XMLNodePtr root = doc.Parse(res);

		XMLAttributePtr attr = root->Attrib("frame_rate");
		if (attr)
		{
			ret->FrameRate(attr->ValueUInt());
		}

		for (XMLNodePtr curve_node = root->FirstNode("curve"); curve_node; curve_node = curve_node->NextSibling("curve"))
		{
			std::string_view const type_str = curve_node->Attrib("type")->ValueString();
			size_t const type_str_hash = HashRange(type_str.begin(), type_str.end());
			CameraPathController::InterpolateType type;
			if (CT_HASH("linear") == type_str_hash)
			{
				type = CameraPathController::IT_Linear;
			}
			else if (CT_HASH("catmull_rom") == type_str_hash)
			{
				type = CameraPathController::IT_CatmullRom;
			}
			else if (CT_HASH("b_spline") == type_str_hash)
			{
				type = CameraPathController::IT_BSpline;
			}
			else if (CT_HASH("bezier") == type_str_hash)
			{
				type = CameraPathController::IT_Bezier;
			}
			else
			{
				KFL_UNREACHABLE("Invalid interpolate type");
			}
			uint32_t num_frames = curve_node->Attrib("num_frames")->ValueUInt();
			uint32_t curve_id = ret->AddCurve(type, num_frames);
			for (XMLNodePtr key_node = curve_node->FirstNode("key"); key_node; key_node = key_node->NextSibling("key"))
			{
				float frame_id = key_node->Attrib("frame")->ValueFloat();

				float3 eye_ctrl_pt;
				{
					std::istringstream attr_ss(std::string(key_node->Attrib("eye")->ValueString()));
					attr_ss >> eye_ctrl_pt.x() >> eye_ctrl_pt.y() >> eye_ctrl_pt.z();
				}				
				float3 target_ctrl_pt;
				{
					std::istringstream attr_ss(std::string(key_node->Attrib("target")->ValueString()));
					attr_ss >> target_ctrl_pt.x() >> target_ctrl_pt.y() >> target_ctrl_pt.z();
				}				
				float3 up_ctrl_pt;
				{
					std::istringstream attr_ss(std::string(key_node->Attrib("up")->ValueString()));
					attr_ss >> up_ctrl_pt.x() >> up_ctrl_pt.y() >> up_ctrl_pt.z();
				}

				bool corner;
				attr = key_node->Attrib("corner");
				if (attr)
				{
					corner = (attr->ValueInt() != 0);
				}
				else
				{
					corner = false;
				}

				ret->AddControlPoint(curve_id, frame_id, eye_ctrl_pt, target_ctrl_pt, up_ctrl_pt, corner);
			}
		}

		return ret;
	}

	void SaveCameraPath(std::ostream& os, CameraPathControllerPtr const & path)
	{
		XMLDocument doc;
		XMLNodePtr root = doc.AllocNode(XNT_Element, "camera_path");
		doc.RootNode(root);

		root->AppendAttrib(doc.AllocAttribUInt("frame_rate", path->FrameRate()));

		for (uint32_t curve_id = 0; curve_id < path->NumCurves(); ++ curve_id)
		{
			XMLNodePtr curve_node = doc.AllocNode(XNT_Element, "curve");
			std::string type_str;
			CameraPathController::InterpolateType type = path->CurveType(curve_id);
			switch (type)
			{
			case CameraPathController::IT_Linear:
				type_str = "linear";
				break;

			case CameraPathController::IT_CatmullRom:
				type_str = "catmull_rom";
				break;

			case CameraPathController::IT_BSpline:
				type_str = "b_spline";
				break;

			case CameraPathController::IT_Bezier:
				type_str = "bezier";
				break;

			default:
				KFL_UNREACHABLE("Invalid interpolate type");
			}
			curve_node->AppendAttrib(doc.AllocAttribString("type", type_str));
			curve_node->AppendAttrib(doc.AllocAttribUInt("num_frames", path->NumCurveFrames(curve_id)));

			for (uint32_t key_id = 0; key_id < path->NumControlPoints(curve_id); ++ key_id)
			{
				XMLNodePtr key_node = doc.AllocNode(XNT_Element, "key");

				key_node->AppendAttrib(doc.AllocAttribFloat("frame", path->FrameID(curve_id, key_id)));

				{
					float3 const & eye_ctrl_pt = path->EyeControlPoint(curve_id, key_id);
					std::string eye_str = boost::lexical_cast<std::string>(eye_ctrl_pt.x())
						+ ' ' + boost::lexical_cast<std::string>(eye_ctrl_pt.y())
						+ ' ' + boost::lexical_cast<std::string>(eye_ctrl_pt.z());
					key_node->AppendAttrib(doc.AllocAttribString("eye", eye_str));
				}
				{
					float3 const & target_ctrl_pt = path->TargetControlPoint(curve_id, key_id);
					std::string target_str = boost::lexical_cast<std::string>(target_ctrl_pt.x())
						+ ' ' + boost::lexical_cast<std::string>(target_ctrl_pt.y())
						+ ' ' + boost::lexical_cast<std::string>(target_ctrl_pt.z());
					key_node->AppendAttrib(doc.AllocAttribString("target", target_str));
				}
				{
					float3 const & up_ctrl_pt = path->EyeControlPoint(curve_id, key_id);
					std::string up_str = boost::lexical_cast<std::string>(up_ctrl_pt.x())
						+ ' ' + boost::lexical_cast<std::string>(up_ctrl_pt.y())
						+ ' ' + boost::lexical_cast<std::string>(up_ctrl_pt.z());
					key_node->AppendAttrib(doc.AllocAttribString("up", up_str));
				}

				key_node->AppendAttrib(doc.AllocAttribUInt("corner", path->Corner(curve_id, key_id)));

				curve_node->AppendNode(key_node);
			}
		}

		doc.Print(os);
	}
}
