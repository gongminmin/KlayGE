// DShowVRM9Allocator.cpp
// KlayGE DirectShow VRM9�������� ʵ���ļ�
// Ver 3.4.0
// ��Ȩ����(C) ������, 2006
// Homepage: http://www.klayge.org
//
// 3.4.0
// ���ν��� (2006.7.15)
//
// �޸ļ�¼
/////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX20/span.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include <cstring>
#include <ostream>

#include <boost/assert.hpp>

#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#pragma GCC diagnostic ignored "-Wunknown-pragmas" // Ignore unknown pragmas
#elif defined(KLAYGE_COMPILER_CLANGCL)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcomment" // Ignore "/*" within block comment
#endif
#include <d3d9.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGCL)
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

DEFINE_UUID_OF(IUnknown);
DEFINE_UUID_OF(IVMRImagePresenter9);
DEFINE_UUID_OF(IVMRSurfaceAllocator9);

namespace KlayGE
{
	DShowVMR9Allocator::DShowVMR9Allocator(HWND wnd)
					: wnd_(wnd), ref_count_(1),
						cur_surf_index_(0xFFFFFFFF)
	{
		if (!mod_d3d9_.Load("d3d9.dll"))
		{
			LogError() << "COULDN'T load d3d9.dll" << std::endl;
			Verify(false);
		}

		DynamicDirect3DCreate9_ = reinterpret_cast<Direct3DCreate9Func>(mod_d3d9_.GetProcAddress("Direct3DCreate9"));

		d3d_.reset(DynamicDirect3DCreate9_(D3D_SDK_VERSION));

		this->CreateDevice();
	}

	DShowVMR9Allocator::~DShowVMR9Allocator()
	{
		this->DeleteSurfaces();

		vmr_surf_alloc_notify_.reset();

		d3d_device_.reset();
		d3d_.reset();

		mod_d3d9_.Free();
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

		TIFHR(d3d_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
			wnd_, vp_mode | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED,
			&d3dpp_, d3d_device_.put()));
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
		auto const & caps = rf.RenderEngineInstance().DeviceCaps();
		static ElementFormat constexpr backup_fmts[] = { EF_ABGR8_SRGB, EF_ARGB8_SRGB, EF_ABGR8, EF_ARGB8 };
		std::span<ElementFormat const> fmt_options = backup_fmts;
		if (!Context::Instance().Config().graphics_cfg.gamma)
		{
			fmt_options = fmt_options.subspan(2);
		}
		auto const fmt = caps.BestMatchTextureFormat(fmt_options);
		BOOST_ASSERT(fmt != EF_Unknown);
		present_tex_ = rf.MakeTexture2D(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, 1, 1, fmt, 1, 0, EAH_CPU_Write | EAH_GPU_Read);

		TIFHR(d3d_device_->CreateOffscreenPlainSurface(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight,
			D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, cache_surf_.put(), nullptr));

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

		vmr_surf_alloc_notify_.reset(lpIVMRSurfAllocNotify);

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
			if (UuidOf<IVMRSurfaceAllocator9>() == reinterpret_cast<Uuid const&>(riid))
			{
				*ppvObject = static_cast<IVMRSurfaceAllocator9*>(this);
				this->AddRef();
				hr = S_OK;
			}
			else
			{
				if (UuidOf<IVMRImagePresenter9>() == reinterpret_cast<Uuid const&>(riid))
				{
					*ppvObject = static_cast<IVMRImagePresenter9*>(this);
					this->AddRef();
					hr = S_OK;
				}
				else
				{
					if (UuidOf<IUnknown>() == reinterpret_cast<Uuid const&>(riid))
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
