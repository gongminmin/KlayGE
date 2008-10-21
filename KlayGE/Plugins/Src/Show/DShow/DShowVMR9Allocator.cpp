// DShowVRM9Allocator.cpp
// KlayGE DirectShow VRM9分配器类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d9.h>
#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/DShow/DShowVMR9Allocator.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3d9.lib")
#endif

namespace KlayGE
{
	DShowVMR9Allocator::DShowVMR9Allocator(HWND wnd)
					: wnd_(wnd), ref_count_(1),
						cur_surf_index_(0xFFFFFFFF)
	{
		d3d_ = MakeCOMPtr(Direct3DCreate9(D3D_SDK_VERSION));

		this->CreateDevice();
	}

	DShowVMR9Allocator::~DShowVMR9Allocator()
	{
		this->DeleteSurfaces();
	}

	void DShowVMR9Allocator::CreateDevice()
	{
		memset(&d3dpp_, 0, sizeof(d3dpp_));
		d3dpp_.Windowed = true;
		d3dpp_.hDeviceWindow = wnd_;
		d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		D3DCAPS9 caps;
		d3d_->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
		uint32_t vp_mode;
		if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
		{
			vp_mode = D3DCREATE_HARDWARE_VERTEXPROCESSING;
			if ((caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
			{
				vp_mode |= D3DCREATE_PUREDEVICE;
			}
		}
		else
		{
			vp_mode = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		IDirect3DDevice9* d3d_device;
		TIF(d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
								wnd_, vp_mode | D3DCREATE_MULTITHREADED,
								&d3dpp_, &d3d_device));
		d3d_device_ = MakeCOMPtr(d3d_device);
	}

	void DShowVMR9Allocator::DeleteSurfaces()
	{
		boost::mutex::scoped_lock lock(mutex_);

		// clear out the private texture
		cache_surf_.reset();

		for (size_t i = 0; i < surfaces_.size(); ++ i)
		{
			if (surfaces_[i] != NULL)
			{
				surfaces_[i]->Release();
				surfaces_[i] = NULL;
			}
		}
	}


	//IVMRSurfaceAllocator9
	HRESULT DShowVMR9Allocator::InitializeDevice(DWORD_PTR dwUserID,
				VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		if (NULL == lpNumBuffers)
		{
			return E_POINTER;
		}

		if (!vmr_surf_alloc_notify_)
		{
			return E_FAIL;
		}

		HRESULT hr = S_OK;

		// NOTE:
		// we need to make sure that we create textures because
		// surfaces can not be textured onto a primitive.
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

		this->DeleteSurfaces();
		surfaces_.resize(*lpNumBuffers);
		hr = vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]);

		// If we couldn't create a texture surface and
		// the format is not an alpha format,
		// then we probably cannot create a texture.
		// So what we need to do is create a private texture
		// and copy the decoded images onto it.
		if (FAILED(hr) && !(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget))
		{
			this->DeleteSurfaces();

			lpAllocInfo->dwFlags &= ~VMR9AllocFlag_TextureSurface;
			lpAllocInfo->dwFlags |= VMR9AllocFlag_OffscreenSurface;

			TIF(vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]));
		}


		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		try
		{
			present_tex_ = rf.MakeTexture2D(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, 1, EF_ARGB8, EAH_CPU_Write | EAH_GPU_Read, NULL);
		}
		catch (...)
		{
			present_tex_ = rf.MakeTexture2D(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, 1, EF_ABGR8, EAH_CPU_Write | EAH_GPU_Read, NULL);
		}

		IDirect3DSurface9* surf;
		TIF(d3d_device_->CreateOffscreenPlainSurface(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight,
			D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &surf, NULL));
		cache_surf_ = MakeCOMPtr(surf);

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::TerminateDevice(DWORD_PTR dwID)
	{
		if (dwID != USER_ID)
		{
			return S_OK;
		}

		this->DeleteSurfaces();
		return S_OK;
	}

	HRESULT DShowVMR9Allocator::GetSurface(DWORD_PTR dwUserID,
			DWORD SurfaceIndex, DWORD /*SurfaceFlags*/, IDirect3DSurface9** lplpSurface)
	{
		if (dwUserID != USER_ID)
		{
			*lplpSurface = NULL;
			return S_OK;
		}

		if (NULL == lplpSurface)
		{
			return E_POINTER;
		}

		if (SurfaceIndex >= surfaces_.size())
		{
			return E_FAIL;
		}

		boost::mutex::scoped_lock lock(mutex_);

		*lplpSurface = surfaces_[SurfaceIndex];
		(*lplpSurface)->AddRef();
		return S_OK;
	}

	HRESULT DShowVMR9Allocator::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
	{
		boost::mutex::scoped_lock lock(mutex_);

		vmr_surf_alloc_notify_ = MakeCOMPtr(lpIVMRSurfAllocNotify);
		vmr_surf_alloc_notify_->AddRef();

		HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		TIF(vmr_surf_alloc_notify_->SetD3DDevice(d3d_device_.get(), hMonitor));

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::StartPresenting(DWORD_PTR dwUserID)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		boost::mutex::scoped_lock lock(mutex_);

		if (!d3d_device_)
		{
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::StopPresenting(DWORD_PTR dwUserID)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		// parameter validation
		if (NULL == lpPresInfo)
		{
			return E_POINTER;
		}
		else
		{
			if (NULL == lpPresInfo->lpSurf)
			{
				return E_POINTER;
			}
		}

		boost::mutex::scoped_lock lock(mutex_);

		cur_surf_index_ = 0xFFFFFFFF;
		for (uint32_t i = 0; i < surfaces_.size(); ++ i)
		{
			if (surfaces_[i] == lpPresInfo->lpSurf)
			{
				cur_surf_index_ = i;
				break;
			}
		}

		if (D3DERR_DEVICENOTRESET == d3d_device_->TestCooperativeLevel())
		{
			cache_surf_.reset();

			for (size_t i = 0; i < surfaces_.size(); ++ i)
			{
				if (surfaces_[i] != NULL)
				{
					surfaces_[i]->Release();
					surfaces_[i] = NULL;
				}
			}

			this->CreateDevice();

			HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
			TIF(vmr_surf_alloc_notify_->ChangeD3DDevice(d3d_device_.get(), hMonitor));
		}

		return S_OK;
	}

	// IUnknown
	HRESULT DShowVMR9Allocator::QueryInterface(REFIID riid, void** ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;

		if (NULL == ppvObject)
		{
			hr = E_POINTER;
		}
		else
		{
			if (IID_IVMRSurfaceAllocator9 == riid)
			{
				*ppvObject = static_cast<IVMRSurfaceAllocator9*>(this);
				this->AddRef();
				hr = S_OK;
			}
			else
			{
				if (IID_IVMRImagePresenter9 == riid)
				{
					*ppvObject = static_cast<IVMRImagePresenter9*>(this);
					this->AddRef();
					hr = S_OK;
				}
				else
				{
					if (IID_IUnknown == riid)
					{
						*ppvObject = static_cast<IUnknown*>(static_cast<IVMRSurfaceAllocator9*>(this));
						this->AddRef();
						hr = S_OK;
					}
				}
			}
		}

		return hr;
	}

	ULONG DShowVMR9Allocator::AddRef()
	{
		++ ref_count_;
		return ref_count_.value();
	}

	ULONG DShowVMR9Allocator::Release()
	{
		-- ref_count_;
		if (0 == ref_count_.value())
		{
			delete this;
			return 0;
		}

		return ref_count_.value();
	}

	TexturePtr DShowVMR9Allocator::PresentTexture()
	{
		boost::mutex::scoped_lock lock(mutex_);

		if (FAILED(d3d_device_->TestCooperativeLevel()))
		{
			return TexturePtr();
		}

		if (cur_surf_index_ < surfaces_.size())
		{
			TIF(d3d_device_->GetRenderTargetData(surfaces_[cur_surf_index_], cache_surf_.get()));

			D3DLOCKED_RECT d3dlocked_rc;
			TIF(cache_surf_->LockRect(&d3dlocked_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

			uint32_t const width = present_tex_->Width(0);
			uint32_t const height = present_tex_->Height(0);

			uint8_t const * src = static_cast<uint8_t const *>(d3dlocked_rc.pBits);
			{
				Texture::Mapper mapper(*present_tex_, 0, TMA_Write_Only, 0, 0, width, height);
				uint8_t* dst = mapper.Pointer<uint8_t>();
				if (EF_ARGB8 == present_tex_->Format())
				{
					for (uint32_t y = 0; y < height; ++ y)
					{
						memcpy(dst, src, width * present_tex_->Bpp() / 8);
						dst += mapper.RowPitch();
						src += d3dlocked_rc.Pitch;
					}
				}
				else
				{
					BOOST_ASSERT(EF_ABGR8 == present_tex_->Format());

					for (uint32_t y = 0; y < height; ++ y)
					{
						for (uint32_t x = 0; x < width; ++ x)
						{
							dst[x * 4 + 0] = src[x * 4 + 2];
							dst[x * 4 + 1] = src[x * 4 + 1];
							dst[x * 4 + 2] = src[x * 4 + 0];
							dst[x * 4 + 3] = src[x * 4 + 3];
						}
						dst += mapper.RowPitch();
						src += d3dlocked_rc.Pitch;
					}
				}
			}

			TIF(cache_surf_->UnlockRect());
		}

		return present_tex_;
	}
}
