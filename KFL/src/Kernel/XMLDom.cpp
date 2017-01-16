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
#include <KFL/Util.hpp>
#include <KFL/ResIdentifier.hpp>

#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable" // Ignore mpl_assertion_in_line_xxx
#endif
#include <boost/lexical_cast.hpp>
#if defined(KLAYGE_COMPILER_CLANGC2)
#pragma clang diagnostic pop
#endif

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

#include <KFL/XMLDom.hpp>

namespace KlayGE
{
	XMLDocument::XMLDocument()
		: doc_(MakeSharedPtr<rapidxml::xml_document<>>())
	{
	}

	XMLNodePtr XMLDocument::Parse(ResIdentifierPtr const & source)
	{
		source->seekg(0, std::ios_base::end);
		int len = static_cast<int>(source->tellg());
		source->seekg(0, std::ios_base::beg);
		xml_src_.resize(len + 1, 0);
		source->read(&xml_src_[0], len);

		static_cast<rapidxml::xml_document<>*>(doc_.get())->parse<0>(&xml_src_[0]);
		root_ = MakeSharedPtr<XMLNode>(static_cast<rapidxml::xml_document<>*>(doc_.get())->first_node());

		return root_;
	}

	void XMLDocument::Print(std::ostream& os)
	{
		os << "<?xml version=\"1.0\"?>" << std::endl << std::endl;
		os << *static_cast<rapidxml::xml_document<>*>(doc_.get());
	}

	XMLNodePtr XMLDocument::CloneNode(XMLNodePtr const & node)
	{
		return MakeSharedPtr<XMLNode>(static_cast<rapidxml::xml_document<>*>(doc_.get())->clone_node(static_cast<rapidxml::xml_node<>*>(node->node_)));
	}

	XMLNodePtr XMLDocument::AllocNode(XMLNodeType type, std::string const & name)
	{
		return MakeSharedPtr<XMLNode>(doc_.get(), type, name);
	}
	
	XMLAttributePtr XMLDocument::AllocAttribInt(std::string const & name, int32_t value)
	{
		return this->AllocAttribString(name, boost::lexical_cast<std::string>(value));
	}

	XMLAttributePtr XMLDocument::AllocAttribUInt(std::string const & name, uint32_t value)
	{
		return this->AllocAttribString(name, boost::lexical_cast<std::string>(value));
	}

	XMLAttributePtr XMLDocument::AllocAttribFloat(std::string const & name, float value)
	{
		return this->AllocAttribString(name, boost::lexical_cast<std::string>(value));
	}

	XMLAttributePtr XMLDocument::AllocAttribString(std::string const & name, std::string const & value)
	{
		return MakeSharedPtr<XMLAttribute>(doc_.get(), name, value);
	}

	void XMLDocument::RootNode(XMLNodePtr const & new_node)
	{
		static_cast<rapidxml::xml_document<>*>(doc_.get())->remove_all_nodes();
		static_cast<rapidxml::xml_document<>*>(doc_.get())->append_node(static_cast<rapidxml::xml_node<>*>(new_node->node_));
		root_ = new_node;
	}


	XMLNode::XMLNode(void* node)
		: node_(node)
	{
		if (node_ != nullptr)
		{
			name_ = std::string(static_cast<rapidxml::xml_node<>*>(node_)->name(),
				static_cast<rapidxml::xml_node<>*>(node_)->name_size());
		}
	}

	XMLNode::XMLNode(void* doc, XMLNodeType type, std::string const & name)
		: name_(name)
	{
		rapidxml::node_type xtype;
		switch (type)
		{
		case XNT_Document:
			xtype = rapidxml::node_document;
			break;

		case XNT_Element:
			xtype = rapidxml::node_element;
			break;

		case XNT_Data:
			xtype = rapidxml::node_data;
			break;

		case XNT_CData:
			xtype = rapidxml::node_cdata;
			break;

		case XNT_Comment:
			xtype = rapidxml::node_comment;
			break;

		case XNT_Declaration:
			xtype = rapidxml::node_declaration;
			break;

		case XNT_Doctype:
			xtype = rapidxml::node_doctype;
			break;

		case XNT_PI:
		default:
			xtype = rapidxml::node_pi;
			break;
		}

		node_ = static_cast<rapidxml::xml_document<>*>(doc)->allocate_node(xtype, name_.c_str());
	}

