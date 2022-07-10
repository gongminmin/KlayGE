// Viewport.cpp
// KlayGE Viewport class implement file
// Ver 4.0.0
// Copyright(C) Minmin Gong, 2011
// Homepage: http://www.klayge.org
//
// 4.0.0
// First release (2011.7.24)
//
// CHANGE LIST
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/Viewport.hpp>

namespace KlayGE
{
	Viewport::Viewport()
		: cameras_(1, MakeSharedPtr<KlayGE::Camera>())
	{
	}

	Viewport::Viewport(int left, int top, int width, int height)
		: left_(left), top_(top),
			width_(width), height_(height),
			cameras_(1, MakeSharedPtr<KlayGE::Camera>())
	{
	}

	uint32_t Viewport::NumCameras() const
	{
		return static_cast<uint32_t>(cameras_.size());
	}

	void Viewport::NumCameras(uint32_t num)
	{
		cameras_.resize(num);
	}

	void Viewport::Camera(CameraPtr const& camera)
	{
		return this->Camera(0, camera);
	}

	CameraPtr const& Viewport::Camera() const
	{
		return this->Camera(0);
	}

	void Viewport::Camera(uint32_t index, CameraPtr const& camera)
	{
		cameras_[index] = camera;
	}

	CameraPtr const& Viewport::Camera(uint32_t index) const
	{
		return cameras_[index];
	}
}
