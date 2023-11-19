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
	XMLAttribute CreateXmlAttribFromRapidXmlAttrib(rapidxml::xml_attribute<char> const& attrib)
	{
		return XMLAttribute(std::string_view(attrib.name(), attrib.name_size()), std::string_view(attrib.value(), attrib.value_size()));
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

	XMLNode CreateXmlNodeFromRapidXmlNode(rapidxml::xml_node<char> const& node)
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

		XMLNode ret(type, std::string_view(node.name(), node.name_size()));
		ret.Value(std::string_view(node.value(), node.value_size()));

		for (auto* child = node.first_node(); child; child = child->next_sibling())
		{
			ret.AppendNode(CreateXmlNodeFromRapidXmlNode(*child));
		}
		for (auto* attr = node.first_attribute(); attr; attr = attr->next_attribute())
		{
			ret.AppendAttrib(CreateXmlAttribFromRapidXmlAttrib(*attr));
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
#ifdef KLAYGE_CXX17_LIBRARY_CHARCONV_FP_SUPPORT
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
		if ((lower_value_str == "true") || (lower_value_str == "1"))
		{
			val = true;
			return true;
		}
		else if ((lower_value_str == "false") || (lower_value_str == "0"))
		{
			val = false;
			return true;
		}
		else
		{
			return false;
		}
	}
} // namespace

namespace KlayGE
{
	class XMLNode::Impl final
	{
	public:
		explicit Impl(XMLNodeType type, std::string_view name) noexcept : type_(type), name_(std::move(name))
		{
		}

		void UpdateParent(XMLNode& new_parent)
		{
			for (auto& child : children_)
			{
				child.Parent(&new_parent);
			}
			for (auto& attr : attrs_)
			{
				attr.Parent(&new_parent);
			}
		}

		std::string_view Name() const noexcept
		{
			return name_;
		}
		void Name(std::string_view name) noexcept
		{
			name_ = std::move(name);
		}

		XMLNodeType Type() const noexcept
		{
			return type_;
		}

		XMLNode* Parent() noexcept
		{
			return parent_;
		}
		void Parent(XMLNode* parent) noexcept
		{
			parent_ = parent;
		}

		XMLAttribute* FirstAttrib(std::string_view name)
		{
			for (auto& attr : attrs_)
			{
				if (attr.Name() == name)
				{
					return &attr;
				}
			}

			return nullptr;
		}
		XMLAttribute* NextAttrib(XMLAttribute const& attrib, std::string_view name)
		{
			for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
			{
				if (&(*iter) == &attrib)
				{
					++iter;
					for (; iter != attrs_.end(); ++iter)
					{
						if (iter->Name() == name)
						{
							return &(*iter);
						}
					}
					break;
				}
			}

			return nullptr;
		}
		XMLAttribute* LastAttrib(std::string_view name)
		{
			for (auto iter = attrs_.rbegin(); iter != attrs_.rend(); ++iter)
			{
				if (iter->Name() == name)
				{
					return &(*iter);
				}
			}

			return nullptr;
		}
		XMLAttribute* FirstAttrib()
		{
			if (attrs_.empty())
			{
				return nullptr;
			}
			else
			{
				return &attrs_.front();
			}
		}
		XMLAttribute* NextAttrib(XMLAttribute const& attrib)
		{
			for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
			{
				if (&(*iter) == &attrib)
				{
					++iter;
					if (iter != attrs_.end())
					{
						return &(*iter);
					}
					break;
				}
			}

			return nullptr;
		}
		XMLAttribute* LastAttrib()
		{
			if (attrs_.empty())
			{
				return nullptr;
			}
			else
			{
				return &attrs_.back();
			}
		}

		XMLNode* FirstNode(std::string_view name)
		{
			for (auto& node : children_)
			{
				if (node.Name() == name)
				{
					return &node;
				}
			}

			return nullptr;
		}
		XMLNode* LastNode(std::string_view name)
		{
			for (auto iter = children_.rbegin(); iter != children_.rend(); ++iter)
			{
				if (iter->Name() == name)
				{
					return &(*iter);
				}
			}

			return nullptr;
		}
		XMLNode* FirstNode()
		{
			if (children_.empty())
			{
				return nullptr;
			}
			else
			{
				return &children_.front();
			}
		}
		XMLNode* LastNode()
		{
			if (children_.empty())
			{
				return nullptr;
			}
			else
			{
				return &children_.back();
			}
		}

