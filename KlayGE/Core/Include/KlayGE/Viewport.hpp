// Viewport.hpp
// KlayGE 渲染视口类 头文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://www.klayge.org
//
// 3.0.0
// camera改为指针 (2005.8.18)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _VIEWPORT_HPP
#define _VIEWPORT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API Viewport final : boost::noncopyable
	{
	public:
		Viewport();
		Viewport(int left, int top, int width, int height);

		void Left(int left)
		{
			left_ = left;
		}
		int Left() const
		{
			return left_;
		}
		void Top(int top)
		{
			top_ = top;
		}
		int Top() const
		{
			return top_;
		}
		void Width(int width)
		{
			width_ = width;
		}
		int Width() const
		{
			return width_;
		}
		void Height(int height)
		{
			height_ = height;
		}
		int Height() const
		{
			return height_;
		}

		uint32_t NumCameras() const;
		void NumCameras(uint32_t num);
		void Camera(CameraPtr const& camera);
		CameraPtr const& Camera() const;
		void Camera(uint32_t index, CameraPtr const& camera);
		CameraPtr const& Camera(uint32_t index) const;

	private:
		int left_;
		int top_;
		int width_;
		int height_;

		std::vector<CameraPtr> cameras_;
	};
}

#endif			// _VIEWPORT_HPP
