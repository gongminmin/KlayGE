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

#ifndef KFL_JSON_DOM_HPP
#define KFL_JSON_DOM_HPP

#pragma once

#include <iosfwd>
#include <vector>

#include <boost/noncopyable.hpp>

namespace KlayGE
{
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

	class JsonDocument : boost::noncopyable
	{
	public:
		JsonValue* RootValue() const;
		void RootValue(std::unique_ptr<JsonValue> new_value);

		std::unique_ptr<JsonValue> AllocValue(JsonValueType type);
		std::unique_ptr<JsonValue> AllocValueNull();
		std::unique_ptr<JsonValue> AllocValueBool(bool value);
		std::unique_ptr<JsonValue> AllocValueInt(int32_t value);
		std::unique_ptr<JsonValue> AllocValueUInt(uint32_t value);
		std::unique_ptr<JsonValue> AllocValueFloat(float value);
		std::unique_ptr<JsonValue> AllocValueString(std::string_view value);
		std::unique_ptr<JsonValue> AllocValueArray(std::vector<std::unique_ptr<JsonValue>> values);
		std::unique_ptr<JsonValue> AllocValueObject(std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values);

	private:
		std::unique_ptr<JsonValue> root_;
	};

	class JsonValue : boost::noncopyable
	{
	public:
		virtual ~JsonValue() noexcept;

		virtual JsonValueType Type() const noexcept = 0;

		virtual std::unique_ptr<JsonValue> Clone() = 0;

		virtual JsonValue* Member(std::string_view name) const;

		virtual void InsertAfterValue(JsonValue const& location, std::string_view name, std::unique_ptr<JsonValue> new_value);
		virtual void AppendValue(std::string_view name, std::unique_ptr<JsonValue> new_value);
		virtual void InsertAfterValue(JsonValue const& location, std::unique_ptr<JsonValue> new_value);
		virtual void AppendValue(std::unique_ptr<JsonValue> new_value);

		virtual void RemoveValue(JsonValue const& value);

		virtual void ClearValues();

		virtual bool ValueBool() const;
		virtual int32_t ValueInt() const;
		virtual uint32_t ValueUInt() const;
		virtual float ValueFloat() const;
		virtual std::string_view ValueString() const;
		virtual std::vector<std::unique_ptr<JsonValue>> const& ValueArray() const;
		virtual std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> const& ValueObject() const;

		virtual void Value(bool value);
		virtual void Value(int32_t value);
		virtual void Value(uint32_t value);
		virtual void Value(float value);
		virtual void Value(std::string_view value);
		virtual void Value(std::vector<std::unique_ptr<JsonValue>> values);
		virtual void Value(std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values);
		virtual void ValueIndex(uint32_t index, std::unique_ptr<JsonValue> value);
		virtual void ValueIndex(uint32_t index, std::string_view name, std::unique_ptr<JsonValue> value);
	};

	std::unique_ptr<JsonDocument> LoadJson(ResIdentifier& source);
	void SaveJson(JsonDocument const& dom, std::ostream& os);
} // namespace KlayGE

#endif // KFL_JSON_DOM_HPP
