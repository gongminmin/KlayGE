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

ogl_ver_db = ['2.0', '3.0', '3.1', '3.2']
glsl_ver_db = ['1.0', '1.1', '3.0', '3.1', '3.2']

features_db = {
	'2.0' : {
			'' : ''
		},

	'3.0' : {
			'OpenGL Shading Language ES 3.00' : lambda : is_supported('GLSL_3_0'),
			'Transform feedback' : lambda : support_one(['GL_EXT_transform_feedback', 'GL_NV_transform_feedback']),
			'Uniform buffer objects' : lambda : support_one(['GL_ARB_uniform_buffer_object', 'GL_EXT_bindable_uniform']),
			'Vertex array object' : lambda : is_supported(['GLES_OES_vertex_array_object']),
			'Sampler objects' : lambda : is_supported('GL_ARB_sampler_objects'),
			'Fence sync objects' : lambda : is_supported('GLES_APPLE_sync'),
			'Pixel buffer object' : lambda : is_supported('GLES_NV_pixel_buffer_object'),
			'Map buffer range' : lambda : is_supported('GLES_EXT_map_buffer_range'),
			'Data copying between buffer objects' : lambda : is_supported('GLES_NV_copy_buffer'),
			'Simple boolean occlusion queries' : lambda : is_supported('GLES_EXT_occlusion_query_boolean'),
			'Instanced rendering' : lambda : support_one(['GLES_EXT_draw_instanced', 'GLES_NV_draw_instanced']),
			'Instanced array' : lambda : support_one(['GLES_EXT_instanced_arrays', 'GLES_NV_instanced_arrays', 'GLES_ANGLE_instanced_arrays']),
			'Multiple Render Targets' : lambda : support_one(['GLES_NV_draw_buffers', 'GLES_EXT_draw_buffers']),
			'Texture array' : lambda : is_supported('GLES_NV_texture_array'),
			'Three-Dimensional Texturing' : lambda : is_supported('GLES_OES_texture3D'),
			'Immutable texture images' : lambda : is_supported('GLES_EXT_texture_storage'),
			'R and RG texture' : lambda : is_supported('GLES_EXT_texture_rg'),
			'Swizzle the components of a texture' : lambda : support_one(['GL_ARB_texture_swizzle', 'GL_EXT_texture_swizzle']),
			'Seamless cube map filtering' : lambda : is_supported('GL_ARB_seamless_cube_map'),
			'Non-Power-Of-Two Textures' : lambda : is_supported('GLES_OES_texture_npot'),
			'Texture LOD Bias' : lambda : is_supported('GLES_EXT_texture_lod_bias'),
			'Integer texture' : lambda : is_supported('GL_EXT_texture_integer'),
			'sRGB-encoded framebuffer' : lambda : is_supported('GLES_EXT_sRGB'),
			'Packed float' : lambda : is_supported('GLES_NV_packed_float'),
			'Shared exponent' : lambda : is_supported('GL_EXT_texture_shared_exponent'),
			'Unsigned 10.10.10.2 integer textures format' : lambda : is_supported('GL_ARB_texture_rgb10_a2ui'),
			'New 2.10.10.10 vertex attribute data formats' : lambda : is_supported('GLES_OES_vertex_type_10_10_10_2'),
			'Half-float data type in vertex' : lambda : is_supported('GLES_OES_vertex_half_float'),
			'Depth Textures' : lambda : support_one(['GLES_OES_depth_texture', 'GLES_ANGLE_depth_texture']),
			'Shadows' : lambda : support_one(['GLES_EXT_shadow_samplers', 'GLES_NV_shadow_samplers_array', 'GLES_NV_shadow_samplers_cube']),
			'Packed depth stencil format' : lambda : is_supported('GLES_OES_packed_depth_stencil'),
			'Floating-point texture' : lambda : support_one(['GLES_OES_texture_float', 'GL_ATI_texture_float', 'GL_NV_float_buffer']),
			'Floating-point color buffer' : lambda : is_supported('GLES_EXT_color_buffer_float'),
			'Floating-point depth buffer' : lambda : support_one(['GL_ARB_depth_buffer_float', 'GL_NV_depth_buffer_float']),
			'Multisample stretch blit' : lambda : support_one(['GLES_ANGLE_framebuffer_multisample', 'GLES_ANGLE_framebuffer_blit', 'GLES_NV_framebuffer_multisample', 'GLES_NV_framebuffer_blit']),
			'Primitive restart' : lambda : is_supported('GL_NV_primitive_restart'),
			'Vertex Array Draw Element Range' : lambda : is_supported('GL_EXT_draw_range_elements'),
			'New Blending Equations' : lambda : is_supported('GLES_EXT_blend_minmax'),
			'Binary represtation of a program object' : lambda : support_one(['GLES_OES_get_program_binary', 'GLES_AMD_program_binary_Z400', 'GLES_IMG_program_binary', 'GLES_IMG_shader_binary', 'GLES_ARM_mali_shader_binary', 'GLES_VIV_shader_binary', 'GLES_DMP_shader_binary', 'GLES_FJ_shader_binary_GCCSO', 'GLES_ARM_mali_program_binary', 'GLES_NV_platform_binary', 'GLES_ANGLE_program_binary'])
		},

	'3.1' : {
			'OpenGL Shading Language ES 3.10' : lambda : is_supported('GLSL_3_1'),
			'Compute shader' : lambda : is_supported('GL_ARB_compute_shader'),
			'Draw many GPU generated objects with one call' : lambda : support_one(['GL_ARB_multi_draw_indirect', 'GL_AMD_multi_draw_indirect']),
			'Framebuffer without attachment' : lambda : is_supported('GL_ARB_framebuffer_no_attachments'),
			'Shader reflection' : lambda : is_supported('GL_ARB_program_interface_query'),
			'Separately shader objects for different shader stages' : lambda : is_supported('GLES_EXT_separate_shader_objects'),
			'Loads from and stores to textures from shader' : lambda : support_one(['GL_ARB_shader_image_load_store', 'GL_EXT_shader_image_load_store']),
			'Immutable storage objects for multisampled textures' : lambda : is_supported('GL_ARB_texture_storage_multisample'),
			'Multisampled textures and texture samplers for specific sample locations' : lambda : is_supported('GL_ARB_texture_multisample'),
			'Separate vertex attribute state from the data stores of each array' : lambda : is_supported('GL_ARB_vertex_attrib_binding'),
			'Pre-assign attribute locations' : lambda : is_supported('GLES_NV_explicit_attrib_location'),
		},

	'3.2' : {
			'OpenGL Shading Language ES 3.20' : lambda : is_supported('GLSL_3_2'),
			'Blend equation advanced' : lambda : support_one(['GLES_KHR_blend_equation_advanced', 'GLES_NV_blend_equation_advanced']),
			'Direct copy of pixels between textures and render buffers' : lambda : support_one(['GLES_OES_copy_image', 'GLES_EXT_copy_image']),
			'Enhanced debug context' : lambda : is_supported('GLES_KHR_debug'),
			'Draw buffers indexed' : lambda : support_one(['GLES_OES_draw_buffers_indexed', 'GLES_EXT_draw_buffers_indexed']),
			'Modification of the base vertex index' : lambda : support_one(['GLES_EXT_draw_elements_base_vertex', 'GLES_OES_draw_elements_base_vertex']),
			'Geometry shaders' : lambda : support_one(['GLES_OES_geometry_shader', 'GLES_EXT_geometry_shader']),
			'Primitive bounding box' : lambda : support_one(['GLES_OES_primitive_bounding_box', 'GLES_EXT_primitive_bounding_box']),
			'Robustness' : lambda : support_one(['GLES_KHR_robustness', 'GLES_EXT_robustness']),
			'Explicitly shading at samples' : lambda : is_supported('GLES_OES_sample_shading'),
			'Tessellation stages' : lambda : support_one(['GLES_OES_tessellation_shader', 'GLES_EXT_tessellation_shader']),
			'Texture border clamp' : lambda : support_one(['GLES_OES_texture_border_clamp', 'GLES_EXT_texture_border_clamp']),
			'Texture buffer' : lambda : support_one(['GLES_OES_texture_buffer', 'GLES_EXT_texture_buffer']),
			'Immutable storage objects for multisampled 2d texture array' : lambda : is_supported('GLES_OES_texture_storage_multisample_2d_array'),
			'Shader multisample interpolation' : lambda : is_supported('GLES_OES_shader_multisample_interpolation'),
			'ASTC texture compression' : lambda : is_supported('GLES_OES_texture_compression_astc'),
			'Cube map array' : lambda : is_supported('GLES_EXT_texture_cube_map_array'),
		},
}

