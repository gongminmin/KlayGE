// PostProcess.hpp
// KlayGE 后期处理类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://klayge.sourceforge.net
//
// 3.5.0
// 增加了GammaCorrectionProcess (2007.1.22)
//
// 3.3.0
// 初次建立 (2006.6.23)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _POSTPROCESS_HPP
#define _POSTPROCESS_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderableHelper.hpp>

namespace KlayGE
{
	class PostProcess : public RenderableHelper
	{
	public:
		explicit PostProcess(RenderTechniquePtr tech);
		virtual ~PostProcess()
		{
		}

		virtual void Source(TexturePtr const & tex, bool flipping);
		virtual void Destinate(RenderTargetPtr const & rt);

		virtual void Apply();

		virtual void OnRenderBegin();

	protected:
		TexturePtr src_texture_;
		bool flipping_;

		RenderTargetPtr render_target_;

		GraphicsBufferPtr pos_vb_;
	};

	class GammaCorrectionProcess : public PostProcess
	{
	public:
		explicit GammaCorrectionProcess();

		void Gamma(float gamma);
		void OnRenderBegin();

	private:
		float inv_gamma_;
	};
}

#endif		// _POSTPROCESS_HPP
