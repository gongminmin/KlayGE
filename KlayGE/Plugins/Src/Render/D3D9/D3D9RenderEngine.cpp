// D3D9RenderEngine.cpp
// KlayGE D3D9渲染引擎类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
//
// 2.0.3
// 优化了Render (2004.2.22)
//
// 2.0.1
// 重构了Render (2003.10.10)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>

#include <KlayGE/Light.hpp>
#include <KlayGE/Material.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9VertexStream.hpp>
#include <KlayGE/D3D9/D3D9IndexStream.hpp>

#include <cassert>
#include <algorithm>
#include <cstring>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")

namespace KlayGE
{
	// 从KlayGE的Matrix4转换到D3DMATRIX
	/////////////////////////////////////////////////////////////////////////////////
	D3DMATRIX Convert(Matrix4 const & mat)
	{
		D3DMATRIX d3dMat;
		std::copy(mat.begin(), mat.end(), &d3dMat._11);

		return d3dMat;
	}

	// 从D3DMATRIX转换到KlayGE的Matrix4
	/////////////////////////////////////////////////////////////////////////////////
	Matrix4 Convert(D3DMATRIX const & mat)
	{
		return Matrix4(&mat.m[0][0]);
	}

	// 从RenderEngine::CompareFunction转换到D3DCMPFUNC
	/////////////////////////////////////////////////////////////////////////////////
	D3DCMPFUNC Convert(RenderEngine::CompareFunction func)
	{
		D3DCMPFUNC ret;
		switch (func)
		{
		case RenderEngine::CF_AlwaysFail:
			ret = D3DCMP_NEVER;
			break;

		case RenderEngine::CF_AlwaysPass:
			ret = D3DCMP_ALWAYS;
			break;

		case RenderEngine::CF_Less:
			ret = D3DCMP_LESS;
			break;

		case RenderEngine::CF_LessEqual:
			ret = D3DCMP_LESSEQUAL;
			break;

		case RenderEngine::CF_Equal:
			ret = D3DCMP_EQUAL;
			break;

		case RenderEngine::CF_NotEqual:
			ret = D3DCMP_NOTEQUAL;
			break;

		case RenderEngine::CF_GreaterEqual:
			ret = D3DCMP_GREATEREQUAL;
			break;

		case RenderEngine::CF_Greater:
			ret = D3DCMP_GREATER;
			break;
		};

		return ret;
	}

	// 从RenderEngine::StencilOperation转换到D3DSTENCILOP
	/////////////////////////////////////////////////////////////////////////////////
	D3DSTENCILOP Convert(RenderEngine::StencilOperation op)
	{
		D3DSTENCILOP ret;

		switch (op)
		{
		case RenderEngine::SOP_Keep:
			ret = D3DSTENCILOP_KEEP;
			break;

		case RenderEngine::SOP_Zero:
			ret = D3DSTENCILOP_ZERO;
			break;

		case RenderEngine::SOP_Replace:
			ret = D3DSTENCILOP_REPLACE;
			break;

		case RenderEngine::SOP_Increment:
			ret = D3DSTENCILOP_INCRSAT;
			break;

		case RenderEngine::SOP_Decrement:
			ret = D3DSTENCILOP_DECRSAT;
			break;

		case RenderEngine::SOP_Invert:
			ret = D3DSTENCILOP_INVERT;
			break;
		};

		return ret;
	}

	// 从RenderEngine::TexFiltering转换到D3D的MagFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32 MagFilter(D3DCAPS9 const & caps, RenderEngine::TexFiltering tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((RenderEngine::TF_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC))
		{
			return D3DTEXF_ANISOTROPIC;
		}
		else
		{
			tf = RenderEngine::TF_Trilinear;
		}

