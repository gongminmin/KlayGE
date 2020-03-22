/**
 * @file NullAudioEngine.cpp
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

#include <KlayGE/NullAudio/NullAudio.hpp>

namespace KlayGE
{
	NullAudioEngine::NullAudioEngine()
	{
		this->SetListenerPos(float3(0, 0, 0));
		this->SetListenerVel(float3(0, 0, 0));
		this->SetListenerOri(float3(0, 0, 1), float3(0, 1, 0));
	}

	void NullAudioEngine::DoSuspend()
	{
	}

	void NullAudioEngine::DoResume()
	{
	}

	std::wstring const & NullAudioEngine::Name() const
	{
		static std::wstring const name(L"Null Audio Engine");
		return name;
	}

	float3 NullAudioEngine::GetListenerPos() const
	{
		return pos_;
	}

	void NullAudioEngine::SetListenerPos(float3 const & v)
	{
		pos_ = v;
	}

	float3 NullAudioEngine::GetListenerVel() const
	{
		return vel_;
	}

	void NullAudioEngine::SetListenerVel(float3 const & v)
	{
		vel_ = v;
	}

	void NullAudioEngine::GetListenerOri(float3& face, float3& up) const
	{
		face = face_;
		up = up_;
	}

	void NullAudioEngine::SetListenerOri(float3 const & face, float3 const & up)
	{
		face_ = face;
		up_ = up;
	}
}
