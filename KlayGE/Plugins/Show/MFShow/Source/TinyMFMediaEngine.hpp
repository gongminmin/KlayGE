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

#ifndef KLAYGE_PLUGINS_MF_SHOW_TINY_MF_MEDIA_ENGINE_HPP
#define KLAYGE_PLUGINS_MF_SHOW_TINY_MF_MEDIA_ENGINE_HPP

DEFINE_GUID(CLSID_MFMediaEngineClassFactory, 0xB44392DA, 0x499B, 0x446B, 0xA4, 0xCB, 0x0, 0x5F, 0xEA, 0xD0, 0xE6, 0xD5);

DEFINE_GUID(MF_MEDIA_ENGINE_CALLBACK, 0xC60381B8, 0x83A4, 0x41F8, 0xA3, 0xD0, 0xDE, 0x05, 0x07, 0x68, 0x49, 0xA9);
DEFINE_GUID(MF_MEDIA_ENGINE_DXGI_MANAGER, 0x065702DA, 0x1094, 0x486D, 0x86, 0x17, 0xEE, 0x7C, 0xC4, 0xEE, 0x46, 0x48);
DEFINE_GUID(MF_MEDIA_ENGINE_VIDEO_OUTPUT_FORMAT, 0x5066893C, 0x8CF9, 0x42BC, 0x8B, 0x8A, 0x47, 0x22, 0x12, 0xE5, 0x27, 0x26);

struct IMFMediaEngineSrcElements;
struct IMFMediaError;
struct IMFMediaTimeRange;

enum MF_MEDIA_ENGINE_CANPLAY
{
	MF_MEDIA_ENGINE_CANPLAY_NOT_SUPPORTED = 0,
	MF_MEDIA_ENGINE_CANPLAY_MAYBE = 1,
	MF_MEDIA_ENGINE_CANPLAY_PROBABLY = 2
};

enum MF_MEDIA_ENGINE_CREATEFLAGS
{
	MF_MEDIA_ENGINE_AUDIOONLY = 0x1,
	MF_MEDIA_ENGINE_WAITFORSTABLE_STATE = 0x2,
	MF_MEDIA_ENGINE_FORCEMUTE = 0x4,
	MF_MEDIA_ENGINE_REAL_TIME_MODE = 0x8,
	MF_MEDIA_ENGINE_DISABLE_LOCAL_PLUGINS = 0x10,
	MF_MEDIA_ENGINE_CREATEFLAGS_MASK = 0x1f
};

enum MF_MEDIA_ENGINE_ERR
{
	MF_MEDIA_ENGINE_ERR_NOERROR = 0,
	MF_MEDIA_ENGINE_ERR_ABORTED = 1,
	MF_MEDIA_ENGINE_ERR_NETWORK = 2,
	MF_MEDIA_ENGINE_ERR_DECODE = 3,
	MF_MEDIA_ENGINE_ERR_SRC_NOT_SUPPORTED = 4,
	MF_MEDIA_ENGINE_ERR_ENCRYPTED = 5
};

enum MF_MEDIA_ENGINE_EVENT
{
	MF_MEDIA_ENGINE_EVENT_LOADSTART = 1,
	MF_MEDIA_ENGINE_EVENT_PROGRESS = 2,
	MF_MEDIA_ENGINE_EVENT_SUSPEND = 3,
	MF_MEDIA_ENGINE_EVENT_ABORT = 4,
	MF_MEDIA_ENGINE_EVENT_ERROR = 5,
	MF_MEDIA_ENGINE_EVENT_EMPTIED = 6,
	MF_MEDIA_ENGINE_EVENT_STALLED = 7,
	MF_MEDIA_ENGINE_EVENT_PLAY = 8,
	MF_MEDIA_ENGINE_EVENT_PAUSE = 9,
	MF_MEDIA_ENGINE_EVENT_LOADEDMETADATA = 10,
	MF_MEDIA_ENGINE_EVENT_LOADEDDATA = 11,
	MF_MEDIA_ENGINE_EVENT_WAITING = 12,
	MF_MEDIA_ENGINE_EVENT_PLAYING = 13,
	MF_MEDIA_ENGINE_EVENT_CANPLAY = 14,
	MF_MEDIA_ENGINE_EVENT_CANPLAYTHROUGH = 15,
	MF_MEDIA_ENGINE_EVENT_SEEKING = 16,
	MF_MEDIA_ENGINE_EVENT_SEEKED = 17,
	MF_MEDIA_ENGINE_EVENT_TIMEUPDATE = 18,
	MF_MEDIA_ENGINE_EVENT_ENDED = 19,
	MF_MEDIA_ENGINE_EVENT_RATECHANGE = 20,
	MF_MEDIA_ENGINE_EVENT_DURATIONCHANGE = 21,
	MF_MEDIA_ENGINE_EVENT_VOLUMECHANGE = 22,
	MF_MEDIA_ENGINE_EVENT_FORMATCHANGE = 1000,
	MF_MEDIA_ENGINE_EVENT_PURGEQUEUEDEVENTS = 1001,
	MF_MEDIA_ENGINE_EVENT_TIMELINE_MARKER = 1002,
	MF_MEDIA_ENGINE_EVENT_BALANCECHANGE = 1003,
	MF_MEDIA_ENGINE_EVENT_DOWNLOADCOMPLETE = 1004,
	MF_MEDIA_ENGINE_EVENT_BUFFERINGSTARTED = 1005,
	MF_MEDIA_ENGINE_EVENT_BUFFERINGENDED = 1006,
	MF_MEDIA_ENGINE_EVENT_FRAMESTEPCOMPLETED = 1007,
	MF_MEDIA_ENGINE_EVENT_NOTIFYSTABLESTATE = 1008,
	MF_MEDIA_ENGINE_EVENT_FIRSTFRAMEREADY = 1009,
	MF_MEDIA_ENGINE_EVENT_TRACKSCHANGE = 1010,
	MF_MEDIA_ENGINE_EVENT_OPMINFO = 1011,
	MF_MEDIA_ENGINE_EVENT_RESOURCELOST = 1012,
	MF_MEDIA_ENGINE_EVENT_DELAYLOADEVENT_CHANGED = 1013,
	MF_MEDIA_ENGINE_EVENT_STREAMRENDERINGERROR = 1014,
	MF_MEDIA_ENGINE_EVENT_SUPPORTEDRATES_CHANGED = 1015
};

