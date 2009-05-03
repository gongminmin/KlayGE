// XMLDom.cpp
// KlayGE XML DOM解析器 实现文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 初次建立 (2009.5.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Context.hpp>

#include <sstream>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
#include <boost/lexical_cast.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4100)
#endif
#include <rapidxml/rapidxml_print.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/XMLDom.hpp>

namespace KlayGE
{
	XMLNodePtr XMLDocument::Parse(ResIdentifierPtr const & source)
	{
		source->seekg(0, std::ios_base::end);
		int len = static_cast<int>(source->tellg());
		source->seekg(0, std::ios_base::beg);
		xml_src_.resize(len + 1, 0);
		source->read(&xml_src_[0], len);

		doc_.parse<0>(&xml_src_[0]);
		root_ = MakeSharedPtr<XMLNode>(doc_.first_node());

		return root_;
	}

	void XMLDocument::Print(std::ostream& os)
	{
		os << "<?xml version=\"1.0\"?>" << std::endl << std::endl;
		os << doc_;
	}

	XMLNodePtr XMLDocument::CloneNode(XMLNodePtr const & node)
	{
		return MakeSharedPtr<XMLNode>(doc_.clone_node(node->node_));
	}

	XMLNodePtr XMLDocument::AllocNode(XMLNodeType type, std::string const & name)
	{
		return MakeSharedPtr<XMLNode>(&doc_, type, name);
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
		return MakeSharedPtr<XMLAttribute>(&doc_, name, value);
	}

	void XMLDocument::RootNode(XMLNodePtr const & new_node)
	{
		doc_.remove_all_nodes();
		doc_.append_node(new_node->node_);
		root_ = new_node;
	}


	XMLNode::XMLNode(rapidxml::xml_node<>* node)
		: node_(node)
	{
		if (node_ != NULL)
		{
			name_ = std::string(node_->name(), node_->name_size());
		}
	}

	XMLNode::XMLNode(rapidxml::xml_document<>* doc, XMLNodeType type, std::string const & name)
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

		node_ = doc->allocate_node(xtype, name_.c_str());
	}

	std::string const & XMLNode::Name() const
	{
		return name_;
	}

	XMLNodeType XMLNode::Type() const
	{
		switch (node_->type())
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

	XMLNodePtr XMLNode::Parent()
	{
		rapidxml::xml_node<>* node = node_->parent();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLAttributePtr XMLNode::FirstAttrib(std::string const & name)
	{
		rapidxml::xml_attribute<>* attr = node_->first_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}
	
	XMLAttributePtr XMLNode::LastAttrib(std::string const & name)
	{
		rapidxml::xml_attribute<>* attr = node_->last_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::FirstAttrib()
	{
		rapidxml::xml_attribute<>* attr = node_->first_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::LastAttrib()
	{
		rapidxml::xml_attribute<>* attr = node_->last_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLNode::Attrib(std::string const & name)
	{
		return this->FirstAttrib(name);
	}

	XMLNodePtr XMLNode::FirstNode(std::string const & name)
	{
		rapidxml::xml_node<>* node = node_->first_node(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::LastNode(std::string const & name)
	{
		rapidxml::xml_node<>* node = node_->last_node(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::FirstNode()
	{
		rapidxml::xml_node<>* node = node_->first_node();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::LastNode()
	{
		rapidxml::xml_node<>* node = node_->last_node();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::PrevSibling(std::string const & name)
	{
		rapidxml::xml_node<>* node = node_->previous_sibling(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::NextSibling(std::string const & name)
	{
		rapidxml::xml_node<>* node = node_->next_sibling(name.c_str());
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::PrevSibling()
	{
		rapidxml::xml_node<>* node = node_->previous_sibling();
		if (node)
		{
			return MakeSharedPtr<XMLNode>(node);
		}
		else
		{
			return XMLNodePtr();
		}
	}

	XMLNodePtr XMLNode::NextSibling()
	{
		rapidxml::xml_node<>* node = node_->next_sibling();
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
		node_->insert_node(location->node_, new_node->node_);
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
		node_->insert_attribute(location->attr_, new_attr->attr_);
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
		node_->append_node(new_node->node_);
		children_.push_back(new_node);
	}

	void XMLNode::AppendAttrib(XMLAttributePtr const & new_attr)
	{
		node_->append_attribute(new_attr->attr_);
		attrs_.push_back(new_attr);
	}

	void XMLNode::RemoveNode(XMLNodePtr const & node)
	{
		node_->remove_node(node->node_);
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
		node_->remove_attribute(attr->attr_);
		for (size_t i = 0; i < attrs_.size(); ++ i)
		{
			if (attrs_[i]->attr_ == attr->attr_)
			{
				attrs_.erase(attrs_.begin() + i);
				break;
			}
		}
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
		return std::string(node_->value(), node_->value_size());
	}


	XMLAttribute::XMLAttribute(rapidxml::xml_attribute<>* attr)
		: attr_(attr)
	{
		if (attr_ != NULL)
		{
			name_ = std::string(attr_->name(), attr_->name_size());
			value_ = std::string(attr_->value(), attr_->value_size());
		}
	}

	XMLAttribute::XMLAttribute(rapidxml::xml_document<>* doc, std::string const & name, std::string const & value)
		: name_(name), value_(value)
	{
		attr_ = doc->allocate_attribute(name_.c_str(), value_.c_str());
	}

	std::string const & XMLAttribute::Name() const
	{
		return name_;
	}

	XMLAttributePtr XMLAttribute::NextAttrib(std::string const & name)
	{
		rapidxml::xml_attribute<>* attr = attr_->next_attribute(name.c_str());
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
	}

	XMLAttributePtr XMLAttribute::NextAttrib()
	{
		rapidxml::xml_attribute<>* attr = attr_->next_attribute();
		if (attr)
		{
			return MakeSharedPtr<XMLAttribute>(attr);
		}
		else
		{
			return XMLAttributePtr();
		}
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
