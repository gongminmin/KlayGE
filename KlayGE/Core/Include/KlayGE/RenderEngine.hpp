// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
// 保存了三个矩阵 (2004.4.7)
//
// 2.0.3
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.1
// 去掉了TexBlendMode (2003.10.16)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERENGINE_HPP
#define _RENDERENGINE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Math.hpp>

#include <vector>
#include <list>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	class RenderEngine
	{
	public:
		enum ShadeOptions
		{
			SO_Flat,
			SO_Gouraud,
			SO_Phong
		};

		enum CompareFunction
		{
			CF_AlwaysFail,
			CF_AlwaysPass,
			CF_Less,
			CF_LessEqual,
			CF_Equal,
			CF_NotEqual,
			CF_GreaterEqual,
			CF_Greater
		};

		enum CullMode
		{
			Cull_None,
			Cull_Clockwise,
			Cull_AntiClockwise,
		};

		enum FogMode
		{
			// No fog. Duh.
			Fog_None,
			// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
			Fog_Exp,
			// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
			Fog_Exp2,
			// Fog density increases linearly between the start and end distances
			Fog_Linear
		};

		// Enum describing the ways to generate texture coordinates
		enum TexCoordCalcMethod
		{
			// No calculated texture coordinates
			TCC_None,
			// Environment map based on vertex normals
			TCC_EnvironmentMap,
			// Environment map based on vertex positions
			TCC_EnvironmentMapPlanar,
			TCC_EnvironmentMapReflection,
			TCC_EnvironmentMapNormal
		};

		enum TexFiltering
		{
			TF_None,
			TF_Bilinear,
			TF_Trilinear,
			TF_Anisotropic
		};

		// Texture addressing modes - default is TAM_WRAP.
		enum TexAddressingMode
		{
			// Texture wraps at values over 1.0
			TAM_Wrap,
			// Texture mirrors (flips) at joins over 1.0
			TAM_Mirror,
			// Texture clamps at 1.0
			TAM_Clamp
		};


		// Type of texture blend mode.
		enum TexBlendType
		{
			TBT_Color,
			TBT_Alpha
		};

		// Enum describing the various actions which can be taken onthe stencil buffer
		enum StencilOperation
		{
			// Leave the stencil buffer unchanged
			SOP_Keep,
			// Set the stencil value to zero
			SOP_Zero,
			// Set the stencil value to the reference value
			SOP_Replace,
			// Increase the stencil value by 1, clamping at the maximum value
			SOP_Increment,
			// Decrease the stencil value by 1, clamping at 0
			SOP_Decrement,
			// Invert the bits of the stencil buffer
			SOP_Invert
		};

	public:
		typedef std::list<RenderTargetPtr> RenderTargetList;
		typedef RenderTargetList::iterator RenderTargetListIterator;

	public:
		RenderEngine();
		virtual ~RenderEngine();

		virtual std::wstring const & Name() const = 0;

		virtual void StartRendering() = 0;

		void SetRenderEffect(RenderEffectPtr const & effect);
		RenderEffectPtr GetRenderEffect() const;

		virtual void BeginFrame() = 0;
		virtual void Render(VertexBuffer const & vb) = 0;
		virtual void EndFrame() = 0;

		virtual void ClearColor(Color const & clr) = 0;

		virtual void ShadingType(ShadeOptions so) = 0;

		virtual void EnableLighting(bool enabled) = 0;
		virtual void AmbientLight(Color const & col) = 0;

		virtual RenderWindowPtr CreateRenderWindow(std::string const & name, RenderSettings const & settings) = 0;

		virtual void CullingMode(CullMode mode) = 0;
		virtual void SetMaterial(Material const & mat) = 0;

		virtual void SetLight(uint32_t index, Light const & lt) = 0;
		virtual void LightEnable(uint32_t index, bool enable) = 0;

		Matrix4 WorldMatrix() const;
		void WorldMatrix(Matrix4 const & mat);
		Matrix4 ViewMatrix();
		void ViewMatrix(Matrix4 const & mat);
		Matrix4 ProjectionMatrix();
		void ProjectionMatrix(Matrix4 const & mat);

		virtual void DepthBufferDepthTest(bool depthTest) = 0;
		virtual void DepthBufferDepthWrite(bool depthWrite) = 0;
		virtual void DepthBufferFunction(CompareFunction depthFunction) = 0;
		virtual void DepthBias(uint16_t bias) = 0;

		virtual void Fog(FogMode mode = Fog_None,
			Color const & color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1) = 0;

		// Attaches the passed render target to the render system.
		virtual RenderTargetListIterator AddRenderTarget(RenderTargetPtr const & target);
		// Renders a iterator to the beginning of the RenderTargetList
		virtual RenderTargetListIterator RenderTargetListBegin();
		// Renders a iterator to the one the pass the last one of the RenderTargetList
		virtual RenderTargetListIterator RenderTargetListEnd();
		// Detaches the render target with the passed name from the render system and returns a pointer to it.
		virtual RenderTargetPtr RemoveRenderTarget(RenderTargetListIterator iter);
		virtual void ActiveRenderTarget(RenderTargetListIterator iter);
		RenderTargetListIterator const & ActiveRenderTarget() const;

		virtual void SetTexture(uint32_t stage, TexturePtr const & texture) = 0;

		// Sets the texture coordinate set to use for a texture unit.
		virtual void TextureCoordSet(uint32_t stage, int index) = 0;

		// Returns the number of texture units the current output hardware supports.
		virtual uint32_t MaxTextureStages() = 0;
		// Turns off a texture unit.
		virtual void DisableTextureStage(uint32_t stage) = 0;

		// Sets a method for automatically calculating texture coordinates for a stage.
		virtual void TextureCoordCalculation(uint32_t stage, TexCoordCalcMethod m) = 0;
		// Sets the texture addressing mode for a texture unit.
		virtual void TextureAddressingMode(uint32_t stage, TexAddressingMode tam) = 0;
		// Sets the texture coordinate transformation matrix for a texture unit.
		virtual void TextureMatrix(uint32_t stage, Matrix4 const & mat) = 0;
		// Sets the texture filtering type for a texture unit.
		virtual void TextureFiltering(uint32_t stage, TexFiltering texFiltering) = 0;
		// Sets the maximal anisotropy for the specified texture unit.
		virtual void TextureAnisotropy(uint32_t stage, uint32_t maxAnisotropy) = 0;

		// Turns stencil buffer checking on or off. 
		virtual void StencilCheckEnabled(bool enabled) = 0;
		// Determines if this system supports hardware accelerated stencil buffer. 
		virtual bool HasHardwareStencil() = 0;

		// Determines the bit depth of the hardware accelerated stencil buffer, if supported.
		virtual uint16_t StencilBufferBitDepth() = 0;

		// Sets the stencil test function.
		virtual void StencilBufferFunction(CompareFunction func) = 0;
		// Sets the stencil buffer reference value.
		virtual void StencilBufferReferenceValue(uint32_t refValue) = 0;
		// Sets the stencil buffer mask value.
		virtual void StencilBufferMask(uint32_t mask) = 0;
		// Sets the action to perform if the stencil test fails.
		virtual void StencilBufferFailOperation(StencilOperation op) = 0;
		// Sets the action to perform if the stencil test passes, but the depth buffer test fails.
		virtual void StencilBufferDepthFailOperation(StencilOperation op) = 0;
		// Sets the action to perform if both the stencil test and the depth buffer test passes.
		virtual void StencilBufferPassOperation(StencilOperation op) = 0;

	protected:
		virtual void DoWorldMatrix() = 0;
		virtual void DoViewMatrix() = 0;
		virtual void DoProjectionMatrix() = 0;

	protected:
		RenderTargetList renderTargetList_;
		RenderTargetListIterator activeRenderTarget_;

		RenderEffectPtr renderEffect_;
		uint32_t renderPasses_;

		Matrix4 worldMat_, viewMat_, projMat_;
	};
}

#endif			// _RENDERENGINE_HPP
