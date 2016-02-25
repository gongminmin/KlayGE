/**
 * @file ScriptFactory.cpp
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
#include <KlayGE/Script.hpp>

#include <string>

#include <KlayGE/ScriptFactory.hpp>

namespace KlayGE
{
	ScriptEngine& ScriptFactory::ScriptEngineInstance()
	{
		if (!se_)
		{
			se_ = this->MakeScriptEngine();
		}

		return *se_;
	}

	void ScriptFactory::Suspend()
	{
		if (se_)
		{
			se_->Suspend();
		}
		this->DoSuspend();
	}

	void ScriptFactory::Resume()
	{
		this->DoResume();
		if (se_)
		{
			se_->Resume();
		}
	}
}