		XMLNode* PrevSibling(std::string_view name)
		{
			XMLNode* ret = nullptr;
			for (auto iter = parent_->pimpl_->children_.begin(); iter != parent_->pimpl_->children_.end(); ++iter)
			{
				if (iter->Name() == name)
				{
					ret = &(*iter);
				}
				if (iter->pimpl_.get() == this)
				{
					return ret;
				}
			}

			return nullptr;
		}
		XMLNode* NextSibling(std::string_view name)
		{
			for (auto iter = parent_->pimpl_->children_.begin(); iter != parent_->pimpl_->children_.end(); ++iter)
			{
				if (iter->pimpl_.get() == this)
				{
					++iter;
					for (; iter != parent_->pimpl_->children_.end(); ++iter)
					{
						if (iter->Name() == name)
						{
							return &(*iter);
						}
					}
					break;
				}
			}

			return nullptr;
		}
		XMLNode* PrevSibling()
		{
			for (auto iter = parent_->pimpl_->children_.begin(); iter != parent_->pimpl_->children_.end(); ++iter)
			{
				if (iter->pimpl_.get() == this)
				{
					if (iter != parent_->pimpl_->children_.begin())
					{
						--iter;
						return &(*iter);
					}
					break;
				}
			}

			return nullptr;
		}
		XMLNode* NextSibling()
		{
			for (auto iter = parent_->pimpl_->children_.begin(); iter != parent_->pimpl_->children_.end(); ++iter)
			{
				if (iter->pimpl_.get() == this)
				{
					++iter;
					if (iter != parent_->pimpl_->children_.end())
					{
						return &(*iter);
					}
					break;
				}
			}

			return nullptr;
		}

		uint32_t FindChildNodeIndex(XMLNode const& node)
		{
			for (size_t i = 0; i < children_.size(); ++i)
			{
				if (&children_[i] == &node)
				{
					return static_cast<uint32_t>(i);
				}
			}

			return ~0U;
		}

		void InsertAt(XMLNode& host_node, uint32_t index, XMLNode new_node)
		{
			new_node.Parent(&host_node);
			children_.insert(children_.begin() + index, std::move(new_node));
		}

		void InsertAfterNode(XMLNode& host_node, XMLNode const& location, XMLNode new_node)
		{
			for (auto iter = children_.begin(); iter != children_.end(); ++iter)
			{
				if (&(*iter) == &location)
				{
					new_node.Parent(&host_node);
					children_.emplace(iter, std::move(new_node));
					break;
				}
			}
		}
		void InsertAfterAttrib(XMLNode& host_node, XMLAttribute const& location, XMLAttribute new_attr)
		{
			for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
			{
				if (&(*iter) == &location)
				{
					new_attr.Parent(&host_node);
					attrs_.emplace(iter, std::move(new_attr));
					break;
				}
			}
		}
		void AppendNode(XMLNode& host_node, XMLNode new_node)
		{
			new_node.Parent(&host_node);
			children_.emplace_back(std::move(new_node));
		}
		void AppendAttrib(XMLNode& host_node, XMLAttribute new_attr)
		{
			new_attr.Parent(&host_node);
			attrs_.emplace_back(std::move(new_attr));
		}

		void RemoveNode(XMLNode const& node)
		{
			for (auto iter = children_.begin(); iter != children_.end(); ++iter)
			{
				if (&(*iter) == &node)
				{
					iter->Parent(nullptr);
					children_.erase(iter);
					break;
				}
			}
		}
		void RemoveAttrib(XMLAttribute const& attr)
		{
			for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
			{
				if (&(*iter) == &attr)
				{
					iter->Parent(nullptr);
					attrs_.erase(iter);
					break;
				}
			}
		}

		void ClearChildren()
		{
			for (auto iter = children_.begin(); iter != children_.end(); ++iter)
			{
				iter->Parent(nullptr);
			}
			children_.clear();
		}
		void ClearAttribs()
		{
			for (auto iter = attrs_.begin(); iter != attrs_.end(); ++iter)
			{
				iter->Parent(nullptr);
			}
			attrs_.clear();
		}

		std::string const& ValueString() const
		{
			return value_;
		}

		void Value(std::string_view value)
		{
			value_ = std::move(value);
		}

