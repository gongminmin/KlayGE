/**
 * @file XMLDom.hpp
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

#ifndef _KFL_XMLDOM_HPP
#define _KFL_XMLDOM_HPP

#pragma once

#include <iosfwd>
#include <vector>

#include <boost/noncopyable.hpp>

namespace rapidxml
{
	template <typename Ch>
	class xml_node;
	template <typename Ch>
	class xml_attribute;
	template <typename Ch>
	class xml_document;
}

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

	class XMLDocument : boost::noncopyable
	{
	public:
		XMLDocument();

		XMLNodePtr Parse(ResIdentifierPtr const & source);
		void Print(std::ostream& os);

		XMLNodePtr CloneNode(XMLNodePtr const & node);

		XMLNodePtr AllocNode(XMLNodeType type, std::string_view name);
		XMLAttributePtr AllocAttribInt(std::string_view name, int32_t value);
		XMLAttributePtr AllocAttribUInt(std::string_view name, uint32_t value);
		XMLAttributePtr AllocAttribFloat(std::string_view name, float value);
		XMLAttributePtr AllocAttribString(std::string_view name, std::string_view value);

		void RootNode(XMLNodePtr const & new_node);

	private:
		std::shared_ptr<rapidxml::xml_document<char>> doc_;
		std::vector<char> xml_src_;

		XMLNodePtr root_;
	};

	class XMLNode : boost::noncopyable
	{
		friend class XMLDocument;

	public:
		explicit XMLNode(rapidxml::xml_node<char>* node);
		XMLNode(rapidxml::xml_document<char>& doc, XMLNodeType type, std::string_view name);

		std::string_view Name() const;
		XMLNodeType Type() const;

		XMLNodePtr Parent() const;

		XMLAttributePtr FirstAttrib(std::string_view name) const;
		XMLAttributePtr LastAttrib(std::string_view name) const;
		XMLAttributePtr FirstAttrib() const;
		XMLAttributePtr LastAttrib() const;

		XMLAttributePtr Attrib(std::string_view name) const;

		bool TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, float& val, float default_val) const;

		int32_t AttribInt(std::string_view name, int32_t default_val) const;
		uint32_t AttribUInt(std::string_view name, uint32_t default_val) const;
		float AttribFloat(std::string_view name, float default_val) const;
		std::string_view AttribString(std::string_view name, std::string_view default_val) const;

		XMLNodePtr FirstNode(std::string_view name) const;
		XMLNodePtr LastNode(std::string_view name) const;
		XMLNodePtr FirstNode() const;
		XMLNodePtr LastNode() const;

		XMLNodePtr PrevSibling(std::string_view name) const;
		XMLNodePtr NextSibling(std::string_view name) const;
		XMLNodePtr PrevSibling() const;
		XMLNodePtr NextSibling() const;

		void InsertNode(XMLNodePtr const & location, XMLNodePtr const & new_node);
		void InsertAttrib(XMLAttributePtr const & location, XMLAttributePtr const & new_attr);
		void AppendNode(XMLNodePtr const & new_node);
		void AppendAttrib(XMLAttributePtr const & new_attr);

		void RemoveNode(XMLNodePtr const & node);
		void RemoveAttrib(XMLAttributePtr const & attr);

		bool TryConvert(int32_t& val) const;
		bool TryConvert(uint32_t& val) const;
		bool TryConvert(float& val) const;

		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string_view ValueString() const;

	private:
		rapidxml::xml_node<char>* node_;
		std::string_view name_;

		std::vector<XMLNodePtr> children_;
		std::vector<XMLAttributePtr> attrs_;
	};

	class XMLAttribute : boost::noncopyable
	{
		friend class XMLDocument;
		friend class XMLNode;

	public:
		explicit XMLAttribute(rapidxml::xml_attribute<char>* attr);
		XMLAttribute(rapidxml::xml_document<char>& doc, std::string_view name, std::string_view value);

		std::string_view Name() const;

		XMLAttributePtr NextAttrib(std::string_view name) const;
		XMLAttributePtr NextAttrib() const;

		bool TryConvert(int32_t& val) const;
		bool TryConvert(uint32_t& val) const;
		bool TryConvert(float& val) const;

		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string_view ValueString() const;

	private:
		rapidxml::xml_attribute<char>* attr_;
		std::string_view name_;
		std::string_view value_;
	};
}

#endif		// _KFL_XMLDOM_HPP
