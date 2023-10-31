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

#include <string>

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#if (_MSC_VER >= 1929)
#pragma warning(disable : 5054) // Operator '|': deprecated between enumerations of different types
#endif
#pragma warning(disable : 6313) // Incorrect operator: zero-valued flag cannot be tested with bitwise-and
#endif
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#endif

#include <KFL/JsonDom.hpp>
#include <KFL/XMLDom.hpp>

using namespace KlayGE;

namespace
{
	class JsonValueNull final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Null;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			return MakeUniquePtr<JsonValueNull>();
		}
	};

	class JsonValueBool final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Bool;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueBool>();
			ret->value_ = value_;
			return ret;
		}

		bool ValueBool() const override
		{
			return value_;
		}

		void Value(bool value) override
		{
			value_ = value;
		}

		using JsonValue::Value;

	private:
		bool value_{};
	};

	class JsonValueInt final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Int;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueInt>();
			ret->value_ = value_;
			return ret;
		}

		int32_t ValueInt() const override
		{
			return value_;
		}

		void Value(int32_t value) override
		{
			value_ = value;
		}

		using JsonValue::Value;

	private:
		int32_t value_{};
	};

	class JsonValueUInt final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::UInt;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueUInt>();
			ret->value_ = value_;
			return ret;
		}

		uint32_t ValueUInt() const override
		{
			return value_;
		}

		void Value(uint32_t value) override
		{
			value_ = value;
		}

		using JsonValue::Value;

	private:
		uint32_t value_{};
	};

	class JsonValueFloat final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Float;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueFloat>();
			ret->value_ = value_;
			return ret;
		}

		float ValueFloat() const override
		{
			return value_;
		}

		void Value(float value) override
		{
			value_ = value;
		}

		using JsonValue::Value;

	private:
		float value_{};
	};

	class JsonValueString final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::String;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueString>();
			ret->value_ = value_;
			return ret;
		}

		std::string_view ValueString() const override
		{
			return value_;
		}

		void Value(std::string_view value) override
		{
			value_ = std::string(std::move(value));
		}

		using JsonValue::Value;

	private:
		std::string value_;
	};

	class JsonValueArray final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Array;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueArray>();
			ret->values_.reserve(values_.size());
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				ret->values_.emplace_back((*iter)->Clone());
			}
			return ret;
		}

		void InsertAfterValue(JsonValue const& location, std::unique_ptr<JsonValue> new_value) override
		{
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				if (iter->get() == &location)
				{
					values_.emplace(iter, std::move(new_value));
					break;
				}
			}
		}

		using JsonValue::InsertAfterValue;

		void AppendValue(std::unique_ptr<JsonValue> new_value) override
		{
			values_.emplace_back(std::move(new_value));
		}

		using JsonValue::AppendValue;

		void RemoveValue(JsonValue const& value) override
		{
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				if (iter->get() == &value)
				{
					values_.erase(iter);
					break;
				}
			}
		}

		void ClearValues() override
		{
			values_.clear();
		}

		std::vector<std::unique_ptr<JsonValue>> const& ValueArray() const override
		{
			return values_;
		}

		void Value(std::vector<std::unique_ptr<JsonValue>> values) override
		{
			values_ = std::move(values);
		}

		using JsonValue::Value;

		void ValueIndex(uint32_t index, std::unique_ptr<JsonValue> value) override
		{
			if (values_.size() < index + 1)
			{
				values_.resize(index + 1);
			}

			values_[index] = std::move(value);
		}

		using JsonValue::ValueIndex;

	private:
		std::vector<std::unique_ptr<JsonValue>> values_;
	};

	class JsonValueObject final : public JsonValue
	{
	public:
		JsonValueType Type() const noexcept override
		{
			return JsonValueType::Object;
		}

		std::unique_ptr<JsonValue> Clone() override
		{
			auto ret = MakeUniquePtr<JsonValueObject>();
			ret->values_.reserve(values_.size());
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				ret->values_.emplace_back(iter->first, iter->second->Clone());
			}
			return ret;
		}

		JsonValue* Member(std::string_view name) const override
		{
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				if (iter->first == name)
				{
					return iter->second.get();
				}
			}
			return nullptr;
		}

		void InsertAfterValue(JsonValue const& location, std::string_view name, std::unique_ptr<JsonValue> new_value) override
		{
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				if (iter->second.get() == &location)
				{
					values_.emplace(iter, std::move(name), std::move(new_value));
					break;
				}
			}
		}

		using JsonValue::InsertAfterValue;

		void AppendValue(std::string_view name, std::unique_ptr<JsonValue> new_value) override
		{
			values_.emplace_back(std::move(name), std::move(new_value));
		}

		using JsonValue::AppendValue;

		void RemoveValue(JsonValue const& value) override
		{
			for (auto iter = values_.begin(); iter != values_.end(); ++iter)
			{
				if (iter->second.get() == &value)
				{
					values_.erase(iter);
					break;
				}
			}
		}

		void ClearValues() override
		{
			values_.clear();
		}

		std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> const& ValueObject() const override
		{
			return values_;
		}

		void Value(std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values) override
		{
			values_ = std::move(values);
		}

		using JsonValue::Value;

		void ValueIndex(uint32_t index, std::string_view name, std::unique_ptr<JsonValue> value) override
		{
			if (values_.size() < index + 1)
			{
				values_.resize(index + 1);
			}

			values_[index] = {std::string(std::move(name)), std::move(value)};
		}

		using JsonValue::ValueIndex;

	private:
		std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values_;
	};

	std::unique_ptr<JsonValue> CreateJsonValueFromRapidJsonValue(JsonDocument& doc, rapidjson::Value& value)
	{
		if (value.IsNull())
		{
			return MakeUniquePtr<JsonValueNull>();
		}
		else if (value.IsFalse())
		{
			auto ret = MakeUniquePtr<JsonValueBool>();
			ret->Value(false);
			return ret;
		}
		else if (value.IsTrue())
		{
			auto ret = MakeUniquePtr<JsonValueBool>();
			ret->Value(true);
			return ret;
		}
		else if (value.IsBool())
		{
			auto ret = MakeUniquePtr<JsonValueBool>();
			ret->Value(value.GetBool());
			return ret;
		}
		else if (value.IsInt())
		{
			auto ret = MakeUniquePtr<JsonValueInt>();
			ret->Value(value.GetInt());
			return ret;
		}
		else if (value.IsUint())
		{
			auto ret = MakeUniquePtr<JsonValueUInt>();
			ret->Value(value.GetUint());
			return ret;
		}
		else if (value.IsInt64())
		{
			auto ret = MakeUniquePtr<JsonValueInt>();
			ret->Value(static_cast<int32_t>(value.GetInt64()));
			return ret;
		}
		else if (value.IsUint64())
		{
			auto ret = MakeUniquePtr<JsonValueInt>();
			ret->Value(static_cast<uint32_t>(value.GetUint64()));
			return ret;
		}
		else if (value.IsDouble())
		{
			auto ret = MakeUniquePtr<JsonValueFloat>();
			ret->Value(static_cast<float>(value.GetDouble()));
			return ret;
		}
		else if (value.IsString())
		{
			auto ret = MakeUniquePtr<JsonValueString>();
			ret->Value(std::string(value.GetString()));
			return ret;
		}
		else if (value.IsArray())
		{
			auto ret = MakeUniquePtr<JsonValueArray>();
			for (auto iter = value.Begin(); iter != value.End(); ++iter)
			{
				ret->AppendValue(CreateJsonValueFromRapidJsonValue(doc, *iter));
			}
			return ret;
		}
		else if (value.IsObject())
		{
			auto ret = MakeUniquePtr<JsonValueObject>();
			for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); ++iter)
			{
				ret->AppendValue(iter->name.GetString(), CreateJsonValueFromRapidJsonValue(doc, iter->value));
			}
			return ret;
		}
		else
		{
			KFL_UNREACHABLE("Invalid type");
		}
	}

	void SetRapidJsonValueFromJsonValue(rapidjson::Document& rapidjson_doc, rapidjson::Value& rapidjson_value, JsonValue const& value)
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
				SetRapidJsonValueFromJsonValue(rapidjson_doc, rapidjson_v, *v);
				rapidjson_value.PushBack(rapidjson_v, allocator);
			}
			break;

		case JsonValueType::Object:
			rapidjson_value.SetObject();
			for (auto const& v : value.ValueObject())
			{
				rapidjson::Value rapidjson_v;
				SetRapidJsonValueFromJsonValue(rapidjson_doc, rapidjson_v, *v.second);
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

	void AppendJsonValueFromDomNode(rapidjson::Document& rapidjson_doc, rapidjson::Value& rapidjson_value, XMLNode const& node)
	{
		auto& allocator = rapidjson_doc.GetAllocator();

		if (node.FirstNode() || node.FirstAttrib())
		{
			rapidjson_value.SetObject();

			std::vector<std::vector<XMLNode*>> cache;
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
					cache.emplace_back(std::vector<XMLNode*>{child});
				}
			}
			for (auto const& objects : cache)
			{
				if (objects.size() == 1)
				{
					rapidjson::Value item_value;
					AppendJsonValueFromDomNode(rapidjson_doc, item_value, *objects[0]);
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
						AppendJsonValueFromDomNode(rapidjson_doc, item_value, *item);
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
	JsonValue* JsonDocument::RootValue() const
	{
		return root_.get();
	}

	void JsonDocument::RootValue(std::unique_ptr<JsonValue> new_value)
	{
		root_ = std::move(new_value);
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValue(JsonValueType type)
	{
		switch (type)
		{
		case JsonValueType::Null:
			return MakeUniquePtr<JsonValueNull>();
		case JsonValueType::Bool:
			return MakeUniquePtr<JsonValueBool>();
		case JsonValueType::Int:
			return MakeUniquePtr<JsonValueInt>();
		case JsonValueType::UInt:
			return MakeUniquePtr<JsonValueUInt>();
		case JsonValueType::Float:
			return MakeUniquePtr<JsonValueFloat>();
		case JsonValueType::String:
			return MakeUniquePtr<JsonValueString>();
		case JsonValueType::Array:
			return MakeUniquePtr<JsonValueArray>();
		case JsonValueType::Object:
			return MakeUniquePtr<JsonValueObject>();

		default:
			KFL_UNREACHABLE("Invalid type");
		}
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueNull()
	{
		return MakeUniquePtr<JsonValueNull>();
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueBool(bool value)
	{
		auto ret = MakeUniquePtr<JsonValueBool>();
		ret->Value(value);
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueInt(int32_t value)
	{
		auto ret = MakeUniquePtr<JsonValueInt>();
		ret->Value(value);
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueUInt(uint32_t value)
	{
		auto ret = MakeUniquePtr<JsonValueUInt>();
		ret->Value(value);
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueFloat(float value)
	{
		auto ret = MakeUniquePtr<JsonValueFloat>();
		ret->Value(value);
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueString(std::string_view value)
	{
		auto ret = MakeUniquePtr<JsonValueString>();
		ret->Value(std::move(value));
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueArray(std::vector<std::unique_ptr<JsonValue>> values)
	{
		auto ret = MakeUniquePtr<JsonValueArray>();
		ret->Value(std::move(values));
		return ret;
	}

	std::unique_ptr<JsonValue> JsonDocument::AllocValueObject(std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values)
	{
		auto ret = MakeUniquePtr<JsonValueObject>();
		ret->Value(std::move(values));
		return ret;
	}


	JsonValue::~JsonValue() noexcept = default;

	JsonValue* JsonValue::Member([[maybe_unused]] std::string_view name) const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::InsertAfterValue([[maybe_unused]] JsonValue const& location, [[maybe_unused]] std::string_view name,
		[[maybe_unused]] std::unique_ptr<JsonValue> new_value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::AppendValue([[maybe_unused]] std::string_view name, [[maybe_unused]] std::unique_ptr<JsonValue> new_value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::InsertAfterValue([[maybe_unused]] JsonValue const& location, [[maybe_unused]] std::unique_ptr<JsonValue> new_value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::AppendValue([[maybe_unused]] std::unique_ptr<JsonValue> new_value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::RemoveValue([[maybe_unused]] JsonValue const& value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::ClearValues()
	{
		KFL_UNREACHABLE("Can't be called");
	}

	bool JsonValue::ValueBool() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	int32_t JsonValue::ValueInt() const
	{
		KFL_UNREACHABLE("Can't be called");
	}
	uint32_t JsonValue::ValueUInt() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	float JsonValue::ValueFloat() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	std::string_view JsonValue::ValueString() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	std::vector<std::unique_ptr<JsonValue>> const& JsonValue::ValueArray() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> const& JsonValue::ValueObject() const
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] bool value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] int32_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] uint32_t value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] float value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] std::string_view value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] std::vector<std::unique_ptr<JsonValue>> value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::Value([[maybe_unused]] std::vector<std::pair<std::string, std::unique_ptr<JsonValue>>> values)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::ValueIndex([[maybe_unused]] uint32_t index, [[maybe_unused]] std::unique_ptr<JsonValue> value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	void JsonValue::ValueIndex(
		[[maybe_unused]] uint32_t index, [[maybe_unused]] std::string_view name, [[maybe_unused]] std::unique_ptr<JsonValue> value)
	{
		KFL_UNREACHABLE("Can't be called");
	}

	std::unique_ptr<JsonDocument> LoadJson(ResIdentifier& source)
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

		auto ret = MakeUniquePtr<JsonDocument>();
		ret->RootValue(CreateJsonValueFromRapidJsonValue(*ret, doc));

		return ret;
	}

	void SaveJson(JsonDocument const& dom, std::ostream& os)
	{
		rapidjson::Document doc;
		SetRapidJsonValueFromJsonValue(doc, doc, *dom.RootValue());

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		doc.Accept(writer);
		os << sb.GetString();
	}

	void SaveJson(XMLDocument const& dom, std::ostream& os)
	{
		rapidjson::Document doc;
		AppendJsonValueFromDomNode(doc, doc, *dom.RootNode());

		rapidjson::StringBuffer sb;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
		doc.Accept(writer);
		os << sb.GetString();
	}
} // namespace KlayGE