	private:
		XMLNode* parent_{};

		XMLNodeType type_;
		std::string name_;
		std::string value_;

		std::vector<XMLNode> children_;
		std::vector<XMLAttribute> attrs_;
	};

	XMLNode::XMLNode(XMLNodeType type, std::string_view name) : pimpl_(MakeUniquePtr<Impl>(type, std::move(name)))
	{
	}

	XMLNode::XMLNode(XMLNode const& rhs)
	{
		pimpl_ = MakeUniquePtr<Impl>(*rhs.pimpl_);
		pimpl_->UpdateParent(*this);
	}

	XMLNode::XMLNode(XMLNode&& rhs) noexcept : pimpl_(std::move(rhs.pimpl_))
	{
		pimpl_->UpdateParent(*this);
	}

	XMLNode::~XMLNode() noexcept = default;

	XMLNode& XMLNode::operator=(XMLNode const& rhs)
	{
		if (this != &rhs)
		{
			pimpl_ = MakeUniquePtr<Impl>(*rhs.pimpl_);
			pimpl_->UpdateParent(*this);
		}
		return *this;
	}

	XMLNode& XMLNode::operator=(XMLNode&& rhs) noexcept
	{
		if (this != &rhs)
		{
			pimpl_ = std::move(rhs.pimpl_);
			pimpl_->UpdateParent(*this);
		}
		return *this;
	}

	std::string_view XMLNode::Name() const noexcept
	{
		return pimpl_->Name();
	}

	void XMLNode::Name(std::string_view name) noexcept
	{
		pimpl_->Name(std::move(name));
	}

	XMLNodeType XMLNode::Type() const noexcept
	{
		return pimpl_->Type();
	}

	XMLNode const* XMLNode::Parent() const noexcept
	{
		return const_cast<XMLNode*>(this)->Parent();
	}

	XMLNode* XMLNode::Parent() noexcept
	{
		return pimpl_->Parent();
	}

	void XMLNode::Parent(XMLNode* parent) noexcept
	{
		pimpl_->Parent(parent);
	}

	XMLAttribute const* XMLNode::FirstAttrib(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->FirstAttrib(std::move(name));
	}

	XMLAttribute* XMLNode::FirstAttrib(std::string_view name)
	{
		return pimpl_->FirstAttrib(std::move(name));
	}

	XMLAttribute const* XMLNode::NextAttrib(XMLAttribute const& attrib, std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->NextAttrib(attrib, std::move(name));
	}

	XMLAttribute* XMLNode::NextAttrib(XMLAttribute const& attrib, std::string_view name)
	{
		return pimpl_->NextAttrib(attrib, std::move(name));
	}

	XMLAttribute const* XMLNode::LastAttrib(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->LastAttrib(std::move(name));
	}

	XMLAttribute* XMLNode::LastAttrib(std::string_view name)
	{
		return pimpl_->LastAttrib(std::move(name));
	}

	XMLAttribute const* XMLNode::FirstAttrib() const
	{
		return const_cast<XMLNode*>(this)->FirstAttrib();
	}

	XMLAttribute* XMLNode::FirstAttrib()
	{
		return pimpl_->FirstAttrib();
	}

	XMLAttribute const* XMLNode::NextAttrib(XMLAttribute const& attrib) const
	{
		return const_cast<XMLNode*>(this)->NextAttrib(attrib);
	}

	XMLAttribute* XMLNode::NextAttrib(XMLAttribute const& attrib)
	{
		return pimpl_->NextAttrib(attrib);
	}

	XMLAttribute const* XMLNode::LastAttrib() const
	{
		return const_cast<XMLNode*>(this)->LastAttrib();
	}

	XMLAttribute* XMLNode::LastAttrib()
	{
		return pimpl_->LastAttrib();
	}

	XMLAttribute const* XMLNode::Attrib(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->Attrib(std::move(name));
	}

	XMLAttribute* XMLNode::Attrib(std::string_view name)
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

	XMLNode const* XMLNode::FirstNode(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->FirstNode(std::move(name));
	}

	XMLNode* XMLNode::FirstNode(std::string_view name)
	{
		return pimpl_->FirstNode(std::move(name));
	}

	XMLNode const* XMLNode::LastNode(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->LastNode(std::move(name));
	}

	XMLNode* XMLNode::LastNode(std::string_view name)
	{
		return pimpl_->LastNode(std::move(name));
	}

