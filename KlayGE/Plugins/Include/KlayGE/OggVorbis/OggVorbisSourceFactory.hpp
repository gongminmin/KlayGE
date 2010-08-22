// OggVorbisSourceFactory.hpp
// KlayGE Ogg vorbis audio data source header file
// Ver 3.11.0
// Copyright(C) Minmin Gong, 2010
// Homepage: http://www.klayge.org
//
// 3.11.0
// First release (2010.8.22)
//
// CHANGE LIST
/////////////////////////////////////////////////////////////////////////////////

#ifndef _OGGVORBISSOURCEFACTORY_HPP
#define _OGGVORBISSOURCEFACTORY_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#ifdef KLAYGE_HAS_DECLSPEC
	#ifdef KLAYGE_OV_ADS_SOURCE				// Build dll
		#define KLAYGE_OV_ADS_API __declspec(dllexport)
	#else									// Use dll
		#define KLAYGE_OV_ADS_API __declspec(dllimport)
	#endif
#else
	#define KLAYGE_OV_ADS_API
#endif // KLAYGE_HAS_DECLSPEC

extern "C"
{
	KLAYGE_OV_ADS_API void MakeAudioDataSourceFactory(KlayGE::AudioDataSourceFactoryPtr& ptr);
}

#endif			// _OGGVORBISSOURCEFACTORY_HPP
