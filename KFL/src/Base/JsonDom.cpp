/**
 * @file JsonDom.cpp
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

#include <KFL/KFL.hpp>

#include <KFL/ErrorHandling.hpp>
#include <KFL/ResIdentifier.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>

#include <variant>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <KFL/JsonDom.hpp>
#include <KFL/XMLDom.hpp>

using namespace KlayGE;

namespace
{
	JsonValue CreateJsonValue(rapidjson::Value const& value)
	{
		if (value.IsNull())
		{
			return JsonValue();
		}
		else if (value.IsFalse())
		{
			return JsonValue(false);
		}
		else if (value.IsTrue())
		{
			return JsonValue(true);
		}
		else if (value.IsBool())
		{
			return JsonValue(value.GetBool());
		}
		else if (value.IsInt())
		{
			return JsonValue(value.GetInt());
		}
		else if (value.IsUint())
		{
			return JsonValue(value.GetUint());
		}
		else if (value.IsInt64())
		{
			return JsonValue(static_cast<int32_t>(value.GetInt64()));
		}
		else if (value.IsUint64())
		{
			return JsonValue(static_cast<uint32_t>(value.GetUint64()));
		}
		else if (value.IsDouble())
		{
			return JsonValue(static_cast<float>(value.GetDouble()));
		}
		else if (value.IsString())
		{
			return JsonValue(std::string(value.GetString()));
		}
		else if (value.IsArray())
		{
			JsonValue ret(JsonValueType::Array);
			for (auto iter = value.Begin(); iter != value.End(); ++iter)
			{
				ret.AppendValue(CreateJsonValue(*iter));
			}
			return ret;
		}
		else if (value.IsObject())
		{
			JsonValue ret(JsonValueType::Object);
			for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter)
			{
				ret.AppendValue(iter->name.GetString(), CreateJsonValue(iter->value));
			}
			return ret;
		}
		else
		{
			KFL_UNREACHABLE("Invalid type");
		}
	}

	void SetRapidJsonValue(rapidjson::Document& rapidjson_doc, rapidjson::Value& rapidjson_value, JsonValue const& value)
	{
		auto& allocator = rapidjson_doc.GetAllocator();

		switch (value.Type())
		{
		case JsonValueType::Null:
			rapidjson_value.SetNull();
			break;

		case JsonValueType::Bool:
			rapidjson_value.SetBool(value.ValueBool());
			break;

		case JsonValueType::Int:
			rapidjson_value.SetInt(value.ValueInt());
			break;

		case JsonValueType::UInt:
			rapidjson_value.SetUint(value.ValueUInt());
			break;

		case JsonValueType::Float:
			rapidjson_value.SetDouble(value.ValueFloat());
			break;

		case JsonValueType::String:
			rapidjson_value.SetString(value.ValueString().data(), static_cast<uint32_t>(value.ValueString().size()), allocator);
			break;

		case JsonValueType::Array:
			rapidjson_value.SetArray();
			for (auto const& v : value.ValueArray())
			{
				rapidjson::Value rapidjson_v;
				SetRapidJsonValue(rapidjson_doc, rapidjson_v, v);
				rapidjson_value.PushBack(rapidjson_v, allocator);
			}
			break;

		case JsonValueType::Object:
			rapidjson_value.SetObject();
			for (auto const& v : value.ValueObject())
			{
				rapidjson::Value rapidjson_v;
				SetRapidJsonValue(rapidjson_doc, rapidjson_v, v.second);
				rapidjson_value.AddMember(rapidjson::Value(v.first.c_str(), allocator), rapidjson_v, allocator);
			}
			break;

		default:
			KFL_UNREACHABLE("Invalid type");
		}
	}

	template <typename T>
	void FillJsonValueFromXML(rapidjson::Document& rapidjson_doc, rapidjson::Value& rapidjson_value, T const& xml)
	{
		bool bool_value;
		if (xml.TryConvertValue(bool_value))
		{
			rapidjson_value.SetBool(bool_value);
		}
		else
		{
			int32_t int_value;
			if (xml.TryConvertValue(int_value))
			{
				rapidjson_value.SetInt(int_value);
			}
			else
			{
				uint32_t uint_value;
				if (xml.TryConvertValue(uint_value))
				{
					rapidjson_value.SetUint(uint_value);
				}
				else
				{
					float float_value;
					if (xml.TryConvertValue(float_value))
					{
						rapidjson_value.SetDouble(float_value);
					}
					else
					{
						auto& allocator = rapidjson_doc.GetAllocator();

						std::string_view const value_sv = xml.ValueString();
						rapidjson_value.SetString(value_sv.data(), static_cast<rapidjson::SizeType>(value_sv.size()), allocator);
					}
				}
			}
		}
	}

	void AppendJsonValue(rapidjson::Document& rapidjson_doc, rapidjson::Value& rapidjson_value, XMLNode const& node)
	{
		auto& allocator = rapidjson_doc.GetAllocator();

		if (node.FirstNode() || node.FirstAttrib())
		{
			rapidjson_value.SetObject();

			std::vector<std::vector<XMLNode const*>> cache;
			for (auto* child = node.FirstNode(); child; child = child->NextSibling())
			{
				bool found = false;
				for (auto& objects : cache)
				{
					if (objects[0]->Name() == child->Name())
					{
						objects.push_back(child);
						found = true;
						break;
					}
				}

				if (!found)
				{
					cache.emplace_back(std::vector<XMLNode const*>{child});
				}
			}
			for (auto const& objects : cache)
			{
				if (objects.size() == 1)
				{
					rapidjson::Value item_value;
					AppendJsonValue(rapidjson_doc, item_value, *objects[0]);
					rapidjson_value.AddMember(
						rapidjson::Value(objects[0]->Name().data(), static_cast<rapidjson::SizeType>(objects[0]->Name().size()), allocator),
						item_value, allocator);
				}
				else
				{
					rapidjson::Value array_value;
					array_value.SetArray();
					for (auto const& item : objects)
					{
						rapidjson::Value item_value;
						AppendJsonValue(rapidjson_doc, item_value, *item);
						array_value.PushBack(item_value, allocator);
					}
					rapidjson_value.AddMember(
						rapidjson::Value(objects[0]->Name().data(), static_cast<rapidjson::SizeType>(objects[0]->Name().size()), allocator),
						array_value, allocator);
				}
			}
			for (auto attr = node.FirstAttrib(); attr; attr = attr->NextAttrib())
			{
				std::string name_str = "@" + std::string(attr->Name());
				rapidjson::Value attr_name(name_str.c_str(), static_cast<rapidjson::SizeType>(name_str.size()), allocator);

				rapidjson::Value attr_value;
				FillJsonValueFromXML(rapidjson_doc, attr_value, *attr);

				rapidjson_value.AddMember(attr_name, attr_value, allocator);
			}

			if (!node.ValueString().empty())
			{
				rapidjson::Value node_value;
				FillJsonValueFromXML(rapidjson_doc, node_value, node);
				rapidjson_value.AddMember("#text", node_value, allocator);
			}
		}
		else
		{
			FillJsonValueFromXML(rapidjson_doc, rapidjson_value, node);
		}
	}
} // namespace

namespace KlayGE
{
	class JsonValue::Impl
	{
	public:
		explicit Impl(JsonValueType type) : type_(type)
		{
			switch (type)
			{
			case JsonValueType::Null:
				break;
			case JsonValueType::Bool:
				value_ = false;
				break;
			case JsonValueType::Int:
				value_ = 0;
				break;
			case JsonValueType::UInt:
				value_ = 0u;
				break;
			case JsonValueType::Float:
				value_ = 0.0f;
				break;
			case JsonValueType::String:
				value_ = std::string();
				break;
			case JsonValueType::Array:
				value_ = std::vector<JsonValue>();
				break;
			case JsonValueType::Object:
				value_ = std::vector<std::pair<std::string, JsonValue>>();
				break;

			default:
				KFL_UNREACHABLE("Invalid type");
			}
		}
		Impl() : type_(JsonValueType::Null)
		{
		}
		explicit Impl(bool value) : type_(JsonValueType::Bool)
		{
			value_ = value;
		}
		explicit Impl(int32_t value) : type_(JsonValueType::Int)
		{
			value_ = value;
		}
		explicit Impl(uint32_t value) : type_(JsonValueType::UInt)
		{
			value_ = value;
		}
		explicit Impl(float value) : type_(JsonValueType::Float)
		{
			value_ = value;
		}
		explicit Impl(std::string_view value) : type_(JsonValueType::String)
		{
			value_ = std::string(std::move(value));
		}
		explicit Impl(std::span<JsonValue> values) : type_(JsonValueType::Array)
		{
			value_ = std::vector<JsonValue>(values.begin(), values.end());
		}
		explicit Impl(std::span<std::pair<std::string, JsonValue>> values) : type_(JsonValueType::Object)
		{
			value_ = std::vector<std::pair<std::string, JsonValue>>(values.begin(), values.end());
		}
		Impl(Impl const& rhs) = default;
		Impl(Impl&& rhs) noexcept = default;
		~Impl() noexcept = default;

		Impl& operator=(Impl const& rhs) = default;
		Impl& operator=(Impl&& rhs) noexcept = default;

		JsonValueType Type() const noexcept
		{
			return type_;
		}

		JsonValue const* Member(std::string_view name) const
		{
			return const_cast<Impl*>(this)->Member(name);
		}

		JsonValue* Member(std::string_view name)
		{
			if (type_ != JsonValueType::Object)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			for (auto& v : obj_value)
			{
				if (v.first == name)
				{
					return &v.second;
				}
			}
			return nullptr;
		}

		void InsertAfterValue(JsonValue const& location, std::string_view name, JsonValue new_value)
		{
			if (type_ != JsonValueType::Object)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			for (auto iter = obj_value.begin(); iter != obj_value.end(); ++iter)
			{
				if (&iter->second == &location)
				{
					obj_value.emplace(iter, std::move(name), std::move(new_value));
					break;
				}
			}
		}

		void InsertAfterValue(JsonValue const& location, JsonValue new_value)
		{
			if (type_ != JsonValueType::Array)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& array_value = std::get<std::vector<JsonValue>>(value_);
			for (auto iter = array_value.begin(); iter != array_value.end(); ++iter)
			{
				if (&(*iter) == &location)
				{
					array_value.emplace(iter, std::move(new_value));
					break;
				}
			}
		}

		void AppendValue(std::string_view name, JsonValue new_value)
		{
			if (type_ != JsonValueType::Object)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			obj_value.emplace_back(std::move(name), std::move(new_value));
		}

		void AppendValue(JsonValue new_value)
		{
			if (type_ != JsonValueType::Array)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& array_value = std::get<std::vector<JsonValue>>(value_);
			array_value.emplace_back(std::move(new_value));
		}

		void RemoveValue(JsonValue const& value)
		{
			if (type_ == JsonValueType::Array)
			{
				auto& array_value = std::get<std::vector<JsonValue>>(value_);
				for (auto iter = array_value.begin(); iter != array_value.end(); ++iter)
				{
					if (&(*iter) == &value)
					{
						array_value.erase(iter);
						break;
					}
				}
			}
			else if (type_ == JsonValueType::Object)
			{
				auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
				for (auto iter = obj_value.begin(); iter != obj_value.end(); ++iter)
				{
					if (&iter->second == &value)
					{
						obj_value.erase(iter);
						break;
					}
				}
			}
			else
			{
				KFL_UNREACHABLE("Can't be called");
			}
		}

		void ClearValues()
		{
			if (type_ == JsonValueType::Array)
			{
				auto& array_value = std::get<std::vector<JsonValue>>(value_);
				array_value.clear();
			}
			else if (type_ == JsonValueType::Object)
			{
				auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
				obj_value.clear();
			}
			else
			{
				KFL_UNREACHABLE("Can't be called");
			}
		}

		void Value(bool value)
		{
			type_ = JsonValueType::Bool;
			value_ = value;
		}

		void Value(int32_t value)
		{
			type_ = JsonValueType::Int;
			value_ = value;
		}

		void Value(uint32_t value)
		{
			type_ = JsonValueType::UInt;
			value_ = value;
		}

		void Value(float value)
		{
			type_ = JsonValueType::Float;
			value_ = value;
		}

		void Value(std::string_view value)
		{
			type_ = JsonValueType::String;
			value_ = std::string(std::move(value));
		}

		void Value(std::span<JsonValue> values)
		{
			type_ = JsonValueType::Array;
			auto& array_value = std::get<std::vector<JsonValue>>(value_);
			array_value.assign(values.begin(), values.end());
		}

		void Value(std::span<std::pair<std::string, JsonValue>> values)
		{
			type_ = JsonValueType::Object;
			auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			obj_value.assign(values.begin(), values.end());
		}

		bool ValueBool() const
		{
			if (type_ != JsonValueType::Bool)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			return std::get<bool>(value_);
		}

		int32_t ValueInt() const
		{
			if (type_ != JsonValueType::Int)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			return std::get<int32_t>(value_);
		}

		uint32_t ValueUInt() const
		{
			if (type_ != JsonValueType::UInt)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			return std::get<uint32_t>(value_);
		}

		float ValueFloat() const
		{
			if (type_ != JsonValueType::Float)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			return std::get<float>(value_);
		}

		std::string_view ValueString() const
		{
			if (type_ != JsonValueType::String)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			return std::get<std::string>(value_);
		}

		std::span<JsonValue const> ValueArray() const
		{
			if (type_ != JsonValueType::Array)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			const auto& array_value = std::get<std::vector<JsonValue>>(value_);
			return MakeSpan(array_value);
		}

		std::span<std::pair<std::string, JsonValue> const> ValueObject() const
		{
			if (type_ != JsonValueType::Object)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			const auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			return MakeSpan(obj_value);
		}

		void ValueIndex(uint32_t index, std::string_view name, JsonValue value)
		{
			if (type_ != JsonValueType::Object)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& obj_value = std::get<std::vector<std::pair<std::string, JsonValue>>>(value_);
			if (obj_value.size() < index + 1)
			{
				obj_value.resize(index + 1);
			}

			obj_value[index] = {std::string(std::move(name)), std::move(value)};
		}

		void ValueIndex(uint32_t index, JsonValue value)
		{
			if (type_ != JsonValueType::Array)
			{
				KFL_UNREACHABLE("Can't be called");
			}

			auto& array_value = std::get<std::vector<JsonValue>>(value_);
			if (array_value.size() < index + 1)
			{
				array_value.resize(index + 1);
			}

			array_value[index] = std::move(value);
		}

	private:
		JsonValueType type_;
		std::variant<bool, int32_t, uint32_t, float, std::string, std::vector<JsonValue>, std::vector<std::pair<std::string, JsonValue>>>
			value_;
	};

	JsonValue::JsonValue(JsonValueType type)
		: pimpl_(MakeUniquePtr<Impl>(type))
	{
	}

	JsonValue::JsonValue()
		: pimpl_(MakeUniquePtr<Impl>())
	{
	}

	JsonValue::JsonValue(bool value)
		: pimpl_(MakeUniquePtr<Impl>(value))
	{
	}

	JsonValue::JsonValue(int32_t value)
		: pimpl_(MakeUniquePtr<Impl>(value))
	{
	}

	JsonValue::JsonValue(uint32_t value)
		: pimpl_(MakeUniquePtr<Impl>(value))
	{
	}

	JsonValue::JsonValue(float value)
		: pimpl_(MakeUniquePtr<Impl>(value))
	{
	}

	JsonValue::JsonValue(std::string_view value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(value)))
	{
	}

	JsonValue::JsonValue(std::span<JsonValue> values)
		: pimpl_(MakeUniquePtr<Impl>(std::move(values)))
	{
	}

	JsonValue::JsonValue(std::span<std::pair<std::string, JsonValue>> values)
		: pimpl_(MakeUniquePtr<Impl>(std::move(values)))
	{
	}

	JsonValue::JsonValue(JsonValue const& rhs)
		: pimpl_(MakeUniquePtr<Impl>(*rhs.pimpl_))
	{
	}

	JsonValue::JsonValue(JsonValue&& rhs) noexcept = default;

	JsonValue::~JsonValue() noexcept = default;

	JsonValue& JsonValue::operator=(JsonValue const& rhs)
	{
		if (this != &rhs)
		{
			pimpl_ = MakeUniquePtr<Impl>(*rhs.pimpl_);
		}

		return *this;
	}

	JsonValue& JsonValue::operator=(JsonValue&& rhs) noexcept = default;

	JsonValueType JsonValue::Type() const noexcept
	{
		return pimpl_->Type();
	}

	const JsonValue* JsonValue::Member(std::string_view name) const
	{
		return pimpl_->Member(std::move(name));
	}
	JsonValue* JsonValue::Member(std::string_view name)
	{
		return pimpl_->Member(std::move(name));
	}

	void JsonValue::InsertAfterValue(JsonValue const& location, std::string_view name, JsonValue new_value)
	{
		pimpl_->InsertAfterValue(location, std::move(name), std::move(new_value));
	}

	void JsonValue::AppendValue(std::string_view name, JsonValue new_value)
	{
		pimpl_->AppendValue(std::move(name), std::move(new_value));
	}

	void JsonValue::InsertAfterValue(JsonValue const& location, JsonValue new_value)
	{
		pimpl_->InsertAfterValue(location, std::move(new_value));
	}

	void JsonValue::AppendValue(JsonValue new_value)
	{
		pimpl_->AppendValue(std::move(new_value));
	}

	void JsonValue::RemoveValue(JsonValue const& value)
	{
		pimpl_->RemoveValue(value);
	}

	void JsonValue::ClearValues()
	{
		pimpl_->ClearValues();
	}

	bool JsonValue::ValueBool() const
	{
		return pimpl_->ValueBool();
	}

	int32_t JsonValue::ValueInt() const
	{
		return pimpl_->ValueInt();
	}

	uint32_t JsonValue::ValueUInt() const
	{
		return pimpl_->ValueUInt();
	}

	float JsonValue::ValueFloat() const
	{
		return pimpl_->ValueFloat();
	}

	std::string_view JsonValue::ValueString() const
	{
		return pimpl_->ValueString();
	}

	std::span<JsonValue const> JsonValue::ValueArray() const
	{
		return pimpl_->ValueArray();
	}

	std::span<std::pair<std::string, JsonValue> const> JsonValue::ValueObject() const
	{
		return pimpl_->ValueObject();
	}

	void JsonValue::Value(bool value)
	{
		pimpl_->Value(value);
	}

	void JsonValue::Value(int32_t value)
	{
		pimpl_->Value(value);
	}

	void JsonValue::Value(uint32_t value)
	{
		pimpl_->Value(value);
	}

	void JsonValue::Value(float value)
	{
		pimpl_->Value(value);
	}

	void JsonValue::Value(std::string_view value)
	{
		pimpl_->Value(std::move(value));
	}

	void JsonValue::Value(std::span<JsonValue> value)
	{
		pimpl_->Value(std::move(value));
	}

	void JsonValue::Value(std::span<std::pair<std::string, JsonValue>> values)
	{
		pimpl_->Value(std::move(values));
	}

	void JsonValue::ValueIndex(uint32_t index, JsonValue value)
	{
		pimpl_->ValueIndex(index, std::move(value));
	}

	void JsonValue::ValueIndex(uint32_t index, std::string_view name, JsonValue value)
	{
		pimpl_->ValueIndex(index, std::move(name), std::move(value));
	}

	JsonValue LoadJson(ResIdentifier& source)
	{
		source.seekg(0, std::ios_base::end);
		size_t const len = static_cast<size_t>(source.tellg());
		source.seekg(0, std::ios_base::beg);
		auto xml_src = MakeUniquePtr<char[]>(len + 1);
		source.read(&xml_src[0], len);
		xml_src[len] = 0;

		rapidjson::Document doc;
		doc.Parse(xml_src.get());
		Verify(!doc.HasParseError());

		return CreateJsonValue(doc);
	}

	void SaveJson(JsonValue const& value, std::ostream& os)
	{
		rapidjson::Document doc;
		SetRapidJsonValue(doc, doc, value);

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		doc.Accept(writer);
		os << sb.GetString();
	}

	void SaveJson(XMLNode const& node, std::ostream& os)
	{
		rapidjson::Document doc;
		AppendJsonValue(doc, doc, node);

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		doc.Accept(writer);
		os << sb.GetString();
	}
} // namespace KlayGE
