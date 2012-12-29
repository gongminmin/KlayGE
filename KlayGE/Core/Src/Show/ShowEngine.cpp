// ShowEngine.cpp
// KlayGE 播放引擎 实现文件
// Ver 1.2.8.11
// 版权所有(C) 龚敏敏, 2001--2002
// Homepage: http://www.klayge.org
//
// 1.2.8.10
// 用string代替字符串指针 (2002.10.27)
//
// 1.2.8.11
// 改用UNICODE核心 (2002.11.7)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/Texture.hpp>

#include <KlayGE/Show.hpp>

namespace KlayGE
{
	class NullShowEngine : public ShowEngine
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring name(L"Null Show Engine");
			return name;
		}

		bool IsComplete()
		{
			return true;
		}

		void Load(std::string const & /*fileName*/)
		{
		}

		TexturePtr PresentTexture()
		{
			return Texture::NullObject();
		}

		ShowState State(long /*timeout*/)
		{
			return SS_Stopped;
		}

	private:
		void DoPlay()
		{
		}

		void DoStop()
		{
		}

		void DoPause()
		{
		}
	};

	ShowEngine::~ShowEngine()
	{
	}

	// 返回空对象
	//////////////////////////////////////////////////////////////////////////////////
	ShowEnginePtr ShowEngine::NullObject()
	{
		static ShowEnginePtr obj = MakeSharedPtr<NullShowEngine>();
		return obj;
	}

	// 可以播放
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPlay() const
	{
		return (SS_Stopped == this->state_) || (SS_Paused == this->state_);
	}

	// 可以停止
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanStop() const
	{
		return (SS_Playing == this->state_) || (SS_Paused == this->state_);
	}

	// 可以暂停
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::CanPause() const
	{
		return (SS_Playing == this->state_) || (SS_Paused == this->state_);
	}

	// 初始化完毕
	/////////////////////////////////////////////////////////////////////////////////
	bool ShowEngine::IsInitialized() const
	{
		return this->state_ != SS_Uninit;
	}

	// 播放
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Play()
	{
		if (this->CanPlay())
		{
			this->DoPlay();

			state_ = SS_Playing;
		}
	}

	// 暂停播放
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Pause()
	{
		if (this->CanPause())
		{
			this->DoPause();

			state_ = SS_Paused;
		}
	}

	// 停止播放
	/////////////////////////////////////////////////////////////////////////////////
	void ShowEngine::Stop()
	{
		if (this->CanStop())
		{
			this->DoStop();

			state_ = SS_Stopped;
		}
	}
}
