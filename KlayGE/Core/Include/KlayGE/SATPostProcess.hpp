// SATPostProcess.hpp
// KlayGE Summed-Area Table后期处理类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 初次建立 (2006.10.10)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _SATPOSTPROCESS_HPP
#define _SATPOSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>

#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class SATSeparableScanSweepPostProcess : public PostProcess
	{
	public:
		SATSeparableScanSweepPostProcess(RenderTechniquePtr tech, bool dir);

		void Length(int length);
		void Step(int step);

	private:
		int length_;
		bool dir_;
	};

	class SummedAreaTablePostProcess : public PostProcess
	{
	public:
		SummedAreaTablePostProcess();

		void Source(TexturePtr const & tex, bool flipping);
		void Apply();

		TexturePtr SATTexture();

	private:
		TexturePtr inter_tex_[2];
		FrameBufferPtr inter_fb_[2];
		bool index_;

		SATSeparableScanSweepPostProcess scan_x_up_;
		SATSeparableScanSweepPostProcess scan_y_up_;
		SATSeparableScanSweepPostProcess scan_x_down_;
		SATSeparableScanSweepPostProcess scan_y_down_;
	};
}

#endif		// _SATPOSTPROCESS_HPP
