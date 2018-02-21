// DShowVRM9Allocator.cpp
// KlayGE DirectShow VRM9分配器类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/COMPtr.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include <cstring>
#include <boost/assert.hpp>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#pragma GCC diagnostic ignored "-Wunknown-pragmas" // Ignore unknown pragmas
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore D3DBUSIMPL_MODIFIER_NON_STANDARD definition
#endif
#include <d3d9.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#ifdef KLAYGE_COMPILER_GCC
#define _WIN32_WINNT_BACKUP _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#include <strmif.h>
#ifdef KLAYGE_COMPILER_GCC
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_BACKUP
#endif
#include <vmr9.h>

#include <KlayGE/DShow/DShowVMR9Allocator.hpp>

namespace KlayGE
{
	DShowVMR9Allocator::DShowVMR9Allocator(HWND wnd)
					: wnd_(wnd), ref_count_(1),
						cur_surf_index_(0xFFFFFFFF)
	{
		mod_d3d9_ = ::LoadLibraryEx(TEXT("d3d9.dll"), nullptr, 0);
		if (nullptr == mod_d3d9_)
		{
			::MessageBoxW(nullptr, L"Can't load d3d9.dll", L"Error", MB_OK);
		}

		if (mod_d3d9_ != nullptr)
		{
			DynamicDirect3DCreate9_ = reinterpret_cast<Direct3DCreate9Func>(::GetProcAddress(mod_d3d9_, "Direct3DCreate9"));
		}

		d3d_ = MakeCOMPtr(DynamicDirect3DCreate9_(D3D_SDK_VERSION));

		this->CreateDevice();
	}

	DShowVMR9Allocator::~DShowVMR9Allocator()
	{
		this->DeleteSurfaces();

		vmr_surf_alloc_notify_.reset();

		d3d_device_.reset();
		d3d_.reset();

		::FreeLibrary(mod_d3d9_);
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
		TIFHR(d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
			wnd_, vp_mode | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED,
			&d3dpp_, &d3d_device));
		d3d_device_ = MakeCOMPtr(d3d_device);
	}

	void DShowVMR9Allocator::DeleteSurfaces()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		// clear out the private texture
		cache_surf_.reset();

		for (size_t i = 0; i < surfaces_.size(); ++ i)
		{
			if (surfaces_[i] != nullptr)
			{
				surfaces_[i]->Release();
				surfaces_[i] = nullptr;
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

		if (nullptr == lpNumBuffers)
		{
			return E_POINTER;
		}

		if (!vmr_surf_alloc_notify_)
		{
			return E_FAIL;
		}

		// NOTE:
		// we need to make sure that we create textures because
		// surfaces can not be textured onto a primitive.
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

		this->DeleteSurfaces();
		surfaces_.resize(*lpNumBuffers);
		HRESULT hr = vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]);

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

			TIFHR(vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]));
		}


		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		ElementFormat fmt;
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8_SRGB))
			{
				fmt = EF_ABGR8_SRGB;
			}
			else
			{
				if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8_SRGB))
				{
					fmt = EF_ARGB8_SRGB;
				}
				else
				{
					if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
					{
						fmt = EF_ABGR8;
					}
					else
					{
						BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

						fmt = EF_ARGB8;
					}
				}
			}
		}
		else
		{
			if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
			{
				fmt = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

				fmt = EF_ARGB8;
			}
		}
		present_tex_ = rf.MakeTexture2D(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, 1, 1, fmt, 1, 0, EAH_CPU_Write | EAH_GPU_Read);

		IDirect3DSurface9* surf;
		TIFHR(d3d_device_->CreateOffscreenPlainSurface(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight,
			D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &surf, nullptr));
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
			*lplpSurface = nullptr;
			return S_OK;
		}

		if (nullptr == lplpSurface)
		{
			return E_POINTER;
		}

		if (SurfaceIndex >= surfaces_.size())
		{
			return E_FAIL;
		}

		std::lock_guard<std::mutex> lock(mutex_);

		*lplpSurface = surfaces_[SurfaceIndex];
		(*lplpSurface)->AddRef();
		return S_OK;
	}

	HRESULT DShowVMR9Allocator::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		vmr_surf_alloc_notify_ = MakeCOMPtr(lpIVMRSurfAllocNotify);
		vmr_surf_alloc_notify_->AddRef();

		HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		TIFHR(vmr_surf_alloc_notify_->SetD3DDevice(d3d_device_.get(), hMonitor));

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::StartPresenting(DWORD_PTR dwUserID)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		std::lock_guard<std::mutex> lock(mutex_);

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
		if (nullptr == lpPresInfo)
		{
			return E_POINTER;
		}
		else
		{
			if (nullptr == lpPresInfo->lpSurf)
			{
				return E_POINTER;
			}
		}

		std::lock_guard<std::mutex> lock(mutex_);

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
				if (surfaces_[i] != nullptr)
				{
					surfaces_[i]->Release();
					surfaces_[i] = nullptr;
				}
			}

			this->CreateDevice();

			HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
			TIFHR(vmr_surf_alloc_notify_->ChangeD3DDevice(d3d_device_.get(), hMonitor));
		}

		return S_OK;
	}

	// IUnknown
	HRESULT DShowVMR9Allocator::QueryInterface(REFIID riid, void** ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;

		if (nullptr == ppvObject)
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
		return ref_count_;
	}

	ULONG DShowVMR9Allocator::Release()
	{
		-- ref_count_;
		if (0 == ref_count_)
		{
			delete this;
			return 0;
		}

		return ref_count_;
	}

	TexturePtr DShowVMR9Allocator::PresentTexture()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (FAILED(d3d_device_->TestCooperativeLevel()))
		{
			return TexturePtr();
		}

		if (cur_surf_index_ < surfaces_.size())
		{
			TIFHR(d3d_device_->GetRenderTargetData(surfaces_[cur_surf_index_], cache_surf_.get()));

			D3DLOCKED_RECT d3dlocked_rc;
			TIFHR(cache_surf_->LockRect(&d3dlocked_rc, nullptr, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));

			uint32_t const width = present_tex_->Width(0);
			uint32_t const height = present_tex_->Height(0);
			uint32_t const texel_size = NumFormatBytes(present_tex_->Format());

			uint8_t const * src = static_cast<uint8_t const *>(d3dlocked_rc.pBits);
			{
				ElementFormat fmt = present_tex_->Format();
				Texture::Mapper mapper(*present_tex_, 0, 0, TMA_Write_Only, 0, 0, width, height);
				uint8_t* dst = mapper.Pointer<uint8_t>();
				if ((EF_ARGB8 == fmt) || (EF_ARGB8_SRGB == fmt))
				{
					for (uint32_t y = 0; y < height; ++ y)
					{
						std::memcpy(dst, src, width * texel_size);
						dst += mapper.RowPitch();
						src += d3dlocked_rc.Pitch;
					}
				}
				else
				{
					BOOST_ASSERT((EF_ABGR8 == fmt) || (EF_ABGR8_SRGB == fmt));

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

			TIFHR(cache_surf_->UnlockRect());
		}

		return present_tex_;
	}
}