		if ((RenderEngine::TF_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = RenderEngine::TF_Bilinear;
		}

		if ((RenderEngine::TF_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}

		return D3DTEXF_POINT;
	}

	// 从RenderEngine::TexFiltering转换到D3D的MinFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32 MinFilter(D3DCAPS9 const & caps, RenderEngine::TexFiltering tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((RenderEngine::TF_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC))
		{
			return D3DTEXF_ANISOTROPIC;
		}
		else
		{
			tf = RenderEngine::TF_Trilinear;
		}

		if ((RenderEngine::TF_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = RenderEngine::TF_Bilinear;
		}

		if ((RenderEngine::TF_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}

		return D3DTEXF_POINT;
	}

	// 从RenderEngine::TexFiltering转换到D3D的MipFilter标志
	/////////////////////////////////////////////////////////////////////////////////
	uint32 MipFilter(D3DCAPS9 const & caps, RenderEngine::TexFiltering tf)
	{
		// NOTE: Fall through if device doesn't support requested type
		if ((RenderEngine::TF_Anisotropic == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = RenderEngine::TF_Trilinear;
		}

		if ((RenderEngine::TF_Trilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_LINEAR;
		}
		else
		{
			tf = RenderEngine::TF_Bilinear;
		}

		if ((RenderEngine::TF_Bilinear == tf) && (caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR))
		{
			return D3DTEXF_POINT;
		}

		return D3DTEXF_NONE;
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::D3D9RenderEngine()
						: cullingMode_(RenderEngine::Cull_None),
							clearFlags_(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER)
	{
		// Create our Direct3D object
		d3d_ = MakeCOMPtr(Direct3DCreate9(D3D_SDK_VERSION));
		Verify(d3d_.get() != NULL);

		adapterList_.Enumerate(d3d_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::~D3D9RenderEngine()
	{
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D9RenderEngine::Name() const
	{
		static const std::wstring name(L"Direct3D9 Render System");
		return name;
	}

	// 获取D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	boost::shared_ptr<IDirect3D9> const & D3D9RenderEngine::D3D() const
	{
		return d3d_;
	}

	// 获取D3D Device接口
	/////////////////////////////////////////////////////////////////////////////////
	boost::shared_ptr<IDirect3DDevice9> const & D3D9RenderEngine::D3DDevice() const
	{
		return d3dDevice_;
	}

	// 获取D3D适配器列表
	/////////////////////////////////////////////////////////////////////////////////
	D3D9AdapterList const & D3D9RenderEngine::D3DAdapters() const
	{
		return adapterList_;
	}

	// 获取当前适配器
	/////////////////////////////////////////////////////////////////////////////////
	D3D9Adapter const & D3D9RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StartRendering()
	{
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		RenderTarget& renderTarget(**RenderEngine::ActiveRenderTarget());
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (renderTarget.Active())
			{
				gotMsg = (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
			}
			else
			{
				gotMsg = (::GetMessage(&msg, NULL, 0, 0) != 0);
			}

			if (gotMsg)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				// 在空余时间渲染帧 (没有等待的消息)
				if (renderTarget.Active())
				{
					renderTarget.Update();
				}
			}
		}
	}

	// 设置环境光
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::AmbientLight(Color const & col)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(col.r(), col.g(), col.b(), 1.0f)));
	}

	// 设置清除颜色
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::ClearColor(Color const & clr)
	{
		clearClr_ = D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), 1.0f);
	}

	// 设置光影类型
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::ShadingType(ShadeOptions so)
	{
		D3DSHADEMODE shadeMode;
		switch (so)
		{
		case SO_Flat:
			shadeMode = D3DSHADE_FLAT;
			break;

		case SO_Gouraud:
			shadeMode = D3DSHADE_GOURAUD;
			break;

		case SO_Phong:
			shadeMode = D3DSHADE_PHONG;
			break;
		}

		TIF(d3dDevice_->SetRenderState(D3DRS_SHADEMODE, shadeMode));
	}

	// 打开/关闭光源
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::EnableLighting(bool enabled)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_LIGHTING, enabled));
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr D3D9RenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		RenderWindowPtr win(new D3D9RenderWindow(d3d_, this->ActiveAdapter(), name,
			static_cast<D3D9RenderSettings const &>(settings)));

