#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import struct
import sys

control_type_enum = {
	"static" : 0,
	"button" : 1,
	"check_box" : 2,
	"radio_button" : 3,
	"slider" : 4,
	"scroll_bar" : 5,
	"list_box" : 6,
	"combo_box" : 7,
	"edit_box" : 8
}

align_enum = {
	"left" : 0,
	"right" : 1,
	"center" : 2,
	"top" : 3,
	"bottom" : 4,
	"middle" : 5
}

list_box_style = {
	"signed" : 0,
	"multi" : 1
}

ctrl_id_list = []

def write_short_string(stream, str):
	stream.write(struct.pack('B', len(str)))
	if 3 == sys.version_info[0]:
		stream.write(bytes(str, encoding = 'ascii'))
	else:
		stream.write(str)
		
class ui_control:
	def __init__(self, tag):
		self.type = control_type_enum[tag.getAttribute('type')]
		id_str = tag.getAttribute('id')
		for i, id in enumerate(ctrl_id_list):
			if id == id_str:
				self.id = i
				break
		else:
			self.id = len(ctrl_id_list)
			ctrl_id_list.append(id_str)
		self.x = int(tag.getAttribute('x'))
		self.y = int(tag.getAttribute('y'))
		self.width = int(tag.getAttribute('width'))
		self.height = int(tag.getAttribute('height'))
		try:
			self.is_default = bool(int(tag.getAttribute('is_default')))
		except:
			self.is_default = False
		try:
			self.align_x = align_enum[tag.getAttribute('align_x')]
		except:
			self.align_x = align_enum["left"]
		try:
			self.align_y = align_enum[tag.getAttribute('align_y')]
		except:
			self.align_y = align_enum["top"]
			
	def write(self, stream):
		stream.write(struct.pack('I', self.type))
		stream.write(struct.pack('I', self.id))
		stream.write(struct.pack('i', self.x))
		stream.write(struct.pack('i', self.y))
		stream.write(struct.pack('I', self.width))
		stream.write(struct.pack('I', self.height))
		stream.write(struct.pack('B', self.is_default))
		stream.write(struct.pack('B', self.align_x))
		stream.write(struct.pack('B', self.align_y))

