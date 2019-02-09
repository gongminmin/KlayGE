/**
 * @file AMDQuadBuffer.hpp
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

#ifndef _KLAYGE_AMDQUADBUFFER_HPP
#define _KLAYGE_AMDQUADBUFFER_HPP

#pragma once

enum AmdDxExtPrimitiveTopology
{
};

class IAmdDxExtInterface
{
public:
	virtual unsigned int AddRef(void) = 0;
	virtual unsigned int Release(void) = 0;

protected:
	// Basic constructor
	IAmdDxExtInterface()
	{
	}
	virtual ~IAmdDxExtInterface()
	{
	}
};

struct AmdDxExtVersion
{
	unsigned int majorVersion;
	unsigned int minorVersion;
};

class IAmdDxExt : public IAmdDxExtInterface
{
public:
	virtual HRESULT GetVersion(AmdDxExtVersion* pExtVer) = 0;
	virtual IAmdDxExtInterface* GetExtInterface(unsigned int iface) = 0;

	virtual HRESULT IaSetPrimitiveTopology(unsigned int topology) = 0;
	virtual HRESULT IaGetPrimitiveTopology(AmdDxExtPrimitiveTopology* pExtTopology) = 0;
	virtual HRESULT SetSingleSampleRead(ID3D10Resource* pResource, BOOL singleSample) = 0;
	virtual HRESULT SetSingleSampleRead11(ID3D11Resource* pResource, BOOL singleSample) = 0;

protected:
	IAmdDxExt()
	{
	}
	virtual ~IAmdDxExt()
	{
	}
};

unsigned int const AmdDxExtQuadBufferStereoID = 2;

class IAmdDxExtQuadBufferStereo : public IAmdDxExtInterface
{
public:
	virtual HRESULT EnableQuadBufferStereo(BOOL enable) = 0;
	virtual HRESULT GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags,
		UINT* pNumModes, DXGI_MODE_DESC* pDesc) = 0;
	virtual UINT GetLineOffset(IDXGISwapChain* pSwapChain) = 0;
};

typedef HRESULT (WINAPI *PFNAmdDxExtCreate11)(ID3D11Device* pDevice, IAmdDxExt** ppExt);

typedef std::shared_ptr<IAmdDxExtQuadBufferStereo> IAmdDxExtQuadBufferStereoPtr;

#endif		// _KLAYGE_AMDQUADBUFFER_HPP
