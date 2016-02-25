// RenderSettings.hpp
// KlayGE ��Ⱦ������ ʵ���ļ�
// Ver 3.10.0
// ��Ȩ����(C) ������, 2005-2010
// Homepage: http://www.klayge.org
//
// 3.10.0
// ������motion_frames (2010.2.22)
// ֧��Stereo (2010.3.20)
//
// 3.7.0
// ȥ����ConfirmDevice�Ĳ��� (2008.3.17)
//
// 2.8.0
// ������ConfirmDevice (2005.7.17)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSETTINGS_HPP
#define _RENDERSETTINGS_HPP

#pragma once

#include <vector>

#include <KlayGE/ElementFormat.hpp>

namespace KlayGE
{
	// ������Ⱦ���ڵ�����
	/////////////////////////////////////////////////////////////////////////////////
	struct RenderSettings
	{
		RenderSettings()
			: hide_win(false), full_screen(false),
				left(0), top(0),
				color_fmt(EF_ARGB8), depth_stencil_fmt(EF_D16),
				sample_count(1), sample_quality(0),
				motion_frames(0), hdr(false), fft_lens_effects(false), ppaa(false), gamma(false), color_grading(false),
				stereo_method(STM_None), stereo_separation(0)
		{
		}

		bool	hide_win;
		bool	full_screen;
		int		left;
		int		top;
		int		width;
		int		height;
		ElementFormat color_fmt;
		ElementFormat depth_stencil_fmt;
		uint32_t sample_count;
		uint32_t sample_quality;
		uint32_t sync_interval;
		uint32_t motion_frames;
		bool hdr;
		bool fft_lens_effects;
		bool ppaa;
		bool gamma;
		bool color_grading;

		StereoMethod stereo_method;
		float stereo_separation;

		std::vector<std::pair<std::string, std::string>> options;
	};
}

#endif			// _RENDERSETTINGS_HPP