	XMLNode const* XMLNode::FirstNode() const
	{
		return const_cast<XMLNode*>(this)->FirstNode();
	}

	XMLNode* XMLNode::FirstNode()
	{
		return pimpl_->FirstNode();
	}

	XMLNode const* XMLNode::LastNode() const
	{
		return const_cast<XMLNode*>(this)->LastNode();
	}

	XMLNode* XMLNode::LastNode()
	{
		return pimpl_->LastNode();
	}

	XMLNode const* XMLNode::PrevSibling(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->PrevSibling(std::move(name));
	}

	XMLNode* XMLNode::PrevSibling(std::string_view name)
	{
		return pimpl_->PrevSibling(std::move(name));
	}

	XMLNode const* XMLNode::NextSibling(std::string_view name) const
	{
		return const_cast<XMLNode*>(this)->NextSibling(std::move(name));
	}

	XMLNode* XMLNode::NextSibling(std::string_view name)
	{
		return pimpl_->NextSibling(std::move(name));
	}

	XMLNode const* XMLNode::PrevSibling() const
	{
		return const_cast<XMLNode*>(this)->PrevSibling();
	}

	XMLNode* XMLNode::PrevSibling()
	{
		return pimpl_->PrevSibling();
	}

	XMLNode const* XMLNode::NextSibling() const
	{
		return const_cast<XMLNode*>(this)->NextSibling();
	}

	XMLNode* XMLNode::NextSibling()
	{
		return pimpl_->NextSibling();
	}

	uint32_t XMLNode::FindChildNodeIndex(XMLNode const& node) const
	{
		return pimpl_->FindChildNodeIndex(node);
	}

	void XMLNode::InsertAt(uint32_t index, XMLNode new_node)
	{
		pimpl_->InsertAt(*this, index, std::move(new_node));
	}

	void XMLNode::InsertAfterNode(XMLNode const& location, XMLNode new_node)
	{
		pimpl_->InsertAfterNode(*this, location, std::move(new_node));
	}

	void XMLNode::InsertAfterAttrib(XMLAttribute const& location, XMLAttribute new_attr)
	{
		pimpl_->InsertAfterAttrib(*this, location, std::move(new_attr));
	}

	void XMLNode::AppendNode(XMLNode new_node)
	{
		pimpl_->AppendNode(*this, std::move(new_node));
	}

	void XMLNode::AppendAttrib(XMLAttribute new_attr)
	{
		pimpl_->AppendAttrib(*this, std::move(new_attr));
	}

	void XMLNode::RemoveNode(XMLNode const& node)
	{
		pimpl_->RemoveNode(node);
	}

	void XMLNode::RemoveAttrib(XMLAttribute const& attr)
	{
		pimpl_->RemoveAttrib(attr);
	}

	void XMLNode::ClearChildren()
	{
		pimpl_->ClearChildren();
	}

	void XMLNode::ClearAttribs()
	{
		pimpl_->ClearAttribs();
	}

