// CameraController.hpp
// KlayGE 摄像机控制器类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/Math.hpp>
#include <KlayGE/Camera.hpp>
#include <boost/timer.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 摄像机控制器
	//////////////////////////////////////////////////////////////////////////////////
	class CameraController
	{
	public:
		CameraController();
		virtual ~CameraController();

		void Scalers(float rotationScaler, float moveScaler);

		void AttachCamera(Camera& camera);
		void DetachCamera();

		virtual void Update();

	protected:
		float		rotationScaler_;	// Scaler for rotation
		float		moveScaler_;		// Scaler for movement

		Camera*		camera_;

		boost::timer timer_;
	};

	class FirstPersonCameraController : public CameraController
	{
	public:
		FirstPersonCameraController();

		void Update();

		void Move(float x, float y, float z);
		void Rotate(float yaw, float pitch, float roll);

	private:
		Matrix4		world_;				// World matrix of the camera (inverse of the view matrix)

		enum
		{
			TurnLeftRight,
			TurnUpDown,

			Forward,
			Backward,
			MoveLeft,
			MoveRight,
		};

		uint32_t action_map_id_;
	};
}

#endif		// _CAMERACONTROLLER_HPP
