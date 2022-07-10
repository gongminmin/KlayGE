// CameraController.hpp
// KlayGE 摄像机控制器类 头文件
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
// 2.4.0
// 初次建立 (2005.3.12)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _CAMERACONTROLLER_HPP
#define _CAMERACONTROLLER_HPP

#pragma once

#include <KFL/Math.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Input.hpp>

namespace KlayGE
{
	// 摄像机控制器
	//////////////////////////////////////////////////////////////////////////////////
	class KLAYGE_CORE_API CameraController : boost::noncopyable
	{
	public:
		CameraController();
		virtual ~CameraController() noexcept;

		void Scalers(float rotationScaler, float moveScaler);

		virtual void AttachCamera(Camera& camera);
		virtual void DetachCamera();

	protected:
		float		rotationScaler_;	// Scaler for rotation
		float		moveScaler_;		// Scaler for movement

		Camera*		camera_;
	};

	class KLAYGE_CORE_API FirstPersonCameraController final : public CameraController
	{
	public:
		explicit FirstPersonCameraController(bool use_input_engine = true);

		virtual void AttachCamera(Camera& camera);

		void RequiresLeftButtonDown(bool lbd);

		void Move(float x, float y, float z);
		void RotateRel(float yaw, float pitch, float roll);
		void RotateAbs(Quaternion const & quat);

	private:
		float2		rot_x_;
		float2		rot_y_;
		float2		rot_z_;
		Quaternion	inv_rot_;

		bool left_button_down_;

		enum
		{
			TurnLeftRight,
			TurnUpDown,
			Turn,
			RollLeft,
			RollRight,

			Forward,
			Backward,
			MoveLeft,
			MoveRight
		};

	private:
		void InputHandler(InputEngine const & sender, InputAction const & action);
	};

	class KLAYGE_CORE_API TrackballCameraController final : public CameraController
	{
	public:
		explicit TrackballCameraController(bool use_input_engine = true,
			uint32_t rotate_button = MB_Left, uint32_t zoom_button = MB_Right,
			uint32_t move_button = MB_Middle);

		virtual void AttachCamera(Camera& camera);

		void Move(float offset_x, float offset_y);
		void Rotate(float offset_x, float offset_y);
		void Zoom(float offset_x, float offset_y);

	private:
		bool reverse_target_;
		float3 target_;
		float3 right_;
		uint32_t move_button_;
		uint32_t rotate_button_;
		uint32_t zoom_button_;

		enum
		{
			Turn,
			ZoomInOut
		};

	private:
		void InputHandler(InputEngine const & sender, InputAction const & action);
	};

	class KLAYGE_CORE_API CameraPathController final : public CameraController
	{
	public:
		enum InterpolateType
		{
			IT_Linear,
			IT_CatmullRom,
			IT_BSpline,
			IT_Bezier
		};

	public:
		CameraPathController();

		uint32_t NumCurves() const;
		uint32_t AddCurve(InterpolateType type, uint32_t num_frames);
		void DelCurve(uint32_t curve_id);
		InterpolateType CurveType(uint32_t curve_id) const;
		uint32_t NumCurveFrames(uint32_t curve_id) const;
		uint32_t NumControlPoints(uint32_t curve_id) const;
		uint32_t AddControlPoint(uint32_t curve_id, float frame_id,
			float3 const & eye_ctrl_pt, float3 const & target_ctrl_pt,
			float3 const & up_ctrl_pt, bool corner);
		void DelControlPoint(uint32_t curve_id, uint32_t pt_id);
		float FrameID(uint32_t curve_id, uint32_t pt_id) const;
		float3 const & EyeControlPoint(uint32_t curve_id, uint32_t pt_id) const;
		float3 const & TargetControlPoint(uint32_t curve_id, uint32_t pt_id) const;
		float3 const & UpControlPoint(uint32_t curve_id, uint32_t pt_id) const;
		bool Corner(uint32_t curve_id, uint32_t pt_id) const;
		void FrameID(uint32_t curve_id, uint32_t pt_id, float frame_id);
		void EyeControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
		void TargetControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
		void UpControlPoint(uint32_t curve_id, uint32_t pt_id, float3 const & pt);
		void Corner(uint32_t curve_id, uint32_t pt_id, bool corner);

		uint32_t FrameRate() const;
		void FrameRate(uint32_t frame_rate);

		float Frame() const;
		void Frame(float frame);

		virtual void AttachCamera(Camera& camera);
		virtual void DetachCamera();

	private:
		void UpdateCameraFunc(Camera& camera, float app_time, float elapsed_time);
		void UpdateCamera();

	private:
		struct CameraCurve
		{
			InterpolateType type;
			uint32_t num_frames;

			std::vector<float> frame_ids;
			std::vector<float3> eye_ctrl_pts;
			std::vector<float3> target_ctrl_pts;
			std::vector<float3> up_ctrl_pts;
			std::vector<bool> corners;
		};

		std::vector<CameraCurve> curves_;

		float start_time_;
		float curr_frame_;
		uint32_t frame_rate_;

		Signal::Connection connection_;
	};

	KLAYGE_CORE_API CameraPathControllerPtr LoadCameraPath(ResIdentifier& res);
	KLAYGE_CORE_API void SaveCameraPath(std::ostream& os, CameraPathControllerPtr const & path);
}

#endif		// _CAMERACONTROLLER_HPP
