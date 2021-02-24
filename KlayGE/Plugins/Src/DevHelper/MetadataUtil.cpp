/**
 * @file MetadataUtil.cpp
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

#include <KFL/ErrorHandling.hpp>

#include "MetadataUtil.hpp"

namespace KlayGE
{
	float GetFloat(rapidjson::Value const& value)
	{
		if (value.IsFloat())
		{
			return value.GetFloat();
		}
		else if (value.IsDouble())
		{
			return static_cast<float>(value.GetDouble());
		}
		else if (value.IsInt())
		{
			return static_cast<float>(value.GetInt());
		}
		else if (value.IsUint())
		{
			return static_cast<float>(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return static_cast<float>(value.GetInt64());
		}
		else if (value.IsUint64())
		{
			return static_cast<float>(value.GetUint64());
		}
		else
		{
			KFL_UNREACHABLE("Invalid value type.");
		}
	}

	int GetInt(rapidjson::Value const& value)
	{
		if (value.IsInt())
		{
			return value.GetInt();
		}
		else if (value.IsUint())
		{
			return static_cast<int>(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return static_cast<int>(value.GetInt64());
		}
		else if (value.IsUint64())
		{
			return static_cast<int>(value.GetUint64());
		}
		else
		{
			KFL_UNREACHABLE("Invalid value type.");
		}
	}
} // namespace KlayGE
