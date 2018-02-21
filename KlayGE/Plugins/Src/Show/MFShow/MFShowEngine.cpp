/**
 * @file MFShowEngine.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#define INITGUID
#include <KFL/ErrorHandling.hpp>
#include <KFL/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <cstring>
#include <mutex>

#include <boost/assert.hpp>

#include <mfapi.h>
#if (_WIN32_WINNT > _WIN32_WINNT_WIN7)
#include <mfmediaengine.h>
#else
#include "TinyMFMediaEngine.hpp"
#endif
#include <KlayGE/SALWrapper.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#endif
#include <dxgi1_6.h>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare" // Ignore comparison between int and uint
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-const-init" // Ignore const init (a Microsoft extension)
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value" // Ignore int enum
#pragma clang diagnostic ignored "-Wsign-compare" // Ignore comparison between int and uint
#endif
#include <d3d11_4.h>
#if defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif
#if defined(KLAYGE_COMPILER_GCC) || defined(KLAYGE_COMPILER_CLANGC2)
#undef __out
#endif

#include <KlayGE/MFShow/MFShow.hpp>

DEFINE_GUID(IID_IMFMediaEngineClassFactory, 0x4D645ACE, 0x26AA, 0x4688, 0x9B, 0xE1, 0xDF, 0x35, 0x16, 0x99, 0x0B, 0x93);
DEFINE_GUID(IID_IMFMediaEngineNotify, 0xFEE7C112, 0xE776, 0x42B5, 0x9B, 0xBF, 0x00, 0x48, 0x52, 0x4E, 0x2B, 0xD5);

namespace KlayGE
{
	class MediaEngineNotify : public IMFMediaEngineNotify
	{
	public:
		MediaEngineNotify()
		{
		}
		virtual ~MediaEngineNotify()
		{
		}

		STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
		{
			if (riid == IID_IMFMediaEngineNotify)
			{
				*ppv = static_cast<IMFMediaEngineNotify*>(this);
			}
			else
			{
				*ppv = nullptr;
				return E_NOINTERFACE;
			}

			this->AddRef();

			return S_OK;
		}

		STDMETHODIMP_(ULONG) AddRef()
		{
			return ::InterlockedIncrement(&ref_);
		}

		STDMETHODIMP_(ULONG) Release()
		{
			long ref = ::InterlockedDecrement(&ref_);
			if (ref == 0)
			{
				delete this;
			}
			return ref;
		}

		STDMETHODIMP EventNotify(DWORD me_event, DWORD_PTR param1, DWORD param2)
		{
			KFL_UNUSED(param2);

			if (me_event == MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE)
			{
				::SetEvent(reinterpret_cast<HANDLE>(param1));
			}
			else
			{
				cb_->OnMediaEngineEvent(me_event);
			}

			return S_OK;
		}

		void MediaEngineNotifyCallback(MFShowEngine* cb)
		{
			cb_ = cb;
		}

	private:
		long ref_ = 1;
		MFShowEngine* cb_ = nullptr;
	};

	MFShowEngine::MFShowEngine()
	{
#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		mod_dxgi_ = ::LoadLibraryEx(TEXT("dxgi.dll"), nullptr, 0);
		if (nullptr == mod_dxgi_)
		{
			::MessageBoxW(nullptr, L"Can't load dxgi.dll", L"Error", MB_OK);
		}
		mod_d3d11_ = ::LoadLibraryEx(TEXT("d3d11.dll"), nullptr, 0);
		if (nullptr == mod_d3d11_)
		{
			::MessageBoxW(nullptr, L"Can't load d3d11.dll", L"Error", MB_OK);
		}
		mod_mfplat_ = ::LoadLibraryEx(TEXT("mfplat.dll"), nullptr, 0);
		if (nullptr == mod_mfplat_)
		{
			::MessageBoxW(nullptr, L"Can't load mfplat.dll", L"Error", MB_OK);
		}

		if (mod_dxgi_ != nullptr)
		{
			DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(::GetProcAddress(mod_dxgi_, "CreateDXGIFactory1"));
		}
		if (mod_d3d11_ != nullptr)
		{
			DynamicD3D11CreateDevice_ = reinterpret_cast<D3D11CreateDeviceFunc>(::GetProcAddress(mod_d3d11_, "D3D11CreateDevice"));
		}
		if (mod_mfplat_ != nullptr)
		{
			DynamicMFStartup_ = reinterpret_cast<MFStartupFunc>(::GetProcAddress(mod_mfplat_, "MFStartup"));
			DynamicMFCreateDXGIDeviceManager_ = reinterpret_cast<MFCreateDXGIDeviceManagerFunc>(::GetProcAddress(mod_mfplat_,
				"MFCreateDXGIDeviceManager"));
			DynamicMFCreateAttributes_ = reinterpret_cast<MFCreateAttributesFunc>(::GetProcAddress(mod_mfplat_, "MFCreateAttributes"));
			DynamicMFShutdown_ = reinterpret_cast<MFShutdownFunc>(::GetProcAddress(mod_mfplat_, "MFShutdown"));
		}
#else
		DynamicCreateDXGIFactory1_ = ::CreateDXGIFactory1;
		DynamicD3D11CreateDevice_ = ::D3D11CreateDevice;

		DynamicMFStartup_ = ::MFStartup;
		DynamicMFCreateDXGIDeviceManager_ = ::MFCreateDXGIDeviceManager;
		DynamicMFCreateAttributes_ = ::MFCreateAttributes;
		DynamicMFShutdown_ = ::MFShutdown;
#endif

		if (SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
		{
			this->Init();
		}
	}

	MFShowEngine::~MFShowEngine()
	{
		this->Free();

		::CoUninitialize();

		DynamicCreateDXGIFactory1_ = nullptr;
		DynamicD3D11CreateDevice_ = nullptr;

		DynamicMFStartup_ = nullptr;
		DynamicMFCreateDXGIDeviceManager_ = nullptr;
		DynamicMFCreateAttributes_ = nullptr;
		DynamicMFShutdown_ = nullptr;

#ifdef KLAYGE_PLATFORM_WINDOWS_DESKTOP
		::FreeLibrary(mod_d3d11_);
		::FreeLibrary(mod_dxgi_);
		::FreeLibrary(mod_mfplat_);
#endif
	}

	void MFShowEngine::Init()
	{
		state_ = SS_Uninit;

		TIFHR(DynamicMFStartup_(MF_VERSION, MFSTARTUP_FULL));

		std::lock_guard<std::mutex> lock(mutex_);

		IDXGIFactory1* dxgi_factory;
		TIFHR(DynamicCreateDXGIFactory1_(IID_IDXGIFactory1, reinterpret_cast<void**>(&dxgi_factory)));
		dxgi_factory_ = MakeCOMPtr(dxgi_factory);
		dxgi_sub_ver_ = 1;

		IDXGIFactory2* gi_factory2;
		dxgi_factory->QueryInterface(IID_IDXGIFactory2, reinterpret_cast<void**>(&gi_factory2));
		if (gi_factory2 != nullptr)
		{
			dxgi_sub_ver_ = 2;

			IDXGIFactory4* gi_factory4;
			dxgi_factory->QueryInterface(IID_IDXGIFactory4, reinterpret_cast<void**>(&gi_factory4));
			if (gi_factory4 != nullptr)
			{
				dxgi_sub_ver_ = 4;
				gi_factory4->Release();
			}

			gi_factory2->Release();
		}

		UINT constexpr create_device_flags = 0;
		static UINT constexpr available_create_device_flags[] =
		{
#ifdef KLAYGE_DEBUG
			create_device_flags | D3D11_CREATE_DEVICE_DEBUG,
#endif
			create_device_flags
		};

		static D3D_DRIVER_TYPE constexpr dev_types[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP
		};

		static D3D_FEATURE_LEVEL constexpr all_feature_levels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0
		};

		ArrayRef<D3D_FEATURE_LEVEL> feature_levels;
		{
			uint32_t feature_level_start_index = 0;
			if (dxgi_sub_ver_ < 4)
			{
				feature_level_start_index = 2;

				if (dxgi_sub_ver_ < 2)
				{
					feature_level_start_index = 3;
				}
			}

			feature_levels = ArrayRef<D3D_FEATURE_LEVEL>(all_feature_levels).Slice(feature_level_start_index);
		}

		ID3D11Device* d3d_device = nullptr;
		ID3D11DeviceContext* d3d_imm_ctx = nullptr;
		for (auto dev_type : dev_types)
		{
			d3d_device = nullptr;
			d3d_imm_ctx = nullptr;

			HRESULT hr = E_FAIL;
			for (auto const & flags : available_create_device_flags)
			{
				ID3D11Device* this_device = nullptr;
				ID3D11DeviceContext* this_imm_ctx = nullptr;
				D3D_FEATURE_LEVEL this_out_feature_level;
				hr = DynamicD3D11CreateDevice_(nullptr, dev_type, nullptr,
					flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
					&feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION,
					&this_device, &this_out_feature_level, &this_imm_ctx);
				if (SUCCEEDED(hr))
				{
					d3d_device = this_device;
					d3d_imm_ctx = this_imm_ctx;
					break;
				}
			}
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
		
		Verify(d3d_device != nullptr);
		Verify(d3d_imm_ctx != nullptr);

		d3d_device_ = MakeCOMPtr(d3d_device);
		d3d_imm_ctx_ = MakeCOMPtr(d3d_imm_ctx);

		ID3D10Multithread* d3d10_multithread;
		TIFHR(d3d_device_->QueryInterface(IID_ID3D10Multithread, reinterpret_cast<void**>(&d3d10_multithread)));
		if (d3d10_multithread != nullptr)
		{
			d3d10_multithread->SetMultithreadProtected(true);
			d3d10_multithread->Release();
		}

		IDXGIDevice2* dxgi_device;
		TIFHR(d3d_device_->QueryInterface(IID_IDXGIDevice2, reinterpret_cast<void**>(&dxgi_device)));
		if (dxgi_device != nullptr)
		{
			TIFHR(dxgi_device->SetMaximumFrameLatency(1));
			dxgi_device->Release();
		}

		UINT reset_token;
		IMFDXGIDeviceManager* dxgi_dev_manager;
		TIFHR(DynamicMFCreateDXGIDeviceManager_(&reset_token, &dxgi_dev_manager));
		dxgi_dev_manager_ = MakeCOMPtr(dxgi_dev_manager);

		TIFHR(dxgi_dev_manager_->ResetDevice(d3d_device_.get(), reset_token));

		auto me_notify = MakeCOMPtr(new MediaEngineNotify());
		me_notify->MediaEngineNotifyCallback(this);

		IMFMediaEngineClassFactory* me_factory;
		TIFHR(::CoCreateInstance(CLSID_MFMediaEngineClassFactory, nullptr, CLSCTX_INPROC_SERVER,
			IID_IMFMediaEngineClassFactory, reinterpret_cast<void**>(&me_factory)));
		auto factory = MakeCOMPtr(me_factory);

		IMFAttributes* mf_attributes;
		TIFHR(DynamicMFCreateAttributes_(&mf_attributes, 1));
		auto attributes = MakeCOMPtr(mf_attributes);

		TIFHR(attributes->SetUnknown(MF_MEDIA_ENGINE_DXGI_MANAGER, dxgi_dev_manager_.get()));
		TIFHR(attributes->SetUnknown(MF_MEDIA_ENGINE_CALLBACK, me_notify.get()));
		TIFHR(attributes->SetUINT32(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, d3d_format_));

		IMFMediaEngine* media_engine;
		TIFHR(factory->CreateInstance(MF_MEDIA_ENGINE_WAITFORSTABLE_STATE, attributes.get(), &media_engine));
		media_engine_ = MakeCOMPtr(media_engine);

		auto& rf = Context::Instance().RenderFactoryInstance();
		auto& re = rf.RenderEngineInstance();
		auto const & caps = re.DeviceCaps();
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			if (caps.texture_format_support(EF_ABGR8_SRGB))
			{
				format_ = EF_ABGR8_SRGB;
			}
			else if (caps.texture_format_support(EF_ARGB8_SRGB))
			{
				format_ = EF_ARGB8_SRGB;
			}
			else if (caps.texture_format_support(EF_ABGR8))
			{
				format_ = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8));

				format_ = EF_ARGB8;
			}
		}
		else
		{
			if (caps.texture_format_support(EF_ABGR8))
			{
				format_ = EF_ABGR8;
			}
			else
			{
				BOOST_ASSERT(caps.texture_format_support(EF_ARGB8));

				format_ = EF_ARGB8;
			}
		}

		switch (format_)
		{
		case EF_ABGR8_SRGB:
		case EF_ABGR8:
			d3d_format_ = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;

		case EF_ARGB8_SRGB:
		case EF_ARGB8:
			d3d_format_ = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;

		default:
			KFL_UNREACHABLE("Unsupported format.");
		}
	}

	void MFShowEngine::Free()
	{
		this->Stop();

		this->StopTimer();
		play_thread_();

		{
			std::lock_guard<std::mutex> lock(mutex_);

			if (media_engine_)
			{
				media_engine_->Shutdown();
			}
		}

		dxgi_factory_.reset();
		d3d_device_.reset();
		d3d_imm_ctx_.reset();
		dxgi_output_.reset();
		d3d_present_tex_.reset();
		d3d_present_cpu_tex_.reset();
		dxgi_dev_manager_.reset();
		media_engine_.reset();

		DynamicMFShutdown_();
	}

	void MFShowEngine::StartTimer()
	{
		IDXGIAdapter* dxgi_adapter;
		TIFHR(dxgi_factory_->EnumAdapters(0, &dxgi_adapter));

		IDXGIOutput* dxgi_output;
		TIFHR(dxgi_adapter->EnumOutputs(0, &dxgi_output));
		dxgi_output_ = MakeCOMPtr(dxgi_output);

		dxgi_adapter->Release();

		stop_timer_ = false;

		play_thread_ = Context::Instance().ThreadPool()([this] { this->RealVSyncTimer(); });
	}

	void MFShowEngine::StopTimer()
	{
		stop_timer_ = true;
		state_ = SS_Stopped;
	}

	void MFShowEngine::OnMediaEngineEvent(DWORD me_event)
	{
		switch (me_event)
		{
		case MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA:
			eos_ = false;
			break;

		case MF_MEDIA_ENGINE_EVENT_CANPLAY:
			{
				std::lock_guard<std::mutex> lock(mutex_);

				DWORD cx, cy;
				TIFHR(media_engine_->GetNativeVideoSize(&cx, &cy));

				rc_target_.right = cx;
				rc_target_.bottom = cy;

				D3D11_TEXTURE2D_DESC tex_desc;
				tex_desc.Width = cx;
				tex_desc.Height = cy;
				tex_desc.MipLevels = 1;
				tex_desc.ArraySize = 1;
				tex_desc.Format = d3d_format_;
				tex_desc.SampleDesc.Count = 1;
				tex_desc.SampleDesc.Quality = 0;
				tex_desc.Usage = D3D11_USAGE_DEFAULT;
				tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET;
				tex_desc.CPUAccessFlags = 0;
				tex_desc.MiscFlags = 0;

				ID3D11Texture2D* d3d_tex;
				TIFHR(d3d_device_->CreateTexture2D(&tex_desc, nullptr, &d3d_tex));
				d3d_present_tex_ = MakeCOMPtr(d3d_tex);

				tex_desc.Usage = D3D11_USAGE_STAGING;
				tex_desc.BindFlags = 0;
				tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

				TIFHR(d3d_device_->CreateTexture2D(&tex_desc, nullptr, &d3d_tex));
				d3d_present_cpu_tex_ = MakeCOMPtr(d3d_tex);

				auto& rf = Context::Instance().RenderFactoryInstance();
				present_tex_ = rf.MakeTexture2D(cx, cy, 1, 1, format_, 1, 0, EAH_CPU_Write | EAH_GPU_Read);

				this->StartPlaying();
			}
			break;

		case MF_MEDIA_ENGINE_EVENT_PLAY:
			state_ = SS_Playing;
			break;

		case MF_MEDIA_ENGINE_EVENT_PAUSE:
			state_ = SS_Paused;
			break;

		case MF_MEDIA_ENGINE_EVENT_ENDED:
			if (media_engine_->HasVideo())
			{
				StopTimer();
			}
			eos_ = true;
			break;

		case MF_MEDIA_ENGINE_EVENT_TIMEUPDATE:
			break;

		case MF_MEDIA_ENGINE_EVENT_ERROR:
			break;
		}
	}

	void MFShowEngine::RealVSyncTimer()
	{
		for (;;)
		{
			if (stop_timer_)
			{
				break;
			}

			if (SUCCEEDED(dxgi_output_->WaitForVBlank()))
			{
				std::lock_guard<std::mutex> lock(mutex_);

				if (media_engine_ != nullptr)
				{
					LONGLONG pts;
					if (media_engine_->OnVideoStreamTick(&pts) == S_OK)
					{
						MFARGB const bkg_clr = { 0, 0, 0, 0 };
						TIFHR(media_engine_->TransferVideoFrame(d3d_present_tex_.get(), nullptr, &rc_target_, &bkg_clr));

						d3d_imm_ctx_->CopyResource(d3d_present_cpu_tex_.get(), d3d_present_tex_.get());
						present_tex_dirty_ = true;
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	void MFShowEngine::DoSuspend()
	{
		IDXGIDevice3* dxgi_device = nullptr;
		d3d_device_->QueryInterface(IID_IDXGIDevice3, reinterpret_cast<void**>(&dxgi_device));
		if (dxgi_device != nullptr)
		{
			dxgi_device->Trim();
			dxgi_device->Release();
		}
	}

	void MFShowEngine::DoResume()
	{
	}

	void MFShowEngine::DoPlay()
	{
		if (media_engine_)
		{
			TIFHR(media_engine_->Play());
		}
	}

	void MFShowEngine::StartPlaying()
	{
		if (media_engine_)
		{
			if (media_engine_->HasVideo() && stop_timer_)
			{
				this->StartTimer();
			}

			if (eos_)
			{
				TIFHR(media_engine_->SetCurrentTime(0));
			}

			eos_ = false;
		}
	}

	void MFShowEngine::DoPause()
	{
		if (media_engine_)
		{
			TIFHR(media_engine_->Pause());
		}
	}

	void MFShowEngine::DoStop()
	{
		if (media_engine_)
		{
			TIFHR(media_engine_->Pause());
			TIFHR(media_engine_->SetCurrentTime(0));
		}
	}

	void MFShowEngine::Load(std::string const & file_name)
	{
		if (eos_)
		{
			this->StopTimer();
		}

		std::wstring url;
		Convert(url, file_name);
		media_engine_->SetSource(url.data());

		state_ = SS_Stopped;
	}

	bool MFShowEngine::IsComplete()
	{
		bool complete = true;
		if (media_engine_)
		{
			complete = (media_engine_->IsEnded() == TRUE);
		}

		return complete;
	}

	ShowState MFShowEngine::State(long msTimeout)
	{
		KFL_UNUSED(msTimeout);

		state_ = SS_Unkown;
		if (media_engine_)
		{
			if (media_engine_->IsEnded())
			{
				state_ = SS_Stopped;
			}
			else if (media_engine_->IsPaused())
			{
				state_ = SS_Paused;
			}
			else
			{
				state_ = SS_Playing;
			}
		}

		return state_;
	}

	TexturePtr MFShowEngine::PresentTexture()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (d3d_present_cpu_tex_ && present_tex_dirty_)
		{
			D3D11_MAPPED_SUBRESOURCE mapped;
			TIFHR(d3d_imm_ctx_->Map(d3d_present_cpu_tex_.get(), 0, D3D11_MAP_READ, 0, &mapped));

			uint32_t const width = present_tex_->Width(0);
			uint32_t const height = present_tex_->Height(0);
			uint32_t const texel_size = NumFormatBytes(present_tex_->Format());

			uint8_t const * src = static_cast<uint8_t const *>(mapped.pData);
			{
				Texture::Mapper mapper(*present_tex_, 0, 0, TMA_Write_Only, 0, 0, width, height);
				uint8_t* dst = mapper.Pointer<uint8_t>();
				for (uint32_t y = 0; y < height; ++ y)
				{
					std::memcpy(dst, src, width * texel_size);
					dst += mapper.RowPitch();
					src += mapped.RowPitch;
				}
			}

			d3d_imm_ctx_->Unmap(d3d_present_cpu_tex_.get(), 0);

			present_tex_dirty_ = false;
		}

		return present_tex_;
	}
}
