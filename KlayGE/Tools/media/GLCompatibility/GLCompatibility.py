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

ogl_ver_db = ['1.1', '1.2', '1.3', '1.4', '1.5', '2.0', '2.1', '3.0', '3.1', '3.2', '3.3', '4.0', '4.1', '4.2', '4.3', '4.4', '4.5', '4.6']
glsl_ver_db = ['0.0', '1.1', '1.2', '1.3', '1.4', '1.5', '3.3', '4.0', '4.1', '4.2', '4.3', '4.4', '4.5', '4.6']

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
			'OpenGL Shading Language 1.20' : lambda : support_one(['GLSL_1_2', 'GL_ATI_shader_texture_lod']),
			'Pixel buffer object' : lambda : support_one(['GL_ARB_pixel_buffer_object', 'GL_EXT_pixel_buffer_object']),
			'sRGB texture' : lambda : support_one(['GL_EXT_texture_sRGB']),
		},
		
	'3.0' : {
			'OpenGL Shading Language 1.30' : lambda : support_one(['GLSL_1_3', 'GL_EXT_gpu_shader4']),
			'Conditional rendering' : lambda : is_supported('GL_NV_conditional_render'),
			'Floating-point color buffer' : lambda : is_supported('GL_ARB_color_buffer_float'),
			'Floating-point depth buffer' : lambda : support_one(['GL_ARB_depth_buffer_float', 'GL_NV_depth_buffer_float']),
			'Floating-point texture' : lambda : support_one(['GL_ARB_texture_float', 'GL_ATI_texture_float', 'GL_NV_float_buffer']),
			'Packed float' : lambda : is_supported('GL_EXT_packed_float'),
			'Shared exponent' : lambda : is_supported('GL_EXT_texture_shared_exponent'),
			'Frame buffer object' : lambda : support_one(['GL_ARB_framebuffer_object', 'GL_EXT_framebuffer_object']),
			'Multisample stretch blit' : lambda : support_all(['GL_EXT_framebuffer_multisample', 'GL_EXT_framebuffer_blit']),
			'Integer texture' : lambda : is_supported('GL_EXT_texture_integer'),
			'Texture array' : lambda : is_supported('GL_EXT_texture_array'),
			'Packed depth stencil format' : lambda : is_supported('GL_EXT_packed_depth_stencil'),
			'Per-color-attachment blend enables and color writemasks' : lambda : is_supported('GL_EXT_draw_buffers2'),
			'Transform feedback' : lambda : support_one(['GL_EXT_transform_feedback', 'GL_NV_transform_feedback']),
			'sRGB-encoded framebuffer' : lambda : support_one(['GL_ARB_framebuffer_sRGB', 'GL_EXT_framebuffer_sRGB']),
			'Half-float data type in vertex' : lambda : is_supported('GL_ARB_half_float_vertex'),
			'Map buffer range' : lambda : is_supported('GL_ARB_map_buffer_range'),
			'R and RG texture compression' : lambda : support_one(['GL_ARB_texture_compression_rgtc', 'GL_EXT_texture_compression_rgtc']),
			'R and RG texture' : lambda : is_supported('GL_ARB_texture_rg'),
			'Vertex array object' : lambda : support_one(['GL_ARB_vertex_array_object', 'GL_APPLE_vertex_array_object']),
		},
		
	'3.1' : {
			'OpenGL Shading Language 1.40' : lambda : is_supported('GLSL_1_4'),
			'Instanced rendering' : lambda : is_supported('GL_ARB_draw_instanced'),
			'Data copying between buffer objects' : lambda : support_one(['GL_ARB_copy_buffer', 'GL_EXT_copy_buffer']),
			'Primitive restart' : lambda : is_supported('GL_NV_primitive_restart'),
			'Texture buffer objects' : lambda : is_supported('GL_ARB_texture_buffer_object'),
			'Rectangular textures' : lambda : support_one(['GL_ARB_texture_rectangle', 'GL_EXT_texture_rectangle', 'GL_NV_texture_rectangle']),
			'Uniform buffer objects' : lambda : support_one(['GL_ARB_uniform_buffer_object', 'GL_EXT_bindable_uniform']),
			'Signed normalized texture component formats' : lambda : support_one(['GL_EXT_texture_snorm', 'GL_NV_texture_shader']),
		},
		
	'3.2' : {
			'OpenGL Shading Language 1.50' : lambda : is_supported('GLSL_1_5'),
			'Compatibility profiles' : lambda : is_supported('GL_ARB_compatibility'),
			'BGRA vertex component ordering' : lambda : support_one(['GL_ARB_vertex_array_bgra', 'GL_EXT_vertex_array_bgra']),
			'Modification of the base vertex index' : lambda : is_supported('GL_ARB_draw_elements_base_vertex'),
			'Shader fragment coordinate convention control' : lambda : is_supported('GL_ARB_fragment_coord_conventions'),
			'Provoking vertex control' : lambda : support_one(['GL_ARB_provoking_vertex', 'GL_EXT_provoking_vertex']),
			'Seamless cube map filtering' : lambda : is_supported('GL_ARB_seamless_cube_map'),
			'Multisampled textures and texture samplers for specific sample locations' : lambda : is_supported('GL_ARB_texture_multisample'),
			'Fragment depth clamping' : lambda : support_one(['GL_ARB_depth_clamp', 'GL_NV_depth_clamp']),
			'Geometry shaders' : lambda : support_one(['GL_ARB_geometry_shader4', 'GL_EXT_geometry_shader4', 'GL_NV_geometry_shader4', 'GL_NV_geometry_program4']),
			'Fence sync objects' : lambda : support_one(['GL_ARB_sync', 'GL_NV_fence']),
		},
		
	'3.3' : {
			'OpenGL Shading Language 3.30' : lambda : support_all(['GLSL_3_3', 'GL_ARB_shader_bit_encoding']),
			'New blending functions' : lambda : is_supported('GL_ARB_blend_func_extended'),
			'Pre-assign attribute locations' : lambda : is_supported('GL_ARB_explicit_attrib_location'),
			'Simple boolean occlusion queries' : lambda : is_supported('GL_ARB_occlusion_query2'),
			'Sampler objects' : lambda : is_supported('GL_ARB_sampler_objects'),
			'Unsigned 10.10.10.2 integer textures format' : lambda : is_supported('GL_ARB_texture_rgb10_a2ui'),
			'Swizzle the components of a texture' : lambda : support_one(['GL_ARB_texture_swizzle', 'GL_EXT_texture_swizzle']),
			'Timer query' : lambda : support_one(['GL_ARB_timer_query', 'GL_EXT_timer_query']),
			'Instanced array' : lambda : is_supported('GL_ARB_instanced_arrays'),
			'New 2.10.10.10 vertex attribute data formats' : lambda : is_supported('GL_ARB_vertex_type_2_10_10_10_rev'),
		},

	'4.0' : {
			'OpenGL Shading Language 4.0' : lambda : support_all(['GLSL_4_0', 'GL_ARB_texture_query_lod']),
			'Set individual blend equations/functions for each color output' : lambda : support_one(['GL_ARB_draw_buffers_blend', 'GL_AMD_draw_buffers_blend']),
			'Supplying the arguments to a drawing command from buffer object' : lambda : is_supported('GL_ARB_draw_indirect'),
			'GPU Shader 5' : lambda : is_supported('GL_ARB_gpu_shader5'),
			'Double-precision floating-point types in shaders' : lambda : is_supported('GL_ARB_gpu_shader_fp64'),
			'Explicitly shading at samples' : lambda : is_supported('GL_ARB_sample_shading'),
			'Support for indirect subroutine calls in shaders' : lambda : is_supported('GL_ARB_shader_subroutine'),
			'Tessellation stages' : lambda : support_one(['GL_ARB_tessellation_shader', 'GL_AMD_vertex_shader_tessellator']),
			'Three-component buffer texture formats' : lambda : is_supported('GL_ARB_texture_buffer_object_rgb32'),
			'Cube map array textures' : lambda : is_supported('GL_ARB_texture_cube_map_array'),
			'textureGather in shaders' : lambda : support_one(['GL_ARB_texture_gather', 'GL_AMD_texture_texture4']),
			'Additional transform feedback functionality': lambda : support_one(['GL_ARB_transform_feedback2', 'GL_ARB_transform_feedback3', 'GL_NV_transform_feedback2']),
		},

	'4.1' : {
			'Improved OpenGL ES 2.0 compatibility' : lambda : is_supported('GL_ARB_ES2_compatibility'),
			'Binary represtation of a program object' : lambda : is_supported('GL_ARB_get_program_binary'),
			'Separately shader objects for different shader stages' : lambda : support_one(['GL_ARB_separate_shader_objects', 'GL_EXT_separate_shader_objects']),
			'Precision requirements for shaders' : lambda : is_supported('GL_ARB_shader_precision'),
			'64-bit fp components for VS inputs' : lambda : support_one(['GL_ARB_vertex_attrib_64bit', 'GL_EXT_vertex_attrib_64bit']),
			'Multiple viewports' : lambda : is_supported ('GL_ARB_viewport_array'),
		},

	'4.2' : {
			'BPTC compressed textures' : lambda : is_supported('GL_ARB_texture_compression_bptc'),
			'Allow pixel storage parameters to affect packing and unpacking of compressed textures' : lambda : is_supported('GL_ARB_compressed_texture_pixel_storage'),
			'Shader atomic counters' : lambda : is_supported('GL_ARB_shader_atomic_counters'),
			'Immutable texture images' : lambda : is_supported('GL_ARB_texture_storage'),
			'Instanced transformed feedback drawing' : lambda : is_supported('GL_ARB_transform_feedback_instanced'),
			'Allow the offset within buffer objects used for instanced rendering to be specified' : lambda : is_supported('GL_ARB_base_instance'),
			'Loads from and stores to textures from shader' : lambda : support_one(['GL_ARB_shader_image_load_store', 'GL_EXT_shader_image_load_store']),
			'Add new layout qualifiers to communicate what kind of changes will be made to gl_FragDepth' : lambda : support_one(['GL_ARB_conservative_depth', 'GL_AMD_conservative_depth']),
			'GLSL 4.20 feature pack' : lambda : is_supported('GL_ARB_shading_language_420pack'),
			'Queries for sample counts available for a given internal format and usage' : lambda : is_supported('GL_ARB_internalformat_query'),
			'More restrictive alignment constraints for mapped buffers' : lambda : is_supported('GL_ARB_map_buffer_alignment'),
		},

	'4.3' : {
			'Allows multi-dimensional arrays in GLSL' : lambda : is_supported('GL_ARB_arrays_of_arrays'),
			'OpenGL ES 3.0 compatibility' : lambda : is_supported('GL_ARB_ES3_compatibility'),
			'Clear a buffer object with a constant value' : lambda : is_supported('GL_ARB_clear_buffer_object'),
			'Compute shader' : lambda : is_supported('GL_ARB_compute_shader'),
			'Direct copy of pixels between textures and render buffers' : lambda : support_one(['GL_ARB_copy_image', 'GL_NV_copy_image']),
			'Enhanced debug context' : lambda : is_supported('GL_KHR_debug'),
			'Debug output' : lambda : is_supported('GL_ARB_debug_output'),
			'Set location of a default-block uniform in the shader' : lambda : is_supported('GL_ARB_explicit_uniform_location'),
			'gl_Layer and gl_ViewportIndex in fragment shader' : lambda : is_supported('GL_ARB_fragment_layer_viewport'),
			'Framebuffer without attachment' : lambda : is_supported('GL_ARB_framebuffer_no_attachments'),
			'Find out actual supported limits for most texture parameters' : lambda : is_supported('GL_ARB_internalformat_query2'),
			'Invalidate all or some of the contents of textures and buffers' : lambda : is_supported('GL_ARB_invalidate_subdata'),
			'Draw many GPU generated objects with one call' : lambda : support_one(['GL_ARB_multi_draw_indirect', 'GL_AMD_multi_draw_indirect']),
			'Shader reflection' : lambda : is_supported('GL_ARB_program_interface_query'),
			'Restricted shader read/write to an object' : lambda : is_supported('GL_ARB_robust_buffer_access_behavior'),
			'Query size of an image in a shader' : lambda : is_supported('GL_ARB_shader_image_size'),
			'Enables all shader stages to read and write arbitrarily to buffers' : lambda : is_supported('GL_ARB_shader_storage_buffer_object'),
			'Read stencil bits of a packed depth-stencil texture' : lambda : is_supported('GL_ARB_stencil_texturing'),
			'Create texture buffer object corresponding to a subrange of a buffer\'s data store' : lambda : is_supported('GL_ARB_texture_buffer_range'),
			'Query number of mipmap levels accessible through a sampler uniform' : lambda : is_supported('GL_ARB_texture_query_levels'),
			'Immutable storage objects for multisampled textures' : lambda : is_supported('GL_ARB_texture_storage_multisample'),
			'Provide different ways to interpret texture data without duplicating the texture' : lambda : is_supported('GL_ARB_texture_view'),
			'Separate vertex attribute state from the data stores of each array' : lambda : is_supported('GL_ARB_vertex_attrib_binding'),
		},

	'4.4' : {
			'Buffer storage' : lambda : is_supported('GL_ARB_buffer_storage'),
			'Clear a texture' : lambda : is_supported('GL_ARB_clear_texture'),
			'Enhanced layouts' : lambda : is_supported('GL_ARB_enhanced_layouts'),
			'Multi-bind' : lambda : is_supported('GL_ARB_multi_bind'),
			'Query buffer object' : lambda : support_one(['GL_ARB_query_buffer_object', 'GL_AMD_query_buffer_object']),
			'Texture mirror clamp to edge' : lambda : support_one(['GL_ARB_texture_mirror_clamp_to_edge', 'GL_EXT_texture_mirror_clamp', 'GL_ATI_texture_mirror_once']),
			'Stencil8 format texture' : lambda : is_supported('GL_ARB_texture_stencil8'),
			'B10G11R11F format for vertex' : lambda : is_supported('GL_ARB_vertex_type_10f_11f_11f_rev'),
		},

	'4.5' : {
			'Clip control' : lambda : is_supported('GL_ARB_clip_control'),
			'Cull distance' : lambda : is_supported('GL_ARB_cull_distance'),
			'OpenGL ES 3.1 compatibility' : lambda : is_supported('GL_ARB_ES3_1_compatibility'),
			'Conditional Rendering inverted' : lambda : is_supported('GL_ARB_conditional_render_inverted'),
			'Context flush control' : lambda : support_one(['GL_KHR_context_flush_control', 'GL_ARB_context_flush_control']),
			'GLSL Derivative control' : lambda : is_supported('GL_ARB_derivative_control'),
			'Direct state access' : lambda : support_one(['GL_ARB_direct_state_access', 'GL_EXT_direct_state_access']),
			'Get texture sub image' : lambda : is_supported('GL_ARB_get_texture_sub_image'),
			'Robustness' : lambda : support_one(['GL_KHR_robustness', 'GL_ARB_robustness']),
			'GLSL texture image samples' : lambda : is_supported('GL_ARB_shader_texture_image_samples'),
			'Texture barrier' : lambda : support_one(['GL_ARB_texture_barrier', 'GL_NV_texture_barrier']),
		},

	'4.6' : {
			'Indirect parameters' : lambda : is_supported('GL_ARB_indirect_parameters'),
			'Pipeline statistics query' : lambda : is_supported('GL_ARB_pipeline_statistics_query'),
			'Polygon offset clamp' : lambda : support_one(['GL_ARB_polygon_offset_clamp', 'GL_EXT_polygon_offset_clamp']),
			'No error context' : lambda : is_supported('GL_KHR_no_error'),
			'Shader atomic counter ops' : lambda : is_supported('GL_ARB_shader_atomic_counter_ops'),
			'Shader draw parameters' : lambda : is_supported('GL_ARB_shader_draw_parameters'),
			'Shader group vote' : lambda : is_supported('GL_ARB_shader_group_vote'),
			'SPIR-V' : lambda : is_supported('GL_ARB_gl_spirv'),
			'SPIR-V extensions' : lambda : is_supported('GL_ARB_spirv_extensions'),
			'Texture filter anisotropic' : lambda : support_one(['GL_ARB_texture_filter_anisotropic', 'GL_EXT_texture_filter_anisotropic']),
			'Transform feedback overflow query' : lambda : is_supported('GL_ARB_transform_feedback_overflow_query'),
		}
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
		stream.write('<h1>OpenGL Compatibility</h1>\n')
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

			stream.write('\t<h3>OpenGL %s potential support rate: %.1f%%</h3>\n' % (feature_info[0], potential_rate))
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
			is_supported.exts.append('GLSL_1_2')
		if glsl_ver_index >= 3:
			is_supported.exts.append('GLSL_1_3')
		if glsl_ver_index >= 4:
			is_supported.exts.append('GLSL_1_4')
		if glsl_ver_index >= 5:
			is_supported.exts.append('GLSL_1_5')
		if glsl_ver_index >= 6:
			is_supported.exts.append('GLSL_3_3')
		if glsl_ver_index >= 7:
			is_supported.exts.append('GLSL_4_0')
		if glsl_ver_index >= 8:
			is_supported.exts.append('GLSL_4_1')
		if glsl_ver_index >= 9:
			is_supported.exts.append('GLSL_4_2')
		if glsl_ver_index >= 10:
			is_supported.exts.append('GLSL_4_3')
		if glsl_ver_index >= 11:
			is_supported.exts.append('GLSL_4_4')
		if glsl_ver_index >= 12:
			is_supported.exts.append('GLSL_4_5')
		if glsl_ver_index >= 13:
			is_supported.exts.append('GLSL_4_6')

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

def gl_compatibility(vendor, renderer, major_ver, minor_ver, glsl_major_ver, glsl_minor_ver, ext_str):
	exts = ext_str.split(' ')

	info = information()
	info.make_reports(vendor, renderer, major_ver, minor_ver, glsl_major_ver, glsl_minor_ver, exts)

	report_file_name = 'GLCompatibilityReport.html'
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
