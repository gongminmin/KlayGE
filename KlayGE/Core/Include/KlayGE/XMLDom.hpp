// XMLDom.hpp
// KlayGE XML DOM解析器 头文件
// Ver 3.9.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.9.0
// 初次建立 (2009.5.2)
//
// 修改记录
///////////////////////////////////////////////////////////////////////////////

#ifndef _XMLDOM_HPP
#define _XMLDOM_HPP

#pragma once

#ifndef KLAYGE_CORE_SOURCE
#define KLAYGE_LIB_NAME KlayGE_Core
#include <KlayGE/config/auto_link.hpp>
#endif

#include <iosfwd>
#include <vector>

#include <rapidxml/rapidxml.hpp>

namespace KlayGE
{
	enum XMLNodeType
	{
		XNT_Document,
		XNT_Element,
		XNT_Data,
		XNT_CData,
		XNT_Comment,
		XNT_Declaration,
		XNT_Doctype,
		XNT_PI
	};

	class KLAYGE_CORE_API XMLDocument
	{
	public:
		XMLNodePtr Parse(ResIdentifierPtr const & source);
		void Print(std::ostream& os);

		XMLNodePtr CloneNode(XMLNodePtr const & node);

		XMLNodePtr AllocNode(XMLNodeType type, std::string const & name);
		XMLAttributePtr AllocAttribInt(std::string const & name, int32_t value);
		XMLAttributePtr AllocAttribUInt(std::string const & name, uint32_t value);
		XMLAttributePtr AllocAttribFloat(std::string const & name, float value);
		XMLAttributePtr AllocAttribString(std::string const & name, std::string const & value);

		void RootNode(XMLNodePtr const & new_node);

	private:
		rapidxml::xml_document<> doc_;
		std::vector<char> xml_src_;

		XMLNodePtr root_;
	};

	class KLAYGE_CORE_API XMLNode
	{
		friend class XMLDocument;

	public:
		explicit XMLNode(rapidxml::xml_node<>* node);
		XMLNode(rapidxml::xml_document<>* doc, XMLNodeType type, std::string const & name);

		std::string const & Name() const;
		XMLNodeType Type() const;

		XMLNodePtr Parent();

		XMLAttributePtr FirstAttrib(std::string const & name);
		XMLAttributePtr LastAttrib(std::string const & name);
		XMLAttributePtr FirstAttrib();
		XMLAttributePtr LastAttrib();

		XMLAttributePtr Attrib(std::string const & name);

		XMLNodePtr FirstNode(std::string const & name);
		XMLNodePtr LastNode(std::string const & name);
		XMLNodePtr FirstNode();
		XMLNodePtr LastNode();

		XMLNodePtr PrevSibling(std::string const & name);
		XMLNodePtr NextSibling(std::string const & name);
		XMLNodePtr PrevSibling();
		XMLNodePtr NextSibling();

		void InsertNode(XMLNodePtr const & location, XMLNodePtr const & new_node);
		void InsertAttrib(XMLAttributePtr const & location, XMLAttributePtr const & new_attr);
		void AppendNode(XMLNodePtr const & new_node);
		void AppendAttrib(XMLAttributePtr const & new_attr);

		void RemoveNode(XMLNodePtr const & node);
		void RemoveAttrib(XMLAttributePtr const & attr);

		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string ValueString() const;

	private:
		rapidxml::xml_node<>* node_;
		std::string name_;

		std::vector<XMLNodePtr> children_;
		std::vector<XMLAttributePtr> attrs_;
	};

	class KLAYGE_CORE_API XMLAttribute
	{
		friend class XMLDocument;
		friend class XMLNode;

	public:
		explicit XMLAttribute(rapidxml::xml_attribute<>* attr);
		XMLAttribute(rapidxml::xml_document<>* doc, std::string const & name, std::string const & value);

		std::string const & Name() const;

		XMLAttributePtr NextAttrib(std::string const & name);
		XMLAttributePtr NextAttrib();

		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string const & ValueString() const;

	private:
		rapidxml::xml_attribute<>* attr_;
		std::string name_;
		std::string value_;
	};
}

#endif		// _XMLDOM_HPP