class information:
	def __init__(self):
		self.vendor = ''
		self.renderer = ''
		self.major_ver = 0
		self.minor_ver = 0
		self.glsl_major_ver = 0
		self.glsl_minor_ver = 0
		self.exts = []
		self.feature_infos = []

	def to_html(self, stream):
		stream.write('<html>\n')
		stream.write('<head>\n')
		stream.write('\t<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>\n')
		stream.write('\t<title>OpenGL Compatibility</title>\n')
		stream.write('</head>\n')
		stream.write('<body>\n')
		stream.write('<h1>OpenGL ES Compatibility</h1>\n')
		stream.write('\t<table width="100%">\n')
		stream.write('\t\t<tr>\n')
		stream.write('\t\t\t<th style="background: gray; color: white">Vendor:</th>\n')
		stream.write('\t\t\t<td style="border-bottom: 1px solid black">%s</td>\n' % self.vendor)
		stream.write('\t\t</tr>\n')
		stream.write('\t\t<tr>\n')
		stream.write('\t\t\t<th style="background: gray; color: white">Renderer:</th>\n')
		stream.write('\t\t\t<td style="border-bottom: 1px solid black">%s</td>\n' % self.renderer)
		stream.write('\t\t</tr>\n')
		stream.write('\t\t<tr>\n')
		stream.write('\t\t\t<th style="background: gray; color: white">Core:</th>\n')
		stream.write('\t\t\t<td style="border-bottom: 1px solid black">%i.%i</td>\n' % (self.major_ver, self.minor_ver))
		stream.write('\t\t</tr>\n')
		stream.write('\t\t<tr>\n')
		stream.write('\t\t\t<th style="background: gray; color: white">GLSL:</th>\n')
		stream.write('\t\t\t<td style="border-bottom: 1px solid black">%i.%i</td>\n' % (self.glsl_major_ver, self.glsl_minor_ver))
		stream.write('\t\t</tr>\n')
		stream.write('\t</table>\n')

		stream.write('\t<h2>Details</h2>\n')
		for feature_info in self.feature_infos:
			supported = feature_info[1][0]
			unsupported = feature_info[1][1]

			potential_rate = len(supported) * 100.0 / (len(supported) + len(unsupported))

			stream.write('\t<h3>OpenGL ES %s potential support rate: %.1f%%</h3>\n' % (feature_info[0], potential_rate))
			stream.write('\t<table width="100%" style="border-bottom: 1px solid black">\n')

			if len(supported) > 0:
				stream.write('\t\t<tr>\n')
				stream.write('\t\t\t<th style="background: green; color: white; text-align: left">Supported</th>\n')
				stream.write('\t\t</tr>\n')
				stream.write('\t\t<tr>\n')
				stream.write('\t\t\t<td style="padding-left:20pt">\n')

				for feature in supported:
					stream.write('\t\t\t\t%s<br />\n' % feature)

				stream.write('\t\t\t</td>\n')
				stream.write('\t\t</tr>\n')

			if len(unsupported) > 0:
				stream.write('\t\t<tr>\n')
				stream.write('\t\t\t<th style="background: red; color: white; text-align: left">Unsupported</th>\n')
				stream.write('\t\t</tr>\n')
				stream.write('\t\t<tr>\n')
				stream.write('\t\t\t<td style="padding-left:20pt">\n')

				for feature in unsupported:
					stream.write('\t\t\t\t%s<br />\n' % feature)

				stream.write('\t\t\t</td>\n')
				stream.write('\t\t</tr>\n')
				
			stream.write('\t</table>\n')

		stream.write('\t<h2>Extensions</h2>\n')
		stream.write('\t<table width="100%">\n')
		for ext in self.exts:
			stream.write('\t\t<tr>\n')
			stream.write('\t\t\t<td>%s</td>\n' % ext)
			stream.write('\t\t</tr>\n')
		stream.write('\t</table>\n')

		stream.write('</body>\n')
		stream.write('</html>\n')

	def make_reports(self, vendor, renderer, major_ver, minor_ver, glsl_major_ver, glsl_minor_ver, exts):
		core_ver_index = ogl_ver_db.index(str(major_ver) + '.' + str(minor_ver))
		glsl_ver_index = glsl_ver_db.index(str(glsl_major_ver) + '.' + str(glsl_minor_ver))

		self.vendor = vendor
		self.renderer = renderer
		self.major_ver = major_ver
		self.minor_ver = minor_ver
		self.glsl_major_ver = glsl_major_ver
		self.glsl_minor_ver = glsl_minor_ver
		self.exts = exts
		self.feature_infos = []

		is_supported.exts = exts

		if glsl_ver_index >= 1:
			is_supported.exts.append('GLSL_1_1')
		if glsl_ver_index >= 2:
			is_supported.exts.append('GLSL_3_0')
		if glsl_ver_index >= 3:
			is_supported.exts.append('GLSL_3_1')
		if glsl_ver_index >= 4:
			is_supported.exts.append('GLSL_3_2')

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

			self.feature_infos.append((ogl_ver_db[i], (supported, unsupported)))

def gles_compatibility(vendor, renderer, major_ver, minor_ver, glsl_major_ver, glsl_minor_ver, ext_str):
	exts = ext_str.split(' ')

	info = information()
	info.make_reports(vendor, renderer, major_ver, minor_ver, glsl_major_ver, glsl_minor_ver, exts)

	report_file_name = 'GLESCompatibilityReport.html'
	report_file = open(report_file_name, 'w')
	info.to_html(report_file)
	report_file.close()

	command = ""
	import os
	if (os.name != "nt"):
		try:
			if ("Darwin" == os.uname()[0]):
				command = "open "
			else:
				command = "firefox "
		except:
			pass
	os.system(command + report_file_name);