	std::string const & XMLNode::Name() const
	{
		return name_;
	}

	XMLNodeType XMLNode::Type() const
	{
		switch (static_cast<rapidxml::xml_node<>*>(node_)->type())
		{
		case rapidxml::node_document:
			return XNT_Document;

		case rapidxml::node_element:
			return XNT_Element;

		case rapidxml::node_data:
			return XNT_Data;

		case rapidxml::node_cdata:
			return XNT_CData;

		case rapidxml::node_comment:
			return XNT_Comment;

		case rapidxml::node_declaration:
			return XNT_Declaration;

		case rapidxml::node_doctype:
			return XNT_Doctype;

		case rapidxml::node_pi:
		default:
			return XNT_PI;
		}
	}

	XMLNodePtr XMLNode::Parent() const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->parent();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLAttributePtr XMLNode::FirstAttrib(std::string const & name) const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_node<>*>(node_)->first_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}
	
	XMLAttributePtr XMLNode::LastAttrib(std::string const & name) const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_node<>*>(node_)->last_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::FirstAttrib() const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_node<>*>(node_)->first_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::LastAttrib() const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_node<>*>(node_)->last_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::Attrib(std::string const & name) const
	{
		return this->FirstAttrib(name);
	}

	bool XMLNode::TryConvertAttrib(std::string const & name, int32_t& val, int32_t default_val) const
	{
		val = default_val;

		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->TryConvert(val) : true;
	}

	bool XMLNode::TryConvertAttrib(std::string const & name, uint32_t& val, uint32_t default_val) const
	{
		val = default_val;

		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->TryConvert(val) : true;
	}

	bool XMLNode::TryConvertAttrib(std::string const & name, float& val, float default_val) const
	{
		val = default_val;

		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->TryConvert(val) : true;
	}

	int32_t XMLNode::AttribInt(std::string const & name, int32_t default_val) const
	{
		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->ValueInt() : default_val;
	}

	uint32_t XMLNode::AttribUInt(std::string const & name, uint32_t default_val) const
	{
		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->ValueUInt() : default_val;
	}

	float XMLNode::AttribFloat(std::string const & name, float default_val) const
	{
		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->ValueFloat() : default_val;
	}

	std::string XMLNode::AttribString(std::string const & name, std::string default_val) const
	{
		XMLAttributePtr attr = this->Attrib(name);
		return attr ? attr->ValueString() : default_val;
	}

	XMLNodePtr XMLNode::FirstNode(std::string const & name) const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->first_node(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::LastNode(std::string const & name) const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->last_node(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::FirstNode() const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->first_node();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::LastNode() const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->last_node();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::PrevSibling(std::string const & name) const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->previous_sibling(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::NextSibling(std::string const & name) const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->next_sibling(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::PrevSibling() const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->previous_sibling();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::NextSibling() const
	{
		rapidxml::xml_node<>* node = static_cast<rapidxml::xml_node<>*>(node_)->next_sibling();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	void XMLNode::InsertNode(XMLNodePtr const & location, XMLNodePtr const & new_node)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->insert_node(static_cast<rapidxml::xml_node<>*>(location->node_),
			static_cast<rapidxml::xml_node<>*>(new_node->node_));
		for (size_t i = 0; i < children_.size(); ++ i)
		{
			if (children_[i]->node_ == location->node_)
			{
				children_.insert(children_.begin() + i, new_node);
				break;
			}
		}
	}

	void XMLNode::InsertAttrib(XMLAttributePtr const & location, XMLAttributePtr const & new_attr)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->insert_attribute(static_cast<rapidxml::xml_attribute<>*>(location->attr_),
			static_cast<rapidxml::xml_attribute<>*>(new_attr->attr_));
		for (size_t i = 0; i < attrs_.size(); ++ i)
		{
			if (attrs_[i]->attr_ == location->attr_)
			{
				attrs_.insert(attrs_.begin() + i, new_attr);
				break;
			}
		}
	}

	void XMLNode::AppendNode(XMLNodePtr const & new_node)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->append_node(static_cast<rapidxml::xml_node<>*>(new_node->node_));
		children_.push_back(new_node);
	}

	void XMLNode::AppendAttrib(XMLAttributePtr const & new_attr)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->append_attribute(static_cast<rapidxml::xml_attribute<>*>(new_attr->attr_));
		attrs_.push_back(new_attr);
	}

	void XMLNode::RemoveNode(XMLNodePtr const & node)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->remove_node(static_cast<rapidxml::xml_node<>*>(node->node_));
		for (size_t i = 0; i < children_.size(); ++ i)
		{
			if (children_[i]->node_ == node->node_)
			{
				children_.erase(children_.begin() + i);
				break;
			}
		}
	}

	void XMLNode::RemoveAttrib(XMLAttributePtr const & attr)
	{
		static_cast<rapidxml::xml_node<>*>(node_)->remove_attribute(static_cast<rapidxml::xml_attribute<>*>(attr->attr_));
		for (size_t i = 0; i < attrs_.size(); ++ i)
		{
			if (attrs_[i]->attr_ == attr->attr_)
			{
				attrs_.erase(attrs_.begin() + i);
				break;
			}
		}
	}

	bool XMLNode::TryConvert(int32_t& val) const
	{
		return boost::conversion::try_lexical_convert(this->ValueString(), val);
	}

	bool XMLNode::TryConvert(uint32_t& val) const
	{
		return boost::conversion::try_lexical_convert(this->ValueString(), val);
	}

	bool XMLNode::TryConvert(float& val) const
	{
		return boost::conversion::try_lexical_convert(this->ValueString(), val);
	}

	int32_t XMLNode::ValueInt() const
	{
		return boost::lexical_cast<int32_t>(this->ValueString());
	}

	uint32_t XMLNode::ValueUInt() const
	{
		return boost::lexical_cast<uint32_t>(this->ValueString());
	}

	float XMLNode::ValueFloat() const
	{
		return boost::lexical_cast<float>(this->ValueString());
	}

	std::string XMLNode::ValueString() const
	{
		return std::string(static_cast<rapidxml::xml_node<>*>(node_)->value(),
			static_cast<rapidxml::xml_node<>*>(node_)->value_size());
	}


	XMLAttribute::XMLAttribute(void* attr)
		: attr_(attr)
	{
		if (attr_ != nullptr)
		{
			name_ = std::string(static_cast<rapidxml::xml_attribute<>*>(attr_)->name(), static_cast<rapidxml::xml_attribute<>*>(attr_)->name_size());
			value_ = std::string(static_cast<rapidxml::xml_attribute<>*>(attr_)->value(), static_cast<rapidxml::xml_attribute<>*>(attr_)->value_size());
		}
	}

	XMLAttribute::XMLAttribute(void* doc, std::string const & name, std::string const & value)
		: name_(name), value_(value)
	{
		attr_ = static_cast<rapidxml::xml_document<>*>(doc)->allocate_attribute(name_.c_str(), value_.c_str());
	}

	std::string const & XMLAttribute::Name() const
	{
		return name_;
	}

	XMLAttributePtr XMLAttribute::NextAttrib(std::string const & name) const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_attribute<>*>(attr_)->next_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLAttribute::NextAttrib() const
	{
		rapidxml::xml_attribute<>* attr = static_cast<rapidxml::xml_attribute<>*>(attr_)->next_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	bool XMLAttribute::TryConvert(int32_t& val) const
	{
		return boost::conversion::try_lexical_convert(value_, val);
	}

	bool XMLAttribute::TryConvert(uint32_t& val) const
	{
		return boost::conversion::try_lexical_convert(value_, val);
	}

	bool XMLAttribute::TryConvert(float& val) const
	{
		return boost::conversion::try_lexical_convert(value_, val);
	}

	int32_t XMLAttribute::ValueInt() const
	{
		return boost::lexical_cast<int32_t>(value_);
	}

	uint32_t XMLAttribute::ValueUInt() const
	{
		return boost::lexical_cast<uint32_t>(value_);
	}

	float XMLAttribute::ValueFloat() const
	{
		return boost::lexical_cast<float>(value_);
	}

	std::string const & XMLAttribute::ValueString() const
	{
		return value_;
	}
}
