// OGLRenderEffect.hpp
// KlayGE OpenGL渲染效果类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2004-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 使用Cg实现 (2005.7.30)
//
// 2.5.0
// 去掉了Clone (2005.4.16)
//
// 2.0.4
// 初次建立 (2004.4.4)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _OGLRENDEREFFECT_HPP
#define _OGLRENDEREFFECT_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_OpenGL
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/MapVector.hpp>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace KlayGE
{
	struct OGLRenderParameterDesc
	{
		RenderEffectParameterPtr param;

		CGparameter cg_handle;
	};

	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class OGLRenderEffect : public RenderEffect
	{
	public:

	private:
		RenderTechniquePtr MakeRenderTechnique();
	};
	typedef boost::shared_ptr<OGLRenderEffect> OGLRenderEffectPtr;

	class OGLRenderTechnique : public RenderTechnique
	{
	public:
		explicit OGLRenderTechnique(RenderEffect& effect)
			: RenderTechnique(effect)
		{
		}

	private:
		RenderPassPtr MakeRenderPass();

		void DoBegin(uint32_t flags);
		void DoEnd();
	};
	typedef boost::shared_ptr<OGLRenderTechnique> OGLRenderTechniquePtr;

	class OGLRenderPass : public RenderPass
	{
	public:
		explicit OGLRenderPass(RenderEffect& effect)
			: RenderPass(effect)
		{
		}
		~OGLRenderPass();

		uint8_t AttribIndex(VertexElementUsage usage, uint8_t usage_index);

	private:
		void DoLoad();

		void DoBegin();
		void DoEnd();

	private:
		boost::array<CGprogram, 2> shaders_;
		boost::array<CGprofile, 2> profiles_;

		boost::array<std::vector<OGLRenderParameterDesc>, 2> param_descs_;
		
		MapVector<std::pair<VertexElementUsage, uint8_t>, uint8_t> vertex_varyings_;

	private:
		CGprogram compile_shader(CGprofile profile, std::string const & name, std::string const & text);
		void create_vertex_shader(CGprofile profile, std::string const & name, std::string const & text);
		void create_pixel_shader(CGprofile profile, std::string const & name, std::string const & text);

		void shader(std::string& profile, std::string& name, std::string& func, std::string const & type) const;
	};
	typedef boost::shared_ptr<OGLRenderPass> OGLRenderPassPtr;
}

#endif		// _OGLRENDEREFFECT_HPP
