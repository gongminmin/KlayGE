/**
 * @file JsonDom.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
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

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <KFL/CXX20/span.hpp>

namespace KlayGE
{
	class XMLDocument;

	enum class JsonValueType
	{
		Null,
		Bool,
		Int,
		UInt,
		Float,
		String,
		Array,
		Object,
	};

	class JsonValue
	{
	public:
		explicit JsonValue(JsonValueType type);
		JsonValue();
		explicit JsonValue(bool value);
		explicit JsonValue(int32_t value);
		explicit JsonValue(uint32_t value);
		explicit JsonValue(float value);
		explicit JsonValue(std::string_view value);
		explicit JsonValue(std::span<JsonValue> values);
		explicit JsonValue(std::span<std::pair<std::string, JsonValue>> values);
		JsonValue(JsonValue const& rhs);
		JsonValue(JsonValue&& rhs) noexcept;
		~JsonValue() noexcept;

		JsonValue& operator=(JsonValue const& rhs);
		JsonValue& operator=(JsonValue&& rhs) noexcept;

		JsonValueType Type() const noexcept;

		JsonValue const* Member(std::string_view name) const;
		JsonValue* Member(std::string_view name);

		void InsertAfterValue(JsonValue const& location, std::string_view name, JsonValue new_value);
		void AppendValue(std::string_view name, JsonValue new_value);
		void InsertAfterValue(JsonValue const& location, JsonValue new_value);
		void AppendValue(JsonValue new_value);

		void RemoveValue(JsonValue const& value);

		void ClearValues();

		bool ValueBool() const;
		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string_view ValueString() const;
		std::span<JsonValue const> ValueArray() const;
		std::span<std::pair<std::string, JsonValue> const> ValueObject() const;

		void Value(bool value);
		void Value(int32_t value);
		void Value(uint32_t value);
		void Value(float value);
		void Value(std::string_view value);
		void Value(std::span<JsonValue> values);
		void Value(std::span<std::pair<std::string, JsonValue>> values);
		void ValueIndex(uint32_t index, JsonValue value);
		void ValueIndex(uint32_t index, std::string_view name, JsonValue value);

	private:
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};

	JsonValue LoadJson(ResIdentifier& source);
	void SaveJson(JsonValue const& value, std::ostream& os);
	void SaveJson(XMLDocument const& dom, std::ostream& os);
} // namespace KlayGE
