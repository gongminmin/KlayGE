/**
 * @file XMLDom.cpp
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

#include <KFL/ResIdentifier.hpp>
#include <KFL/StringUtil.hpp>
#include <KFL/Util.hpp>

#include <iterator>
#include <string>
#ifdef KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
#include <charconv>
#endif

#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 6313) // Incorrect operator: zero-valued flag cannot be tested with bitwise-and
#endif
#include <rapidxml.hpp>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#endif
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable : 4100) // 'flags': unreferenced formal parameter
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" // Ignore unused parameter 'flags'
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // Ignore unused parameter 'flags'
#endif
#include <rapidxml_print.hpp>
#if defined(KLAYGE_COMPILER_MSVC)
#pragma warning(pop)
#elif defined(KLAYGE_COMPILER_GCC)
#pragma GCC diagnostic pop
#elif defined(KLAYGE_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

#include <KFL/XMLDom.hpp>

using namespace KlayGE;

namespace
{
	std::unique_ptr<XMLAttribute> CreateXmlAttribFromRapidXmlAttrib(XMLDocument& doc, rapidxml::xml_attribute<char> const& attrib)
	{
		return doc.AllocAttribString(
			std::string_view(attrib.name(), attrib.name_size()), std::string_view(attrib.value(), attrib.value_size()));
	}

	rapidxml::xml_attribute<char>* CreateRapidXmlAttribFromXmlAttrib(rapidxml::xml_document<char>& doc, XMLAttribute const& attrib)
	{
		auto* ret = doc.allocate_attribute();

		std::string_view const name = attrib.Name();
		std::string_view const value = attrib.ValueString();
		ret->name(name.data(), name.size());
		ret->value(value.data(), value.size());

		return ret;
	}

	std::unique_ptr<XMLNode> CreateXmlNodeFromRapidXmlNode(XMLDocument& doc, rapidxml::xml_node<char> const& node)
	{
		XMLNodeType type;
		switch (node.type())
		{
		case rapidxml::node_document:
			type = XMLNodeType::Document;
			break;

		case rapidxml::node_element:
			type = XMLNodeType::Element;
			break;

		case rapidxml::node_data:
			type = XMLNodeType::Data;
			break;

		case rapidxml::node_cdata:
			type = XMLNodeType::CData;
			break;

		case rapidxml::node_comment:
			type = XMLNodeType::Comment;
			break;

		case rapidxml::node_declaration:
			type = XMLNodeType::Declaration;
			break;

		case rapidxml::node_doctype:
			type = XMLNodeType::Doctype;
			break;

		case rapidxml::node_pi:
		default:
			type = XMLNodeType::PI;
			break;
		}

		auto ret = doc.AllocNode(type, std::string_view(node.name(), node.name_size()));
		ret->Value(std::string_view(node.value(), node.value_size()));

		for (auto* child = node.first_node(); child; child = child->next_sibling())
		{
			ret->AppendNode(CreateXmlNodeFromRapidXmlNode(doc, *child));
		}
		for (auto* attr = node.first_attribute(); attr; attr = attr->next_attribute())
		{
			ret->AppendAttrib(CreateXmlAttribFromRapidXmlAttrib(doc, *attr));
		}

		return ret;
	}

	rapidxml::xml_node<char>* CreateRapidXmlNodeFromXmlNode(rapidxml::xml_document<char>& doc, XMLNode const& node)
	{
		rapidxml::node_type type;
		switch (node.Type())
		{
		case XMLNodeType::Document:
			type = rapidxml::node_document;
			break;

		case XMLNodeType::Element:
			type = rapidxml::node_element;
			break;

		case XMLNodeType::Data:
			type = rapidxml::node_data;
			break;

		case XMLNodeType::CData:
			type = rapidxml::node_cdata;
			break;

		case XMLNodeType::Comment:
			type = rapidxml::node_comment;
			break;

		case XMLNodeType::Declaration:
			type = rapidxml::node_declaration;
			break;

		case XMLNodeType::Doctype:
			type = rapidxml::node_doctype;
			break;

		case XMLNodeType::PI:
		default:
			type = rapidxml::node_pi;
			break;
		}

		auto* ret = doc.allocate_node(type);

		std::string_view const name = node.Name();
		std::string_view const value = node.ValueString();
		ret->name(name.data(), name.size());
		ret->value(value.data(), value.size());

		for (auto child = node.FirstNode(); child; child = child->NextSibling())
		{
			ret->append_node(CreateRapidXmlNodeFromXmlNode(doc, *child));
		}
		for (auto attr = node.FirstAttrib(); attr; attr = attr->NextAttrib())
		{
			ret->append_attribute(CreateRapidXmlAttribFromXmlAttrib(doc, *attr));
		}

		return ret;
	}

	bool TryConvertStringToValue(std::string const& value_str, int32_t& val)
	{
#ifdef KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
		char const* str = value_str.data();
		std::from_chars_result result = std::from_chars(str, str + value_str.size(), val);
		return (result.ec == std::errc());
#else
		try
		{
			val = std::stol(value_str);
			return true;
		}
		catch (...)
		{
			return false;
		}
#endif
	}

	bool TryConvertStringToValue(std::string const& value_str, uint32_t& val)
	{
#ifdef KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
		char const* str = value_str.data();
		std::from_chars_result result = std::from_chars(str, str + value_str.size(), val);
		return (result.ec == std::errc());
#else
		try
		{
			val = std::stoul(value_str);
			return true;
		}
		catch (...)
		{
			return false;
		}
#endif
	}

	bool TryConvertStringToValue(std::string const& value_str, float& val)
	{
#ifdef KLAYGE_CXX17_LIBRARY_CHARCONV_SUPPORT
		char const* str = value_str.data();
		std::from_chars_result result = std::from_chars(str, str + value_str.size(), val);
		return (result.ec == std::errc());
#else
		try
		{
			val = std::stof(value_str);
			return true;
		}
		catch (...)
		{
			return false;
		}
#endif
	}

	bool TryConvertStringToValue(std::string const& value_str, bool& val)
	{
		std::string lower_value_str = value_str;
		StringUtil::ToLower(lower_value_str);
		if (lower_value_str == "true")
		{
			val = true;
			return true;
		}
		else if (lower_value_str == "false")
		{
			val = false;
			return true;
		}
		else
		{
			uint32_t uint_val;
			bool const ret = TryConvertStringToValue(value_str, uint_val);
			val = (uint_val != 0) ? true : false;
			return ret;
		}
	}
} // namespace

namespace KlayGE
{
	XMLNode* XMLDocument::RootNode() const
	{
		return root_.get();
	}

	void XMLDocument::RootNode(std::unique_ptr<XMLNode> new_node)
	{
		root_ = std::move(new_node);
	}

	std::unique_ptr<XMLNode> XMLDocument::CloneNode(XMLNode const& node)
	{
		auto ret = this->AllocNode(node.Type(), node.Name());
		ret->Value(node.ValueString());

		for (auto child = node.FirstNode(); child; child = child->NextSibling())
		{
			ret->AppendNode(this->CloneNode(*child));
		}
		for (auto attr = node.FirstAttrib(); attr; attr = attr->NextAttrib())
		{
			ret->AppendAttrib(this->CloneAttrib(*attr));
		}

		return ret;
	}

	std::unique_ptr<XMLAttribute> XMLDocument::CloneAttrib(XMLAttribute const& attrib)
	{
		return this->AllocAttribString(attrib.Name(), attrib.ValueString());
	}

	std::unique_ptr<XMLNode> XMLDocument::AllocNode(XMLNodeType type, std::string_view name)
	{
		auto ret = MakeUniquePtr<XMLNode>(type);
		ret->Name(std::move(name));
		return ret;
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttrib(std::string_view name)
	{
		auto ret = MakeUniquePtr<XMLAttribute>();
		ret->Name(std::move(name));
		return ret;
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttribBool(std::string_view name, bool value)
	{
		return this->AllocAttribString(std::move(name), std::to_string(value ? 1U : 0U));
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttribInt(std::string_view name, int32_t value)
	{
		return this->AllocAttribString(std::move(name), std::to_string(value));
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttribUInt(std::string_view name, uint32_t value)
	{
		return this->AllocAttribString(std::move(name), std::to_string(value));
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttribFloat(std::string_view name, float value)
	{
		return this->AllocAttribString(std::move(name), std::to_string(value));
	}

	std::unique_ptr<XMLAttribute> XMLDocument::AllocAttribString(std::string_view name, std::string_view value)
	{
		auto ret = this->AllocAttrib(std::move(name));
		ret->Value(std::move(value));
		return ret;
	}


	XMLNode::XMLNode(XMLNodeType type) : type_(type)
	{
	}

	std::string_view XMLNode::Name() const
	{
		return name_;
	}

	void XMLNode::Name(std::string_view name)
	{
		name_ = std::move(name);
	}

	XMLNodeType XMLNode::Type() const
	{
		return type_;
	}

	XMLNode* XMLNode::Parent() const
	{
		return parent_;
	}

	void XMLNode::Parent(XMLNode* parent)
	{
		parent_ = parent;
	}

	XMLAttribute* XMLNode::FirstAttrib(std::string_view name) const
	{
		for (auto const& attr : attrs_)
		{
			if (attr->Name() == name)
			{
				return attr.get();
			}
		}

		return nullptr;
	}

	XMLAttribute* XMLNode::NextAttrib(XMLAttribute const& attrib, std::string_view name) const
	{
		for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
		{
			if (iter->get() == &attrib)
			{
				++iter;
				for (; iter != attrs_.end(); ++iter)
				{
					if ((*iter)->Name() == name)
					{
						return iter->get();
					}
				}
				break;
			}
		}

		return nullptr;
	}

	XMLAttribute* XMLNode::LastAttrib(std::string_view name) const
	{
		for (auto iter = attrs_.rbegin(); iter != attrs_.rend(); ++iter)
		{
			if ((*iter)->Name() == name)
			{
				return iter->get();
			}
		}

		return nullptr;
	}

	XMLAttribute* XMLNode::FirstAttrib() const
	{
		if (attrs_.empty())
		{
			return nullptr;
		}
		else
		{
			return attrs_.front().get();
		}
	}

	XMLAttribute* XMLNode::NextAttrib(XMLAttribute const& attrib) const
	{
		for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
		{
			if (iter->get() == &attrib)
			{
				++iter;
				if (iter != attrs_.end())
				{
					return iter->get();
				}
				break;
			}
		}

		return nullptr;
	}

	XMLAttribute* XMLNode::LastAttrib() const
	{
		if (attrs_.empty())
		{
			return nullptr;
		}
		else
		{
			return attrs_.back().get();
		}
	}

	XMLAttribute* XMLNode::Attrib(std::string_view name) const
	{
		return this->FirstAttrib(std::move(name));
	}

	bool XMLNode::TryConvertAttrib(std::string_view name, bool& val, bool default_val) const
	{
		val = default_val;

		auto attr = this->Attrib(std::move(name));
		return attr ? attr->TryConvertValue(val) : true;
	}

	bool XMLNode::TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const
	{
		val = default_val;

		auto attr = this->Attrib(std::move(name));
		return attr ? attr->TryConvertValue(val) : true;
	}

	bool XMLNode::TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const
	{
		val = default_val;

		auto attr = this->Attrib(std::move(name));
		return attr ? attr->TryConvertValue(val) : true;
	}

	bool XMLNode::TryConvertAttrib(std::string_view name, float& val, float default_val) const
	{
		val = default_val;

		auto attr = this->Attrib(std::move(name));
		return attr ? attr->TryConvertValue(val) : true;
	}

	bool XMLNode::AttribBool(std::string_view name, bool default_val) const
	{
		auto attr = this->Attrib(std::move(name));
		return attr ? attr->ValueBool() : default_val;
	}

	int32_t XMLNode::AttribInt(std::string_view name, int32_t default_val) const
	{
		auto attr = this->Attrib(std::move(name));
		return attr ? attr->ValueInt() : default_val;
	}

	uint32_t XMLNode::AttribUInt(std::string_view name, uint32_t default_val) const
	{
		auto attr = this->Attrib(std::move(name));
		return attr ? attr->ValueUInt() : default_val;
	}

	float XMLNode::AttribFloat(std::string_view name, float default_val) const
	{
		auto attr = this->Attrib(std::move(name));
		return attr ? attr->ValueFloat() : default_val;
	}

	std::string_view XMLNode::AttribString(std::string_view name, std::string_view default_val) const
	{
		auto attr = this->Attrib(std::move(name));
		return attr ? attr->ValueString() : default_val;
	}

	XMLNode* XMLNode::FirstNode(std::string_view name) const
	{
		for (auto const& node : children_)
		{
			if (node->Name() == name)
			{
				return node.get();
			}
		}

		return nullptr;
	}

	XMLNode* XMLNode::LastNode(std::string_view name) const
	{
		for (auto iter = children_.rbegin(); iter != children_.rend(); ++iter)
		{
			if ((*iter)->Name() == name)
			{
				return iter->get();
			}
		}

		return nullptr;
	}

	XMLNode* XMLNode::FirstNode() const
	{
		if (children_.empty())
		{
			return nullptr;
		}
		else
		{
			return children_.front().get();
		}
	}

	XMLNode* XMLNode::LastNode() const
	{
		if (children_.empty())
		{
			return nullptr;
		}
		else
		{
			return children_.back().get();
		}
	}

	XMLNode* XMLNode::PrevSibling(std::string_view name) const
	{
		XMLNode* ret = nullptr;
		for (auto iter = parent_->children_.begin(); iter != parent_->children_.end(); ++iter)
		{
			if ((*iter)->Name() == name)
			{
				ret = iter->get();
			}
			if (iter->get() == this)
			{
				return ret;
			}
		}

		return nullptr;
	}

	XMLNode* XMLNode::NextSibling(std::string_view name) const
	{
		for (auto iter = parent_->children_.begin(); iter != parent_->children_.end(); ++iter)
		{
			if (iter->get() == this)
			{
				++iter;
				for (; iter != parent_->children_.end(); ++iter)
				{
					if ((*iter)->Name() == name)
					{
						return iter->get();
					}
				}
				break;
			}
		}

		return nullptr;
	}

	XMLNode* XMLNode::PrevSibling() const
	{
		for (auto iter = parent_->children_.begin(); iter != parent_->children_.end(); ++iter)
		{
			if (iter->get() == this)
			{
				if (iter != parent_->children_.begin())
				{
					--iter;
					return iter->get();
				}
				break;
			}
		}

		return nullptr;
	}

	XMLNode* XMLNode::NextSibling() const
	{
		for (auto iter = parent_->children_.begin(); iter != parent_->children_.end(); ++iter)
		{
			if (iter->get() == this)
			{
				++iter;
				if (iter != parent_->children_.end())
				{
					return iter->get();
				}
				break;
			}
		}

		return nullptr;
	}

	void XMLNode::InsertAfterNode(XMLNode const& location, std::unique_ptr<XMLNode> new_node)
	{
		for (auto iter = children_.begin(); iter != children_.end(); ++iter)
		{
			if (iter->get() == &location)
			{
				new_node->Parent(this);
				children_.insert(iter, std::move(new_node));
				break;
			}
		}
	}

	void XMLNode::InsertAfterAttrib(XMLAttribute const& location, std::unique_ptr<XMLAttribute> new_attr)
	{
		for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
		{
			if (iter->get() == &location)
			{
				new_attr->Parent(this);
				attrs_.insert(iter, std::move(new_attr));
				break;
			}
		}
	}

	void XMLNode::AppendNode(std::unique_ptr<XMLNode> new_node)
	{
		new_node->Parent(this);
		children_.emplace_back(std::move(new_node));
	}

	void XMLNode::AppendAttrib(std::unique_ptr<XMLAttribute> new_attr)
	{
		new_attr->Parent(this);
		attrs_.emplace_back(std::move(new_attr));
	}

	void XMLNode::RemoveNode(XMLNode const& node)
	{
		for (auto iter = children_.begin(); iter != children_.end(); ++iter)
		{
			if (iter->get() == &node)
			{
				(*iter)->parent_ = nullptr;
				children_.erase(iter);
				break;
			}
		}
	}

	void XMLNode::RemoveAttrib(XMLAttribute const& attr)
	{
		for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
		{
			if (iter->get() == &attr)
			{
				(*iter)->Parent(nullptr);
				attrs_.erase(iter);
				break;
			}
		}
	}

	void XMLNode::ClearChildren()
	{
		for (auto iter = children_.begin(); iter != children_.end(); ++iter)
		{
			(*iter)->Parent(nullptr);
		}
		children_.clear();
	}

	void XMLNode::ClearAttribs()
	{
		for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
		{
			(*iter)->Parent(nullptr);
		}
		attrs_.clear();
	}

	bool XMLNode::TryConvertValue(bool& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLNode::TryConvertValue(int32_t& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLNode::TryConvertValue(uint32_t& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLNode::TryConvertValue(float& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLNode::ValueBool() const
	{
		bool val = false;
		this->TryConvertValue(val);
		return val;
	}

	int32_t XMLNode::ValueInt() const
	{
		int32_t val = 0;
		this->TryConvertValue(val);
		return val;
	}

	uint32_t XMLNode::ValueUInt() const
	{
		uint32_t val = 0;
		this->TryConvertValue(val);
		return val;
	}

	float XMLNode::ValueFloat() const
	{
		float val = 0;
		this->TryConvertValue(val);
		return val;
	}

	std::string_view XMLNode::ValueString() const
	{
		return value_;
	}

	void XMLNode::Value(bool value)
	{
		value_ = std::to_string(value ? 1U : 0U);
	}

	void XMLNode::Value(int32_t value)
	{
		value_ = std::to_string(value);
	}

	void XMLNode::Value(uint32_t value)
	{
		value_ = std::to_string(value);
	}

	void XMLNode::Value(float value)
	{
		value_ = std::to_string(value);
	}

	void XMLNode::Value(std::string_view value)
	{
		value_ = std::move(value);
	}


	std::string_view XMLAttribute::Name() const
	{
		return name_;
	}

	void XMLAttribute::Name(std::string_view name)
	{
		name_ = std::move(name);
	}

	XMLNode* XMLAttribute::Parent() const
	{
		return parent_;
	}

	void XMLAttribute::Parent(XMLNode* parent)
	{
		parent_ = parent;
	}

	XMLAttribute* XMLAttribute::NextAttrib(std::string_view name) const
	{
		return parent_->NextAttrib(*this, std::move(name));
	}

	XMLAttribute* XMLAttribute::NextAttrib() const
	{
		return parent_->NextAttrib(*this);
	}

	bool XMLAttribute::TryConvertValue(bool& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLAttribute::TryConvertValue(int32_t& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLAttribute::TryConvertValue(uint32_t& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLAttribute::TryConvertValue(float& val) const
	{
		return TryConvertStringToValue(value_, val);
	}

	bool XMLAttribute::ValueBool() const
	{
		uint32_t val = 0;
		this->TryConvertValue(val);
		return val ? true : false;
	}

	int32_t XMLAttribute::ValueInt() const
	{
		int32_t val = 0;
		this->TryConvertValue(val);
		return val;
	}

	uint32_t XMLAttribute::ValueUInt() const
	{
		uint32_t val = 0;
		this->TryConvertValue(val);
		return val;
	}

	float XMLAttribute::ValueFloat() const
	{
		float val = 0;
		this->TryConvertValue(val);
		return val;
	}

	std::string_view XMLAttribute::ValueString() const
	{
		return value_;
	}

	void XMLAttribute::Value(bool value)
	{
		value_ = std::to_string(value ? 1U : 0U);
	}

	void XMLAttribute::Value(int32_t value)
	{
		value_ = std::to_string(value);
	}

	void XMLAttribute::Value(uint32_t value)
	{
		value_ = std::to_string(value);
	}

	void XMLAttribute::Value(float value)
	{
		value_ = std::to_string(value);
	}

	void XMLAttribute::Value(std::string_view value)
	{
		value_ = std::move(value);
	}

	std::unique_ptr<XMLDocument> LoadXml(ResIdentifier& source)
	{
		source.seekg(0, std::ios_base::end);
		int len = static_cast<int>(source.tellg());
		source.seekg(0, std::ios_base::beg);
		auto xml_src = MakeUniquePtr<char[]>(len + 1);
		source.read(&xml_src[0], len);
		xml_src[len] = 0;

		rapidxml::xml_document<char> doc;
		doc.parse<0>(xml_src.get());

		auto ret = MakeUniquePtr<XMLDocument>();
		ret->RootNode(CreateXmlNodeFromRapidXmlNode(*ret, *doc.first_node()));

		return ret;
	}

	void SaveXml(XMLDocument const& dom, std::ostream& os)
	{
		rapidxml::xml_document<char> doc;
		rapidxml::xml_node<char>* root_node = CreateRapidXmlNodeFromXmlNode(doc, *dom.RootNode());
		doc.append_node(root_node);

		os << "<?xml version=\"1.0\"?>" << std::endl << std::endl;
		os << doc;
	}
} // namespace KlayGE
