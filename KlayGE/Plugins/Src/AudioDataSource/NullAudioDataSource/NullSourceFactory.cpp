/**
 * @file NullSourceFactory.cpp
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

#include <KlayGE/NullAudioDataSource/NullSource.hpp>

namespace KlayGE
{
	class NullAudioDataSourceFactory : public AudioDataSourceFactory
	{
	public:
		std::wstring const & Name() const
		{
			static std::wstring const name(L"Null Audio Data Source Factory");
			return name;
		}

	private:
		AudioDataSourcePtr MakeAudioDataSource()
		{
			return MakeSharedPtr<NullSource>();
		}

		virtual void DoSuspend() override
		{
			// TODO
		}
		virtual void DoResume() override
		{
			// TODO
		}
	};
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeAudioDataSourceFactory(std::unique_ptr<KlayGE::AudioDataSourceFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::NullAudioDataSourceFactory>();
	}
}