		IDirect3DDevice9* d3dDevice;
		win->CustomAttribute("D3DDEVICE", &d3dDevice);
		d3dDevice_ = MakeCOMPtr(d3dDevice);
		d3dDevice_->AddRef();

		this->ActiveRenderTarget(this->AddRenderTarget(win));

		this->DepthBufferDepthTest(settings.depthBuffer);
		this->DepthBufferDepthWrite(settings.depthBuffer);

		// get caps
		d3d_->GetDeviceCaps(this->ActiveAdapter().AdapterNo(), D3DDEVTYPE_HAL, &caps_);

		return win;
	}

	// 设置剪裁模式
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::CullingMode(CullMode mode)
	{
		uint32 d3dMode;

		cullingMode_ = mode;

		switch (mode)
		{
		case Cull_None:
			d3dMode = D3DCULL_NONE;
			break;

		case Cull_Clockwise:
			d3dMode = (*RenderEngine::ActiveRenderTarget())->RequiresTextureFlipping() ? D3DCULL_CCW : D3DCULL_CW;
			break;

		case Cull_AntiClockwise:
			d3dMode = (*RenderEngine::ActiveRenderTarget())->RequiresTextureFlipping() ? D3DCULL_CW : D3DCULL_CCW;
			break;
		}

		TIF(d3dDevice_->SetRenderState(D3DRS_CULLMODE, d3dMode));
	}

	// 设置光源
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetLight(uint32 index, Light const & lt)
	{
		D3DLIGHT9 d3dLight;
		std::memset(&d3dLight, 0, sizeof(d3dLight));

		switch (lt.lightType)
		{
		case Light::LT_Point:
			d3dLight.Type = D3DLIGHT_POINT;
			break;

		case Light::LT_Directional:
			d3dLight.Type = D3DLIGHT_DIRECTIONAL;
			break;

		case Light::LT_Spot:
			d3dLight.Type		= D3DLIGHT_SPOT;
			d3dLight.Falloff	= lt.spotFalloff;
			d3dLight.Theta		= lt.spotInner;
			d3dLight.Phi		= lt.spotOuter;
			break;
		}

		d3dLight.Diffuse	= D3DXCOLOR(lt.diffuse.r(), lt.diffuse.g(), lt.diffuse.b(), lt.diffuse.a());
		d3dLight.Specular	= D3DXCOLOR(lt.specular.r(), lt.specular.g(), lt.specular.b(), lt.specular.a());
		d3dLight.Ambient	= D3DXCOLOR(lt.ambient.r(), lt.ambient.g(), lt.ambient.b(), lt.ambient.a());

		if (lt.lightType != Light::LT_Directional)
		{
			d3dLight.Position = D3DXVECTOR3(lt.position.x(), lt.position.y(), lt.position.z());
		}
		if (lt.lightType != Light::LT_Point)
		{
			d3dLight.Direction = D3DXVECTOR3(lt.direction.x(), lt.direction.y(), lt.direction.z());
		}

		d3dLight.Range = lt.range;
		d3dLight.Attenuation0 = lt.attenuationConst;
		d3dLight.Attenuation1 = lt.attenuationLinear;
		d3dLight.Attenuation2 = lt.attenuationQuad;

		TIF(d3dDevice_->SetLight(index, &d3dLight));
	}

	// 打开/关闭某个光源
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::LightEnable(uint32 index, bool enabled)
	{
		TIF(d3dDevice_->LightEnable(index, enabled));
	}

	// 实现设置世界矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoWorldMatrix()
	{
		D3DMATRIX d3dmat(Convert(worldMat_));
		TIF(d3dDevice_->SetTransform(D3DTS_WORLD, &d3dmat));
	}

	// 实现设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoViewMatrix()
	{
		D3DMATRIX d3dMat(Convert(viewMat_));
		TIF(d3dDevice_->SetTransform(D3DTS_VIEW, &d3dMat));
	}

	// 实现设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoProjectionMatrix()
	{
		D3DMATRIX d3dMat(Convert(projMat_));

		if ((*activeRenderTarget_)->RequiresTextureFlipping())
		{
			d3dMat._22 = -d3dMat._22;
		}

		TIF(d3dDevice_->SetTransform(D3DTS_PROJECTION, &d3dMat));
	}

	// 设置材质
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetMaterial(Material const & m)
	{
		D3DMATERIAL9 material;

		material.Diffuse	= D3DXCOLOR(m.diffuse.r(), m.diffuse.g(), m.diffuse.b(), m.diffuse.a());
		material.Ambient	= D3DXCOLOR(m.ambient.r(), m.ambient.g(), m.ambient.b(), m.ambient.a());
		material.Specular	= D3DXCOLOR(m.specular.r(), m.specular.g(), m.specular.b(), m.specular.a());
		material.Emissive	= D3DXCOLOR(m.emissive.r(), m.emissive.g(), m.emissive.b(), m.emissive.a());
		material.Power		= m.shininess;

		TIF(d3dDevice_->SetMaterial(&material));
	}

	// 设置当前渲染目标，该渲染目标必须已经在列表中
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::ActiveRenderTarget(RenderTargetListIterator iter)
	{
		RenderEngine::ActiveRenderTarget(iter);

		IDirect3DSurface9* back;
		(*activeRenderTarget_)->CustomAttribute("DDBACKBUFFER", &back);
		TIF(d3dDevice_->SetRenderTarget(0, back));
		back->Release();

		IDirect3DSurface9* zBuffer;
		(*activeRenderTarget_)->CustomAttribute("D3DZBUFFER", &zBuffer);
		TIF(d3dDevice_->SetDepthStencilSurface(zBuffer));
		zBuffer->Release();

		this->CullingMode(cullingMode_);

		Viewport const & vp((*iter)->GetViewport());
		D3DVIEWPORT9 d3dvp = { vp.left, vp.top, vp.width, vp.height, 0, 1 };
		TIF(d3dDevice_->SetViewport(&d3dvp));
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::BeginFrame()
	{
		TIF(d3dDevice_->Clear(0, NULL, clearFlags_, clearClr_, 1, 0));

		TIF(d3dDevice_->BeginScene());
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::Render(RenderBuffer const & rb)
	{
		D3DPRIMITIVETYPE primType;
		uint32 primCount;
		uint32 const vertexCount(static_cast<uint32>(rb.UseIndices() ? rb.NumIndices() : rb.NumVertices()));
		switch (rb.Type())
		{
		case RenderBuffer::BT_PointList:
			primType = D3DPT_POINTLIST;
			primCount = vertexCount;
			break;

		case RenderBuffer::BT_LineList:
			primType = D3DPT_LINELIST;
			primCount = vertexCount / 2;
			break;

		case RenderBuffer::BT_LineStrip:
			primType = D3DPT_LINESTRIP;
			primCount = vertexCount - 1;
			break;

		case RenderBuffer::BT_TriangleList:
			primType = D3DPT_TRIANGLELIST;
			primCount = vertexCount / 3;
			break;

		case RenderBuffer::BT_TriangleStrip:
			primType = D3DPT_TRIANGLESTRIP;
			primCount = vertexCount - 2;
			break;

		case RenderBuffer::BT_TriangleFan:
			primType = D3DPT_TRIANGLEFAN;
			primCount = vertexCount - 2;
			break;
		}


		VertexDeclType shaderDecl;
		shaderDecl.reserve(currentDecl_.size());

		D3DVERTEXELEMENT9 element;
		element.Offset = 0;
		element.Method = D3DDECLMETHOD_DEFAULT;

		for (RenderBuffer::VertexStreamConstIterator iter = rb.VertexStreamBegin();
			iter != rb.VertexStreamEnd(); ++ iter)
		{
			VertexStream& stream(*(*iter));
			VertexStreamType type(stream.Type());

			element.Stream = static_cast<WORD>(shaderDecl.size());

			switch (type)
			{
			// Vertex xyzs
			case VST_Positions:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(stream.ElementsPerVertex());
				element.Usage		= D3DDECLUSAGE_POSITION;
				element.UsageIndex	= 0;
				break;

			// Normal
			case VST_Normals:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(stream.ElementsPerVertex());
				element.Usage		= D3DDECLUSAGE_NORMAL;
				element.UsageIndex	= 0;
				break;

			// Vertex colors
			case VST_Diffuses:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_COLOR;
				element.UsageIndex	= 0;
				break;

			// Vertex speculars
			case VST_Speculars:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_COLOR;
				element.UsageIndex	= 1;
				break;
			
			// Blend Weights
			case VST_BlendWeights:
				element.Type		= D3DDECLTYPE_FLOAT4;
				element.Usage		= D3DDECLUSAGE_BLENDWEIGHT;
				element.UsageIndex	= 0;
				break;

			// Blend Indices
			case VST_BlendIndices:
				element.Type		= D3DDECLTYPE_D3DCOLOR;
				element.Usage		= D3DDECLUSAGE_BLENDINDICES;
				element.UsageIndex	= 0;
				break;

			// Do texture coords
			case VST_TextureCoords0:
			case VST_TextureCoords1:
			case VST_TextureCoords2:
			case VST_TextureCoords3:
			case VST_TextureCoords4:
			case VST_TextureCoords5:
			case VST_TextureCoords6:
			case VST_TextureCoords7:
				element.Type		= D3DDECLTYPE_FLOAT1 - 1 + static_cast<BYTE>(stream.ElementsPerVertex());
				element.Usage		= D3DDECLUSAGE_TEXCOORD;
				element.UsageIndex	= type - VST_TextureCoords0;
				break;
			}

			shaderDecl.push_back(element);


			D3D9VertexStream& d3d9vs(static_cast<D3D9VertexStream&>(stream));
			TIF(d3dDevice_->SetStreamSource(element.Stream,
				d3d9vs.D3D9Buffer().get(), 0,
				static_cast<::UINT>(stream.sizeElement() * stream.ElementsPerVertex())));
		}

		{
			element.Stream		= 0xFF;
			element.Type		= D3DDECLTYPE_UNUSED;
			element.Usage		= 0;
			element.UsageIndex	= 0;
			shaderDecl.push_back(element);
		}

		// Clear any previous steam sources
		for (size_t i = shaderDecl.size() - 1; i < currentDecl_.size(); ++ i)
		{
			d3dDevice_->SetStreamSource(static_cast<uint32>(i), NULL, 0, 0);
		}


		if ((currentDecl_.size() != shaderDecl.size())
			|| std::memcmp(&currentDecl_[0], &shaderDecl[0],
								sizeof(shaderDecl[0]) * shaderDecl.size()) != 0)
		{
			currentDecl_ = shaderDecl;

			IDirect3DVertexDeclaration9* theVertexDecl;
			d3dDevice_->CreateVertexDeclaration(&currentDecl_[0], &theVertexDecl);
			currentVertexDecl_ = MakeCOMPtr(theVertexDecl);
		}

		d3dDevice_->SetVertexDeclaration(currentVertexDecl_.get());


		if (rb.UseIndices())
		{
			D3D9IndexStream& d3dis(static_cast<D3D9IndexStream&>(*rb.GetIndexStream()));
			d3dDevice_->SetIndices(d3dis.D3D9Buffer().get());

			for (uint32 i = 0; i < renderPasses_; ++ i)
			{
				renderEffect_->BeginPass(i);

				TIF(d3dDevice_->DrawIndexedPrimitive(primType, 0, 0,
					static_cast<UINT>(rb.NumVertices()), 0, primCount));

				renderEffect_->EndPass();
			}
		}
		else
		{
			for (uint32 i = 0; i < renderPasses_; ++ i)
			{
				renderEffect_->BeginPass(i);

				TIF(d3dDevice_->DrawPrimitive(primType, 0, primCount));

				renderEffect_->EndPass();
			}
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::EndFrame()
	{
		TIF(d3dDevice_->EndScene());
	}

	// 打开/关闭深度测试
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DepthBufferDepthTest(bool depthTest)
	{
		if (depthTest)
		{
			TIF(d3dDevice_->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE));
		}
		else
		{
			TIF(d3dDevice_->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));
		}
	}

	// 打开/关闭深度缓存
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DepthBufferDepthWrite(bool depthWrite)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, depthWrite));
	}

	// 设置深度比较函数
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DepthBufferFunction(CompareFunction depthFunction)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_ZFUNC, Convert(depthFunction)));
	}

	// 设置深度偏移
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DepthBias(uint16 bias)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS, bias));
	}

	// 设置雾化效果
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::Fog(FogMode mode, Color const & color,
		float expDensity, float linearStart, float linearEnd)
	{
		if (Fog_None == mode)
		{
			// just disable
			d3dDevice_->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
			d3dDevice_->SetRenderState(D3DRS_FOGENABLE, FALSE);
		}
		else
		{
			// Allow fog
			d3dDevice_->SetRenderState(D3DRS_FOGENABLE, TRUE);

			// Set pixel fog mode
			D3DFOGMODE d3dFogMode;
			switch (mode)
			{
			case Fog_Exp:
				d3dFogMode = D3DFOG_EXP;
				break;

			case Fog_Exp2:
				d3dFogMode = D3DFOG_EXP2;
				break;

			case Fog_Linear:
				d3dFogMode = D3DFOG_LINEAR;
				break;
			}

			d3dDevice_->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE);
			d3dDevice_->SetRenderState(D3DRS_FOGTABLEMODE, d3dFogMode);

			d3dDevice_->SetRenderState(D3DRS_FOGCOLOR, D3DCOLOR_COLORVALUE(color.r(), color.g(), color.b(), color.a()));
			d3dDevice_->SetRenderState(D3DRS_FOGSTART, *reinterpret_cast<uint32*>(&linearStart));
			d3dDevice_->SetRenderState(D3DRS_FOGEND, *reinterpret_cast<uint32*>(&linearEnd));
			d3dDevice_->SetRenderState(D3DRS_FOGDENSITY, *reinterpret_cast<uint32*>(&expDensity));
		}
	}

	// 设置纹理
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetTexture(uint32 stage, TexturePtr const & texture)
	{
		if (!texture)
		{
			TIF(d3dDevice_->SetTexture(stage, NULL));
		}
		else
		{
			TIF(d3dDevice_->SetTexture(stage,
				D3D9TexturePtr(static_cast<D3D9Texture*>(texture.get()))->D3DTexture().get()));
		}
	}

	// 设置纹理坐标集
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureCoordSet(uint32 stage, int index)
	{
		TIF(d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, index));
	}

	// 获取最大纹理阶段数
	/////////////////////////////////////////////////////////////////////////////////
	uint32 D3D9RenderEngine::MaxTextureStages()
	{
		return caps_.MaxSimultaneousTextures;
	}

	// 关闭某个纹理阶段
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DisableTextureStage(uint32 stage)
	{
		TIF(d3dDevice_->SetTexture(stage, NULL));
		TIF(d3dDevice_->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE));
	}

	// 计算纹理坐标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureCoordCalculation(uint32 stage, TexCoordCalcMethod m)
	{
		HRESULT hr = S_OK;
		D3DXMATRIX matTrans;
		switch (m)
		{
		case TCC_None:
			hr = d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			D3DXMatrixIdentity(&matTrans);
			hr = d3dDevice_->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + stage), &matTrans);
			break;

		case TCC_EnvironmentMap:
			// Sets the flags required for an environment map effect
			hr = d3dDevice_->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
			hr = d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
			hr = d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

			D3DXMatrixIdentity(&matTrans);
			matTrans._11 = 0.5f;
			matTrans._41 = 0.5f;
			matTrans._22 = -0.5f;
			matTrans._42 = 0.5f;
			hr = d3dDevice_->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + stage), &matTrans);
			break;

		case TCC_EnvironmentMapPlanar:
			// Sets the flags required for an environment map effect
			hr = d3dDevice_->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
			hr = d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
			hr = d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

			D3DXMatrixIdentity(&matTrans);
			matTrans._11 = 0.5f;
			matTrans._41 = 0.5f;
			matTrans._22 = -0.5f;
			matTrans._42 = 0.5f;
			hr = d3dDevice_->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + stage), &matTrans);
			break;
		}

		TIF(hr);
	}

	// 设置纹理寻址模式
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureAddressingMode(uint32 stage, TexAddressingMode tam)
	{
		D3DTEXTUREADDRESS d3dType;
		switch (tam)
		{
		case TAM_Wrap:
			d3dType = D3DTADDRESS_WRAP;
			break;

		case TAM_Mirror:
			d3dType = D3DTADDRESS_MIRROR;
			break;

		case TAM_Clamp:
			d3dType = D3DTADDRESS_CLAMP;
			break;
		}

		TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSU, d3dType));
		TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSV, d3dType));
	}

	// 设置纹理坐标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureMatrix(uint32 stage, Matrix4 const & mat)
	{
		if (Matrix4::Identity() == mat)
		{
			TIF(d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE));
		}
		else
		{
			// Set 2D input
			// TODO: deal with 3D coordinates when cubic environment mapping supported
			TIF(d3dDevice_->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));

			D3DMATRIX d3dMat(Convert(mat));
			TIF(d3dDevice_->SetTransform(static_cast<D3DTRANSFORMSTATETYPE>(D3DTS_TEXTURE0 + stage), &d3dMat));
		}
	}

	// 设置纹理过滤模式
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureFiltering(uint32 stage, TexFiltering texFiltering)
	{
		d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, MagFilter(caps_, texFiltering));
		d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, MinFilter(caps_, texFiltering));
		d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, MipFilter(caps_, texFiltering));
	}

	// 设置纹理异性过滤
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::TextureAnisotropy(uint32 stage, uint32 maxAnisotropy)
	{
		maxAnisotropy = std::min(maxAnisotropy, caps_.MaxAnisotropy);

		uint32 curAnisotropy;
		d3dDevice_->GetSamplerState(stage, D3DSAMP_MAXANISOTROPY, &curAnisotropy);
		if (maxAnisotropy != curAnisotropy)
		{
			d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, maxAnisotropy);
		}
	}

	// 打开模板缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilCheckEnabled(bool enabled)
	{
		if (enabled)
		{
			clearFlags_ |= D3DCLEAR_STENCIL;
		}
		else
		{
			clearFlags_ &= ~D3DCLEAR_STENCIL;
		}

		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, enabled));
	}

	// 硬件是否支持模板缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	bool D3D9RenderEngine::HasHardwareStencil()
	{
		IDirect3DSurface9* surf;
		D3DSURFACE_DESC surfDesc;
		d3dDevice_->GetDepthStencilSurface(&surf);
		surf->GetDesc(&surfDesc);

		if (D3DFMT_D24S8 == surfDesc.Format)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	// 设置模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16 D3D9RenderEngine::StencilBufferBitDepth()
	{
		return 8;
	}

	// 设置模板比较函数
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferFunction(CompareFunction func)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, Convert(func)));
	}

	// 设置模板缓冲区参考值
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferReferenceValue(uint32 refValue)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILREF, refValue));
	}

	// 设置模板缓冲区掩码
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferMask(uint32 mask)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILMASK, mask));
	}

	// 设置模板缓冲区测试失败后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferFailOperation(StencilOperation op)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILFAIL, Convert(op)));
	}

	// 设置模板缓冲区深度测试失败后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferDepthFailOperation(StencilOperation op)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL, Convert(op)));
	}

	// 设置模板缓冲区通过后的操作
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StencilBufferPassOperation(StencilOperation op)
	{
		TIF(d3dDevice_->SetRenderState(D3DRS_STENCILPASS, Convert(op)));
	}
}