enum MF_MEDIA_ENGINE_PRELOAD
{
	MF_MEDIA_ENGINE_PRELOAD_MISSING = 0,
	MF_MEDIA_ENGINE_PRELOAD_EMPTY = 1,
	MF_MEDIA_ENGINE_PRELOAD_NONE = 2,
	MF_MEDIA_ENGINE_PRELOAD_METADATA = 3,
	MF_MEDIA_ENGINE_PRELOAD_AUTOMATIC = 4
};

struct MFVideoNormalizedRect
{
	float left;
	float top;
	float right;
	float bottom;
};

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
struct IMFMediaEngine : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE GetError(
		IMFMediaError** ppError) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetErrorCode(
		MF_MEDIA_ENGINE_ERR error) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetSourceElements(
		IMFMediaEngineSrcElements* pSrcElements) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetSource(
		BSTR pUrl) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentSource(
		BSTR* ppUrl) = 0;

	virtual USHORT STDMETHODCALLTYPE GetNetworkState() = 0;

	virtual MF_MEDIA_ENGINE_PRELOAD STDMETHODCALLTYPE GetPreload() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetPreload(
		MF_MEDIA_ENGINE_PRELOAD Preload) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetBuffered(
		IMFMediaTimeRange** ppBuffered) = 0;

	virtual HRESULT STDMETHODCALLTYPE Load() = 0;

	virtual HRESULT STDMETHODCALLTYPE CanPlayType(
		BSTR type,
		MF_MEDIA_ENGINE_CANPLAY* pAnswer) = 0;

	virtual USHORT STDMETHODCALLTYPE GetReadyState() = 0;

	virtual BOOL STDMETHODCALLTYPE IsSeeking() = 0;

	virtual double STDMETHODCALLTYPE GetCurrentTime() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetCurrentTime(
		double seekTime) = 0;

	virtual double STDMETHODCALLTYPE GetStartTime() = 0;

	virtual double STDMETHODCALLTYPE GetDuration() = 0;

	virtual BOOL STDMETHODCALLTYPE IsPaused() = 0;

	virtual double STDMETHODCALLTYPE GetDefaultPlaybackRate() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetDefaultPlaybackRate(
		double Rate) = 0;

	virtual double STDMETHODCALLTYPE GetPlaybackRate() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetPlaybackRate(
		double Rate) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetPlayed(
		IMFMediaTimeRange** ppPlayed) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetSeekable(
		IMFMediaTimeRange** ppSeekable) = 0;

	virtual BOOL STDMETHODCALLTYPE IsEnded() = 0;

	virtual BOOL STDMETHODCALLTYPE GetAutoPlay() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetAutoPlay(
		BOOL AutoPlay) = 0;

	virtual BOOL STDMETHODCALLTYPE GetLoop() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetLoop(
		BOOL Loop) = 0;

	virtual HRESULT STDMETHODCALLTYPE Play() = 0;

	virtual HRESULT STDMETHODCALLTYPE Pause() = 0;

	virtual BOOL STDMETHODCALLTYPE GetMuted() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetMuted(
		BOOL Muted) = 0;

	virtual double STDMETHODCALLTYPE GetVolume() = 0;

	virtual HRESULT STDMETHODCALLTYPE SetVolume(
		double Volume) = 0;

	virtual BOOL STDMETHODCALLTYPE HasVideo() = 0;

	virtual BOOL STDMETHODCALLTYPE HasAudio() = 0;

	virtual HRESULT STDMETHODCALLTYPE GetNativeVideoSize(
		DWORD* cx,
		DWORD* cy) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetVideoAspectRatio(
		DWORD* cx,
		DWORD* cy) = 0;

	virtual HRESULT STDMETHODCALLTYPE Shutdown() = 0;

	virtual HRESULT STDMETHODCALLTYPE TransferVideoFrame(
		IUnknown* pDstSurf,
		const MFVideoNormalizedRect* pSrc,
		const RECT* pDst,
		const MFARGB* pBorderClr) = 0;

	virtual HRESULT STDMETHODCALLTYPE OnVideoStreamTick(
		LONGLONG* pPts) = 0;
};

struct IMFMediaEngineClassFactory : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateInstance(
		DWORD dwFlags,
		IMFAttributes* pAttr,
		IMFMediaEngine** ppPlayer) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateTimeRange(
		IMFMediaTimeRange** ppTimeRange) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateError(
		IMFMediaError** ppError) = 0;
};

struct IMFMediaEngineNotify : public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE EventNotify(
		DWORD event,
		DWORD_PTR param1,
		DWORD param2) = 0;
};

#endif		// KLAYGE_PLUGINS_MF_SHOW_TINY_MF_MEDIA_ENGINE_HPP