class ui_static(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		self.caption = tag.getAttribute('caption')

	def write(self, stream):
		ui_control.write(self, stream)
		write_short_string(stream, self.caption)

class ui_button(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		self.caption = tag.getAttribute('caption')
		try:
			self.hotkey = int(tag.getAttribute('hotkey'))
		except:
			self.hotkey = 0

	def write(self, stream):
		ui_control.write(self, stream)
		write_short_string(stream, self.caption)
		stream.write(struct.pack('I', self.hotkey))

class ui_check_box(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		self.caption = tag.getAttribute('caption')
		try:
			self.checked = bool(int(tag.getAttribute('checked')))
		except:
			self.checked = False
		try:
			self.hotkey = int(tag.getAttribute('hotkey'))
		except:
			self.hotkey = 0

	def write(self, stream):
		ui_control.write(self, stream)
		write_short_string(stream, self.caption)
		stream.write(struct.pack('B', self.checked))
		stream.write(struct.pack('I', self.hotkey))

class ui_radio_button(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		self.button_group = tag.getAttribute('button_group')
		self.caption = tag.getAttribute('caption')
		try:
			self.checked = bool(int(tag.getAttribute('checked')))
		except:
			self.checked = False
		try:
			self.hotkey = int(tag.getAttribute('hotkey'))
		except:
			self.hotkey = 0

	def write(self, stream):
		ui_control.write(self, stream)
		write_short_string(stream, self.caption)
		stream.write(struct.pack('i', self.button_group))
		stream.write(struct.pack('B', self.checked))
		stream.write(struct.pack('I', self.hotkey))

class ui_slider(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		try:
			self.min = int(tag.getAttribute('min'))
		except:
			self.min = 0
		try:
			self.max = int(tag.getAttribute('max'))
		except:
			self.max = 100
		try:
			self.value = int(tag.getAttribute('value'))
		except:
			self.value = 50

	def write(self, stream):
		ui_control.write(self, stream)
		stream.write(struct.pack('i', self.min))
		stream.write(struct.pack('i', self.max))
		stream.write(struct.pack('i', self.value))

class ui_scroll_bar(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		try:
			self.track_start = int(tag.getAttribute('track_start'))
		except:
			self.track_start = 0
		try:
			self.track_end = int(tag.getAttribute('track_end'))
		except:
			self.track_end = 1
		try:
			self.track_pos = int(tag.getAttribute('track_pos'))
		except:
			self.track_pos = 1
		try:
			self.page_size = int(tag.getAttribute('page_size'))
		except:
			self.page_size = 1

	def write(self, stream):
		ui_control.write(self, stream)
		stream.write(struct.pack('i', self.track_start))
		stream.write(struct.pack('i', self.track_end))
		stream.write(struct.pack('i', self.track_pos))
		stream.write(struct.pack('i', self.page_size))

class ui_list_box(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		try:
			self.style = list_box_style[tag.getAttribute('style')]
		except:
			self.style = list_box_style['signed']

		self.items = []
		items = tag.getElementsByTagName('item')
		for item_tag in items:
			self.items.append(item_tag.getAttribute('name'))

	def write(self, stream):
		ui_control.write(self, stream)
		stream.write(struct.pack('B', self.style))
		stream.write(struct.pack('I', len(self.items)))
		for item in self.items:
			write_short_string(stream, item)

class ui_combo_box(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		try:
			self.hotkey = int(tag.getAttribute('hotkey'))
		except:
			self.hotkey = 0

		self.items = []
		items = tag.getElementsByTagName('item')
		for item_tag in items:
			self.items.append(item_tag.getAttribute('name'))

	def write(self, stream):
		ui_control.write(self, stream)
		stream.write(struct.pack('I', self.hotkey))
		stream.write(struct.pack('I', len(self.items)))
		for item in self.items:
			write_short_string(stream, item)

class ui_edit_box(ui_control):
	def __init__(self, tag):
		ui_control.__init__(self, tag)
		self.caption = tag.getAttribute('caption')

	def write(self, stream):
		ui_control.write(self, stream)
		write_short_string(stream, self.caption)

class dialog:
	def __init__(self, tag):
		try:
			self.caption = tag.getAttribute('caption')
		except:
			self.caption = ''
		try:
			self.skin = tag.getAttribute('skin')
		except:
			self.skin = ''

		self.controls = []
		control_tags = tag.getElementsByTagName('control')
		for control_tag in control_tags:
			t = control_tag.getAttribute('type')
			eval("self.controls.append(ui_%s(control_tag))" % t)

	def write(self, stream):
		write_short_string(stream, self.caption)
		write_short_string(stream, self.skin)
		stream.write(struct.pack('I', len(ctrl_id_list)))
		for id in ctrl_id_list:
			write_short_string(stream, id)
		stream.write(struct.pack('I', len(self.controls)))
		for control in self.controls:
			control.write(stream)

class ui_cfg:
	def __init__(self, tag):
		self.dialogs = []
		dialog_tags = tag.getElementsByTagName('dialog')
		for dialog_tag in dialog_tags:
			self.dialogs.append(dialog(dialog_tag))

	def write(self, stream):
		if 3 == sys.version_info[0]:
			stream.write(b'UIML')
		else:
			stream.write('UIML')
		stream.write(struct.pack('I', 1))
		stream.write(struct.pack('I', len(self.dialogs)))

		for dlg in self.dialogs:
			dlg.write(stream)

def preprocess(file_name):
	ret = ''

	dir = ''
	last_slash = file_name.rfind('/')
	if last_slash != -1:
		dir = file_name[0 : last_slash + 1]

	import xml.dom
	
	from xml.dom.minidom import parse
	dom = parse(file_name)
	
	from sys import	getfilesystemencoding
	encoding = getfilesystemencoding()

	include_files = dom.documentElement.getElementsByTagName('include')
	while len(include_files) != 0:
		for include_file in include_files:
			if 3 == sys.version_info[0]:
				include_name = include_file.getAttribute('name')
			else:
				include_name = include_file.getAttribute('name').encode(encoding)
			if len(include_name) != 0:
				include_dom = parse(dir + include_name)
				for child in include_dom.documentElement.childNodes:
					if xml.dom.Node.ELEMENT_NODE == child.nodeType:
						dom.documentElement.insertBefore(child, include_file)
			dom.documentElement.removeChild(include_file)

		include_files = dom.documentElement.getElementsByTagName('include')

	return dom
			

if __name__ == '__main__':
	input_name = 'sample.uiml'
	output_name = 'output.kui'

	import sys
	if len(sys.argv) >= 2:
		input_name = sys.argv[1]
		if len(sys.argv) < 3:
			for i in range(0, len(input_name) - 1):
				if input_name[len(input_name) - 1 - i] == '.':
					output_name = input_name[0 : len(input_name) - 1 - i] + ".kui"
		else:
			output_name = sys.argv[2]

	input_name = input_name.replace('\\', '/')
	output_name = output_name.replace('\\', '/')

	dom = preprocess(input_name);
	ui = ui_cfg(dom.documentElement)
	ui.write(open(output_name, 'wb'))
