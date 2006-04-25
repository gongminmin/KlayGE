def is_supported(feature_name):
	return feature_name in is_supported.exts

def support_all(feature_names):
	for feature_name in feature_names:
		if not is_supported(feature_name):
			return False
	return True

def support_one(feature_names):
	for feature_name in feature_names:
		if is_supported(feature_name):
			return True
	return False

ogl_ver_db = ['1.1', '1.2', '1.3', '1.4', '1.5', '2.0', '2.1']

features_db = {
	'1.1' : {
			'Vertex Array' : lambda : is_supported('GL_EXT_vertex_array'),
			'Polygon Offset' : lambda : is_supported('GL_EXT_polygon_offset'),
			'Logical Operation' : lambda : is_supported('GL_EXT_blend_logic_op'),
			'Texture Image Formats' : lambda : is_supported('GL_EXT_texture'),
			'Texture Replace Environment' : lambda : is_supported('GL_EXT_texture'),
			'Texture Proxies' : lambda : is_supported('GL_EXT_texture'),
			'Copy Texture and Subtexture' : lambda : support_all(['GL_EXT_copy_texture', 'GL_EXT_subtexture']),
			'Texture Objects' : lambda : is_supported('GL_EXT_texture_object')
		},

	'1.2' : {
			'Three-Dimensional Texturing' : lambda : is_supported('GL_EXT_texture3D'),
			'BGRA Pixel Formats' : lambda : is_supported('GL_EXT_bgra'),
			'Packed Pixel Formats' : lambda : is_supported('GL_EXT_packed_pixels'),
			'Normal Rescaling' : lambda : is_supported('GL_EXT_rescale_normal'),
			'Separate Specular Color' : lambda : is_supported('GL_EXT_separate_specular_color'),
			'Texture Coordinate Edge Clamping' : lambda : is_supported('GL_SGIS_texture_edge_clamp'),
			'Texture Level of Detail Control' : lambda : is_supported('GL_SGIS_texture_lod'),
			'Vertex Array Draw Element Range' : lambda : is_supported('GL_EXT_draw_range_elements'),
			'Imaging Subset' : lambda : is_supported('GL_ARB_imaging'),
			'Color Tables' : lambda : support_one(['GL_SGI_color_table', 'GL_EXT_paletted_texture']) and is_supported('GL_EXT_color_subtable'),
			'Convolution' : lambda : support_all(['GL_EXT_convolution', 'GL_HP_convolution_border_modes']),
			'Color Matrix' : lambda : is_supported('GL_SGI_color_matrix'),
			'Pixel Pipeline Statistics' : lambda : is_supported('GL_EXT_histogram'),
			'Constant Blend Color' : lambda : is_supported('GL_EXT_blend_color'),
			'New Blending Equations' : lambda : support_all(['GL_EXT_blend_minmax', 'GL_EXT_blend_subtract'])
		},

	'1.3' : {
			'Compressed Textures' : lambda : is_supported('GL_ARB_texture_compression'),
			'Cube Map Textures' : lambda : is_supported('GL_ARB_texture_cube_map'),
			'Multisample' : lambda : is_supported('GL_ARB_multisample'),
			'Multitexture' : lambda : is_supported('GL_ARB_multitexture'),
			'Texture Add Environment Mode' : lambda : support_one(['GL_ARB_texture_env_add', 'GL_EXT_texture_env_add']),
			'Texture Combine Environment Mode' : lambda : support_one(['GL_ARB_texture_env_combine', 'GL_EXT_texture_env_combine']),
			'Texture Dot3 Environment Mode' : lambda : support_one(['GL_ARB_texture_env_dot3', 'GL_EXT_texture_env_dot3']),
			'Texture Border Clamp' : lambda : support_one(['GL_ARB_texture_border_clamp', 'GL_SGIS_texture_border_clamp']),
			'Transpose Matrix' : lambda : is_supported('GL_ARB_transpose_matrix')
		},

	'1.4' : {
			'Automatic Mipmap Generation' : lambda : is_supported('GL_SGIS_generate_mipmap'),
			'Blend Squaring' : lambda : is_supported('GL_NV_blend_square'),
			'Depth Textures' : lambda : support_one(['GL_ARB_depth_texture', 'GL_SGIX_depth_texture']),
			'Shadows' : lambda : support_one(['GL_ARB_shadow', 'GL_SGIX_shadow']),
			'Fog Coordinate' : lambda : is_supported('GL_EXT_fog_coord'),
			'Multiple Draw Arrays' : lambda : support_one(['GL_EXT_multi_draw_arrays', 'GL_SUN_multi_draw_arrays']),
			'Point Parameters' : lambda : support_one(['GL_ARB_point_parameters', 'GL_EXT_point_parameters']),
			'Secondary Color' : lambda : is_supported('GL_EXT_secondary_color'),
			'Separate Blend Functions' : lambda : is_supported('GL_EXT_blend_func_separate'),
			'Stencil Wrap' : lambda : is_supported('GL_EXT_stencil_wrap'),
			'Texture Crossbar Environment Mode' : lambda : support_one(['GL_ARB_texture_env_crossbar', 'GL_NV_texture_env_combine4']),
			'Texture LOD Bias' : lambda : is_supported('GL_EXT_texture_lod_bias'),
			'Texture Mirrored Repeat' : lambda : is_supported('GL_ARB_texture_mirrored_repeat'),
			'Window Raster Position' : lambda : support_one(['GL_ARB_window_pos', 'GL_MESA_window_pos'])
		},

	'1.5' : {
			'Buffer Objects' : lambda : is_supported('GL_ARB_vertex_buffer_object'),
			'Occlusion Queries' : lambda : support_one(['GL_ARB_occlusion_query', 'GL_NV_occlusion_query', 'GL_HP_occlusion_test']),
			'Shadow Functions' : lambda : is_supported('GL_EXT_shadow_funcs')
		},

	'2.0' : {
			'Shader Objects' : lambda : is_supported('GL_ARB_shader_objects'),
			'Shader Programs' : lambda : support_all(['GL_ARB_vertex_shader', 'GL_ARB_fragment_shader']),
			'OpenGL Shading Language' : lambda : is_supported('GL_ARB_shading_language_100'),
			'Multiple Render Targets' : lambda : support_one(['GL_ARB_draw_buffers', 'GL_ATI_draw_buffers']),
			'Non-Power-Of-Two Textures' : lambda : is_supported('GL_ARB_texture_non_power_of_two'),
			'Point Sprites' : lambda : support_one(['GL_ARB_point_sprite', 'GL_NV_point_sprite']),
			'Separate Stencil' : lambda : support_one(['GL_ATI_separate_stencil', 'GL_EXT_stencil_two_side']),
			'Separated Blend Equation' : lambda : is_supported('GL_EXT_blend_equation_separate')
		},

	'2.1' : {
			'Pixel buffer object' : lambda : support_one(['GL_ARB_pixel_buffer_object', 'GL_EXT_pixel_buffer_object']),
			'Floating point buffer' : lambda : support_one(['GL_ARB_color_buffer_float', 'GL_ARB_texture_float', 'GL_ATI_texture_float', 'GL_NV_float_buffer', 'GL_ARB_half_float_pixel']),
			'sRGB texture' : lambda : support_one(['GL_EXT_texture_sRGB']),
			'Sync object' : lambda : support_one(['GL_ARB_synch_object', 'GL_NV_fence', 'GL2_async_core']),
			'Frame buffer object' : lambda : support_one(['GL_EXT_framebuffer_object', 'EXT_framebuffer_multisample', 'EXT_framebuffer_blit'])
		}
}