	bool XMLNode::TryConvertValue(bool& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLNode::TryConvertValue(int32_t& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLNode::TryConvertValue(uint32_t& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLNode::TryConvertValue(float& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
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
		return pimpl_->ValueString();
	}

	void XMLNode::Value(bool value)
	{
		pimpl_->Value(value ? "true" : "false");
	}

	void XMLNode::Value(int32_t value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLNode::Value(uint32_t value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLNode::Value(float value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLNode::Value(std::string_view value)
	{
		pimpl_->Value(std::move(value));
	}


	class XMLAttribute::Impl final
	{
	public:
		explicit Impl(std::string_view name)
			: name_(std::move(name))
		{
		}

		std::string_view Name() const noexcept
		{
			return name_;
		}
		void Name(std::string_view name) noexcept
		{
			name_ = std::move(name);
		}

		XMLNode* Parent() noexcept
		{
			return parent_;
		}
		void Parent(XMLNode* parent) noexcept
		{
			parent_ = parent;
		}

		std::string const& ValueString() const
		{
			return value_;
		}

		void Value(std::string_view value)
		{
			value_ = std::move(value);
		}

	private:
		XMLNode* parent_{};

		std::string name_;
		std::string value_;
	};


	XMLAttribute::XMLAttribute(std::string_view name)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
	}
	XMLAttribute::XMLAttribute(std::string_view name, bool value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
		this->Value(value);
	}
	XMLAttribute::XMLAttribute(std::string_view name, int32_t value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
		this->Value(value);
	}
	XMLAttribute::XMLAttribute(std::string_view name, uint32_t value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
		this->Value(value);
	}
	XMLAttribute::XMLAttribute(std::string_view name, float value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
		this->Value(value);
	}
	XMLAttribute::XMLAttribute(std::string_view name, std::string_view value)
		: pimpl_(MakeUniquePtr<Impl>(std::move(name)))
	{
		pimpl_->Value(std::move(value));
	}

	XMLAttribute::XMLAttribute(XMLAttribute const& rhs)
		: pimpl_(MakeUniquePtr<Impl>(*rhs.pimpl_))
	{
	}

	XMLAttribute::XMLAttribute(XMLAttribute&& rhs) noexcept = default;

	XMLAttribute::~XMLAttribute() noexcept = default;

	XMLAttribute& XMLAttribute::operator=(XMLAttribute const& rhs)
	{
		if (this != &rhs)
		{
			pimpl_ = MakeUniquePtr<Impl>(*rhs.pimpl_);
		}
		return *this;
	}

	XMLAttribute& XMLAttribute::operator=(XMLAttribute&& rhs) noexcept = default;

	std::string_view XMLAttribute::Name() const noexcept
	{
		return pimpl_->Name();
	}

	void XMLAttribute::Name(std::string_view name) noexcept
	{
		pimpl_->Name(std::move(name));
	}

	XMLNode const* XMLAttribute::Parent() const noexcept
	{
		return const_cast<XMLAttribute*>(this)->Parent();
	}

	XMLNode* XMLAttribute::Parent() noexcept
	{
		return pimpl_->Parent();
	}

	void XMLAttribute::Parent(XMLNode* parent) noexcept
	{
		pimpl_->Parent(parent);
	}

	XMLAttribute const* XMLAttribute::NextAttrib(std::string_view name) const
	{
		return const_cast<XMLAttribute*>(this)->NextAttrib(std::move(name));
	}

	XMLAttribute* XMLAttribute::NextAttrib(std::string_view name)
	{
		return pimpl_->Parent()->NextAttrib(*this, std::move(name));
	}

	XMLAttribute const* XMLAttribute::NextAttrib() const
	{
		return const_cast<XMLAttribute*>(this)->NextAttrib();
	}

	XMLAttribute* XMLAttribute::NextAttrib()
	{
		return pimpl_->Parent()->NextAttrib(*this);
	}

	bool XMLAttribute::TryConvertValue(bool& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLAttribute::TryConvertValue(int32_t& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLAttribute::TryConvertValue(uint32_t& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLAttribute::TryConvertValue(float& val) const
	{
		return TryConvertStringToValue(pimpl_->ValueString(), val);
	}

	bool XMLAttribute::ValueBool() const
	{
		bool val = false;
		this->TryConvertValue(val);
		return val;
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
		return pimpl_->ValueString();
	}

	void XMLAttribute::Value(bool value)
	{
		pimpl_->Value(value ? "true" : "false");
	}

	void XMLAttribute::Value(int32_t value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLAttribute::Value(uint32_t value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLAttribute::Value(float value)
	{
		pimpl_->Value(std::to_string(value));
	}

	void XMLAttribute::Value(std::string_view value)
	{
		pimpl_->Value(std::move(value));
	}


	XMLNode LoadXml(ResIdentifier& source)
	{
		source.seekg(0, std::ios_base::end);
		size_t const len = static_cast<size_t>(source.tellg());
		source.seekg(0, std::ios_base::beg);
		auto xml_src = MakeUniquePtr<char[]>(len + 1);
		source.read(&xml_src[0], len);
		xml_src[len] = 0;

		rapidxml::xml_document<char> doc;
		doc.parse<0>(xml_src.get());

		return CreateXmlNodeFromRapidXmlNode(*doc.first_node());
	}

	void SaveXml(XMLNode const& node, std::ostream& os)
	{
		rapidxml::xml_document<char> doc;
		rapidxml::xml_node<char>* root_node = CreateRapidXmlNodeFromXmlNode(doc, node);
		doc.append_node(root_node);

		os << "<?xml version=\"1.0\"?>" << std::endl << std::endl;
		os << doc;
	}
} // namespace KlayGE
