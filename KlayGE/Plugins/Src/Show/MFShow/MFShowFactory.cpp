/**
 * @file MFShowFactory.cpp
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
#include <KFL/Util.hpp>
#include <KlayGE/ShowFactory.hpp>

#include <KlayGE/MFShow/MFShow.hpp>

namespace KlayGE
{
	class MFShowFactory : public ShowFactory
	{
	public:
		std::wstring const & Name() const override
		{
			static std::wstring const name(L"DirectShow Show Factory");
			return name;
		}

	private:
		std::unique_ptr<ShowEngine> MakeShowEngine() override
		{
			return MakeUniquePtr<MFShowEngine>();
		}

		void DoSuspend() override
		{
		}
		void DoResume() override
		{
		}
	};
}

extern "C"
{
	KLAYGE_SYMBOL_EXPORT void MakeShowFactory(std::unique_ptr<KlayGE::ShowFactory>& ptr)
	{
		ptr = KlayGE::MakeUniquePtr<KlayGE::MFShowFactory>();
	}
}