class information:
	def __init__(self):
		self.vendor = ''
		self.renderer = ''
		self.major_ver = 0
		self.minor_ver = 0
		self.feature_infos = []

	def to_xml(self, stream):
		stream.write('<?xml version="1.0" encoding="utf-8"?>\n')
		stream.write('<?xml-stylesheet type="text/xsl" href="report.xsl"?>\n\n')

		stream.write('<compatibility vendor="%s" renderer="%s" core="%i.%i">\n' % (self.vendor, self.renderer, self.major_ver, self.minor_ver))

		for feature_info in self.feature_infos:
			supported = feature_info[1][0]
			unsupported = feature_info[1][1]

			potential_rate = len(supported) * 100.0 / (len(supported) + len(unsupported))

			stream.write('\t<version name="%s" rate="%.1f">\n' % (feature_info[0], potential_rate))

			for feature in supported:
				stream.write('\t\t<supported name="%s"/>\n' % feature)

			for feature in unsupported:
				stream.write('\t\t<unsupported name="%s"/>\n' % feature)

			stream.write('\t</version>\n')

		stream.write('</compatibility>\n')

	def make_reports(self, vendor, renderer, major_ver, minor_ver, exts):
		core_ver_index = ogl_ver_db.index(str(major_ver) + '.' + str(minor_ver))

		is_supported.exts = exts

		self.vendor = vendor
		self.renderer = renderer
		self.major_ver = major_ver
		self.minor_ver = minor_ver
		self.feature_infos = []

		for i in range(0, len(ogl_ver_db)):
			supported = []
			unsupported = []

			this_ver_features = features_db[ogl_ver_db[i]]

			if i < core_ver_index + 1:
				supported = this_ver_features.keys()
			else:
				for feature_name in this_ver_features.keys():
					if this_ver_features[feature_name]():
						supported.append(feature_name)
					else:
						unsupported.append(feature_name)

			info.feature_infos.append((ogl_ver_db[i], (supported, unsupported)))

if __name__ == '__main__':
	from xml.dom.minidom import parse
	import sys

	if len(sys.argv) >= 2:
		dom = parse(sys.argv[1])

		vendor = dom.documentElement.getAttribute('vendor')
		renderer = dom.documentElement.getAttribute('renderer')
		major_ver = int(dom.documentElement.getAttribute('major_ver'))
		minor_ver = int(dom.documentElement.getAttribute('minor_ver'))

		exts = []
		ext_tags = dom.documentElement.getElementsByTagName('extension')
		for ext in ext_tags:
			exts.append(ext.getAttribute('name'))

		print 'OpenGL Compatibility Viewer'
		print 'Copyright(C) 2004-2005 Minmin Gong\n'

		info = information()
		info.make_reports(vendor, renderer, major_ver, minor_ver, exts)

		report_file_name = 'report.xml'

		info.to_xml(open(report_file_name, 'w'))

		print 'The results are saved in the file ' + report_file_name
	else:
		print 'Usage: GLCompatibility.py info.xml'
