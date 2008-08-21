/*
// glloader
// Copyright (C) 2004-2005 Minmin Gong
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _GL10_H
#define _GL10_H

#ifdef __cplusplus
extern "C"
{
#endif

/* OpenGL 1.0 */

#ifndef GL_VERSION_1_0
#define GL_VERSION_1_0 1

#define GL_ACCUM										0x0100
#define GL_LOAD											0x0101
#define GL_RETURN										0x0102
#define GL_MULT											0x0103
#define GL_ADD											0x0104
#define GL_NEVER										0x0200
#define GL_LESS											0x0201
#define GL_EQUAL										0x0202
#define GL_LEQUAL										0x0203
#define GL_GREATER										0x0204
#define GL_NOTEQUAL										0x0205
#define GL_GEQUAL										0x0206
#define GL_ALWAYS										0x0207
#define GL_CURRENT_BIT									0x00000001
#define GL_POINT_BIT									0x00000002
#define GL_LINE_BIT										0x00000004
#define GL_POLYGON_BIT									0x00000008
#define GL_POLYGON_STIPPLE_BIT							0x00000010
#define GL_PIXEL_MODE_BIT								0x00000020
#define GL_LIGHTING_BIT									0x00000040
#define GL_FOG_BIT										0x00000080
#define GL_DEPTH_BUFFER_BIT								0x00000100
#define GL_ACCUM_BUFFER_BIT								0x00000200
#define GL_STENCIL_BUFFER_BIT							0x00000400
#define GL_VIEWPORT_BIT									0x00000800
#define GL_TRANSFORM_BIT								0x00001000
#define GL_ENABLE_BIT									0x00002000
#define GL_COLOR_BUFFER_BIT								0x00004000
#define GL_HINT_BIT										0x00008000
#define GL_EVAL_BIT										0x00010000
#define GL_LIST_BIT										0x00020000
#define GL_TEXTURE_BIT									0x00040000
#define GL_SCISSOR_BIT									0x00080000
#define GL_ALL_ATTRIB_BITS								0x000FFFFF
#define GL_POINTS										0x0000
#define GL_LINES										0x0001
#define GL_LINE_LOOP									0x0002
#define GL_LINE_STRIP									0x0003
#define GL_TRIANGLES									0x0004
#define GL_TRIANGLE_STRIP								0x0005
#define GL_TRIANGLE_FAN									0x0006
#define GL_QUADS										0x0007
#define GL_QUAD_STRIP									0x0008
#define GL_POLYGON										0x0009
#define GL_ZERO											0
#define GL_ONE											1
#define GL_SRC_COLOR									0x0300
#define GL_ONE_MINUS_SRC_COLOR							0x0301
#define GL_SRC_ALPHA									0x0302
#define GL_ONE_MINUS_SRC_ALPHA							0x0303
#define GL_DST_ALPHA									0x0304
#define GL_ONE_MINUS_DST_ALPHA							0x0305
#define GL_DST_COLOR									0x0306
#define GL_ONE_MINUS_DST_COLOR							0x0307
#define GL_SRC_ALPHA_SATURATE							0x0308
#define GL_TRUE											1
#define GL_FALSE										0
#define GL_CLIP_DISTANCE0								0x3000
#define GL_CLIP_DISTANCE1								0x3001
#define GL_CLIP_DISTANCE2								0x3002
#define GL_CLIP_DISTANCE3								0x3003
#define GL_CLIP_DISTANCE4								0x3004
#define GL_CLIP_DISTANCE5								0x3005
#define GL_BYTE											0x1400
#define GL_UNSIGNED_BYTE								0x1401
#define GL_SHORT										0x1402
#define GL_UNSIGNED_SHORT								0x1403
#define GL_INT											0x1404
#define GL_UNSIGNED_INT									0x1405
#define GL_FLOAT										0x1406
#define GL_2_BYTES										0x1407
#define GL_3_BYTES										0x1408
#define GL_4_BYTES										0x1409
#define GL_NONE											0
#define GL_FRONT_LEFT									0x0400
#define GL_FRONT_RIGHT									0x0401
#define GL_BACK_LEFT									0x0402
#define GL_BACK_RIGHT									0x0403
#define GL_FRONT										0x0404
#define GL_BACK											0x0405
#define GL_LEFT											0x0406
#define GL_RIGHT										0x0407
#define GL_FRONT_AND_BACK								0x0408
#define GL_AUX0											0x0409
#define GL_AUX1											0x040A
#define GL_AUX2											0x040B
#define GL_AUX3											0x040C
#define GL_NO_ERROR										0
#define GL_INVALID_ENUM									0x0500
#define GL_INVALID_VALUE								0x0501
#define GL_INVALID_OPERATION							0x0502
#define GL_STACK_OVERFLOW								0x0503
#define GL_STACK_UNDERFLOW								0x0504
#define GL_OUT_OF_MEMORY								0x0505
#define GL_2D											0x0600
#define GL_3D											0x0601
#define GL_3D_COLOR										0x0602
#define GL_3D_COLOR_TEXTURE								0x0603
#define GL_4D_COLOR_TEXTURE								0x0604
#define GL_PASS_THROUGH_TOKEN							0x0700
#define GL_POINT_TOKEN									0x0701
#define GL_LINE_TOKEN									0x0702
#define GL_POLYGON_TOKEN								0x0703
#define GL_BITMAP_TOKEN									0x0704
#define GL_DRAW_PIXEL_TOKEN								0x0705
#define GL_COPY_PIXEL_TOKEN								0x0706
#define GL_LINE_RESET_TOKEN								0x0707
#define GL_EXP											0x0800
#define GL_EXP2											0x0801
#define GL_CW											0x0900
#define GL_CCW											0x0901
#define GL_COEFF										0x0A00
#define GL_ORDER										0x0A01
#define GL_DOMAIN										0x0A02
#define GL_CURRENT_COLOR								0x0B00
#define GL_CURRENT_INDEX								0x0B01
#define GL_CURRENT_NORMAL								0x0B02
#define GL_CURRENT_TEXTURE_COORDS						0x0B03
#define GL_CURRENT_RASTER_COLOR							0x0B04
#define GL_CURRENT_RASTER_INDEX							0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS				0x0B06
#define GL_CURRENT_RASTER_POSITION						0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID				0x0B08
#define GL_CURRENT_RASTER_DISTANCE						0x0B09
#define GL_POINT_SMOOTH									0x0B10
#define GL_POINT_SIZE									0x0B11
#define GL_POINT_SIZE_RANGE								0x0B12
#define GL_POINT_SIZE_GRANULARITY						0x0B13
#define GL_LINE_SMOOTH									0x0B20
#define GL_LINE_WIDTH									0x0B21
#define GL_LINE_WIDTH_RANGE								0x0B22
#define GL_LINE_WIDTH_GRANULARITY						0x0B23
#define GL_LINE_STIPPLE									0x0B24
#define GL_LINE_STIPPLE_PATTERN							0x0B25
#define GL_LINE_STIPPLE_REPEAT							0x0B26
#define GL_LIST_MODE									0x0B30
#define GL_MAX_LIST_NESTING								0x0B31
#define GL_LIST_BASE									0x0B32
#define GL_LIST_INDEX									0x0B33
#define GL_POLYGON_MODE									0x0B40
#define GL_POLYGON_SMOOTH								0x0B41
#define GL_POLYGON_STIPPLE								0x0B42
#define GL_EDGE_FLAG									0x0B43
#define GL_CULL_FACE									0x0B44
#define GL_CULL_FACE_MODE								0x0B45
#define GL_FRONT_FACE									0x0B46
#define GL_LIGHTING										0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER						0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE							0x0B52
#define GL_LIGHT_MODEL_AMBIENT							0x0B53
#define GL_SHADE_MODEL									0x0B54
#define GL_COLOR_MATERIAL_FACE							0x0B55
#define GL_COLOR_MATERIAL_PARAMETER						0x0B56
#define GL_COLOR_MATERIAL								0x0B57
#define GL_FOG											0x0B60
#define GL_FOG_INDEX									0x0B61
#define GL_FOG_DENSITY									0x0B62
#define GL_FOG_START									0x0B63
#define GL_FOG_END										0x0B64
#define GL_FOG_MODE										0x0B65
#define GL_FOG_COLOR									0x0B66
#define GL_DEPTH_RANGE									0x0B70
#define GL_DEPTH_TEST									0x0B71
#define GL_DEPTH_WRITEMASK								0x0B72
#define GL_DEPTH_CLEAR_VALUE							0x0B73
#define GL_DEPTH_FUNC									0x0B74
#define GL_ACCUM_CLEAR_VALUE							0x0B80
#define GL_STENCIL_TEST									0x0B90
#define GL_STENCIL_CLEAR_VALUE							0x0B91
#define GL_STENCIL_FUNC									0x0B92
#define GL_STENCIL_VALUE_MASK							0x0B93
#define GL_STENCIL_FAIL									0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL						0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS						0x0B96
#define GL_STENCIL_REF									0x0B97
#define GL_STENCIL_WRITEMASK							0x0B98
#define GL_MATRIX_MODE									0x0BA0
#define GL_NORMALIZE									0x0BA1
#define GL_VIEWPORT										0x0BA2
#define GL_MODELVIEW_STACK_DEPTH						0x0BA3
#define GL_PROJECTION_STACK_DEPTH						0x0BA4
#define GL_MODELVIEW_MATRIX								0x0BA6
#define GL_TEXTURE_STACK_DEPTH							0x0BA5
#define GL_PROJECTION_MATRIX							0x0BA7
#define GL_TEXTURE_MATRIX								0x0BA8
#define GL_ATTRIB_STACK_DEPTH							0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH					0x0BB1
#define GL_ALPHA_TEST									0x0BC0
#define GL_ALPHA_TEST_FUNC								0x0BC1
#define GL_ALPHA_TEST_REF								0x0BC2
#define GL_DITHER										0x0BD0
#define GL_BLEND_DST									0x0BE0
#define GL_BLEND_SRC									0x0BE1
#define GL_BLEND										0x0BE2
#define GL_LOGIC_OP_MODE								0x0BF0
#define GL_INDEX_LOGIC_OP								0x0BF1
#define GL_COLOR_LOGIC_OP								0x0BF2
#define GL_AUX_BUFFERS									0x0C00
#define GL_DRAW_BUFFER									0x0C01
#define GL_READ_BUFFER									0x0C02
#define GL_SCISSOR_BOX									0x0C10
#define GL_SCISSOR_TEST									0x0C11
#define GL_INDEX_CLEAR_VALUE							0x0C20
#define GL_INDEX_WRITEMASK								0x0C21
#define GL_COLOR_CLEAR_VALUE							0x0C22
#define GL_COLOR_WRITEMASK								0x0C23
#define GL_INDEX_MODE									0x0C30
#define GL_RGBA_MODE									0x0C31
#define GL_DOUBLEBUFFER									0x0C32
#define GL_STEREO										0x0C33
#define GL_RENDER_MODE									0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT					0x0C50
#define GL_POINT_SMOOTH_HINT							0x0C51
#define GL_LINE_SMOOTH_HINT								0x0C52
#define GL_POLYGON_SMOOTH_HINT							0x0C53
#define GL_FOG_HINT										0x0C54
#define GL_TEXTURE_GEN_S								0x0C60
#define GL_TEXTURE_GEN_T								0x0C61
#define GL_TEXTURE_GEN_R								0x0C62
#define GL_TEXTURE_GEN_Q								0x0C63
#define GL_PIXEL_MAP_I_TO_I								0x0C70
#define GL_PIXEL_MAP_S_TO_S								0x0C71
#define GL_PIXEL_MAP_I_TO_R								0x0C72
#define GL_PIXEL_MAP_I_TO_G								0x0C73
#define GL_PIXEL_MAP_I_TO_B								0x0C74
#define GL_PIXEL_MAP_I_TO_A								0x0C75
#define GL_PIXEL_MAP_R_TO_R								0x0C76
#define GL_PIXEL_MAP_G_TO_G								0x0C77
#define GL_PIXEL_MAP_B_TO_B								0x0C78
#define GL_PIXEL_MAP_A_TO_A								0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE						0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE						0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE						0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE						0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE						0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE						0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE						0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE						0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE						0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE						0x0CB9
#define GL_UNPACK_SWAP_BYTES							0x0CF0
#define GL_UNPACK_LSB_FIRST								0x0CF1
#define GL_UNPACK_ROW_LENGTH							0x0CF2
#define GL_UNPACK_SKIP_ROWS								0x0CF3
#define GL_UNPACK_SKIP_PIXELS							0x0CF4
#define GL_UNPACK_ALIGNMENT								0x0CF5
#define GL_PACK_SWAP_BYTES								0x0D00
#define GL_PACK_LSB_FIRST								0x0D01
#define GL_PACK_ROW_LENGTH								0x0D02
#define GL_PACK_SKIP_ROWS								0x0D03
#define GL_PACK_SKIP_PIXELS								0x0D04
#define GL_PACK_ALIGNMENT								0x0D05
#define GL_MAP_COLOR									0x0D10
#define GL_MAP_STENCIL									0x0D11
#define GL_INDEX_SHIFT									0x0D12
#define GL_INDEX_OFFSET									0x0D13
#define GL_RED_SCALE									0x0D14
#define GL_RED_BIAS										0x0D15
#define GL_ZOOM_X										0x0D16
#define GL_ZOOM_Y										0x0D17
#define GL_GREEN_SCALE									0x0D18
#define GL_GREEN_BIAS									0x0D19
#define GL_BLUE_SCALE									0x0D1A
#define GL_BLUE_BIAS									0x0D1B
#define GL_ALPHA_SCALE									0x0D1C
#define GL_ALPHA_BIAS									0x0D1D
#define GL_DEPTH_SCALE									0x0D1E
#define GL_DEPTH_BIAS									0x0D1F
#define GL_MAX_EVAL_ORDER								0x0D30
#define GL_MAX_LIGHTS									0x0D31
#define GL_MAX_CLIP_DISTANCES							0x0D32
#define GL_MAX_TEXTURE_SIZE								0x0D33
#define GL_MAX_PIXEL_MAP_TABLE							0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH						0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH					0x0D36
#define GL_MAX_NAME_STACK_DEPTH							0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH					0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH						0x0D39
#define GL_MAX_VIEWPORT_DIMS							0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH				0x0D3B
#define GL_SUBPIXEL_BITS								0x0D50
#define GL_INDEX_BITS									0x0D51
#define GL_RED_BITS										0x0D52
#define GL_GREEN_BITS									0x0D53
#define GL_BLUE_BITS									0x0D54
#define GL_ALPHA_BITS									0x0D55
#define GL_DEPTH_BITS									0x0D56
#define GL_STENCIL_BITS									0x0D57
#define GL_ACCUM_RED_BITS								0x0D58
#define GL_ACCUM_GREEN_BITS								0x0D59
#define GL_ACCUM_BLUE_BITS								0x0D5A
#define GL_ACCUM_ALPHA_BITS								0x0D5B
#define GL_NAME_STACK_DEPTH								0x0D70
#define GL_AUTO_NORMAL									0x0D80
#define GL_MAP1_COLOR_4									0x0D90
#define GL_MAP1_INDEX									0x0D91
#define GL_MAP1_NORMAL									0x0D92
#define GL_MAP1_TEXTURE_COORD_1							0x0D93
#define GL_MAP1_TEXTURE_COORD_2							0x0D94
#define GL_MAP1_TEXTURE_COORD_3							0x0D95
#define GL_MAP1_TEXTURE_COORD_4							0x0D96
#define GL_MAP1_VERTEX_3								0x0D97
#define GL_MAP1_VERTEX_4								0x0D98
#define GL_MAP2_COLOR_4									0x0DB0
#define GL_MAP2_INDEX									0x0DB1
#define GL_MAP2_NORMAL									0x0DB2
#define GL_MAP2_TEXTURE_COORD_1							0x0DB3
#define GL_MAP2_TEXTURE_COORD_2							0x0DB4
#define GL_MAP2_TEXTURE_COORD_3							0x0DB5
#define GL_MAP2_TEXTURE_COORD_4							0x0DB6
#define GL_MAP2_VERTEX_3								0x0DB7
#define GL_MAP2_VERTEX_4								0x0DB8
#define GL_MAP1_GRID_DOMAIN								0x0DD0
#define GL_MAP1_GRID_SEGMENTS							0x0DD1
#define GL_MAP2_GRID_DOMAIN								0x0DD2
#define GL_MAP2_GRID_SEGMENTS							0x0DD3
#define GL_TEXTURE_1D									0x0DE0
#define GL_TEXTURE_2D									0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER						0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE							0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE							0x0DF2
#define GL_SELECTION_BUFFER_POINTER						0x0DF3
#define GL_SELECTION_BUFFER_SIZE						0x0DF4
#define GL_TEXTURE_WIDTH								0x1000
#define GL_TEXTURE_HEIGHT								0x1001
#define GL_TEXTURE_INTERNAL_FORMAT						0x1003
#define GL_TEXTURE_BORDER_COLOR							0x1004
#define GL_TEXTURE_BORDER								0x1005
#define GL_DONT_CARE									0x1100
#define GL_FASTEST										0x1101
#define GL_NICEST										0x1102
#define GL_LIGHT0										0x4000
#define GL_LIGHT1										0x4001
#define GL_LIGHT2										0x4002
#define GL_LIGHT3										0x4003
#define GL_LIGHT4										0x4004
#define GL_LIGHT5										0x4005
#define GL_LIGHT6										0x4006
#define GL_LIGHT7										0x4007
#define GL_AMBIENT										0x1200
#define GL_DIFFUSE										0x1201
#define GL_SPECULAR										0x1202
#define GL_POSITION										0x1203
#define GL_SPOT_DIRECTION								0x1204
#define GL_SPOT_EXPONENT								0x1205
#define GL_SPOT_CUTOFF									0x1206
#define GL_CONSTANT_ATTENUATION							0x1207
#define GL_LINEAR_ATTENUATION							0x1208
#define GL_QUADRATIC_ATTENUATION						0x1209
#define GL_COMPILE										0x1300
#define GL_COMPILE_AND_EXECUTE							0x1301
#define GL_CLEAR										0x1500
#define GL_AND											0x1501
#define GL_AND_REVERSE									0x1502
#define GL_COPY											0x1503
#define GL_AND_INVERTED									0x1504
#define GL_NOOP											0x1505
#define GL_XOR											0x1506
#define GL_OR											0x1507
#define GL_NOR											0x1508
#define GL_EQUIV										0x1509
#define GL_INVERT										0x150A
#define GL_OR_REVERSE									0x150B
#define GL_COPY_INVERTED								0x150C
#define GL_OR_INVERTED									0x150D
#define GL_NAND											0x150E
#define GL_SET											0x150F
#define GL_EMISSION										0x1600
#define GL_SHININESS									0x1601
#define GL_AMBIENT_AND_DIFFUSE							0x1602
#define GL_COLOR_INDEXES								0x1603
#define GL_MODELVIEW									0x1700
#define GL_PROJECTION									0x1701
#define GL_TEXTURE										0x1702
#define GL_COLOR										0x1800
#define GL_DEPTH										0x1801
#define GL_STENCIL										0x1802
#define GL_COLOR_INDEX									0x1900
#define GL_STENCIL_INDEX								0x1901
#define GL_DEPTH_COMPONENT								0x1902
#define GL_RED											0x1903
#define GL_GREEN										0x1904
#define GL_BLUE											0x1905
#define GL_ALPHA										0x1906
#define GL_RGB											0x1907
#define GL_RGBA											0x1908
#define GL_LUMINANCE									0x1909
#define GL_LUMINANCE_ALPHA								0x190A
#define GL_BITMAP										0x1A00
#define GL_POINT										0x1B00
#define GL_LINE											0x1B01
#define GL_FILL											0x1B02
#define GL_RENDER										0x1C00
#define GL_FEEDBACK										0x1C01
#define GL_SELECT										0x1C02
#define GL_FLAT											0x1D00
#define GL_SMOOTH										0x1D01
#define GL_KEEP											0x1E00
#define GL_REPLACE										0x1E01
#define GL_INCR											0x1E02
#define GL_DECR											0x1E03
#define GL_VENDOR										0x1F00
#define GL_RENDERER										0x1F01
#define GL_VERSION										0x1F02
#define GL_EXTENSIONS									0x1F03
#define GL_S											0x2000
#define GL_T											0x2001
#define GL_R											0x2002
#define GL_Q											0x2003
#define GL_MODULATE										0x2100
#define GL_DECAL										0x2101
#define GL_TEXTURE_ENV_MODE								0x2200
#define GL_TEXTURE_ENV_COLOR							0x2201
#define GL_TEXTURE_ENV									0x2300
#define GL_EYE_LINEAR									0x2400
#define GL_OBJECT_LINEAR								0x2401
#define GL_SPHERE_MAP									0x2402
#define GL_TEXTURE_GEN_MODE								0x2500
#define GL_OBJECT_PLANE									0x2501
#define GL_EYE_PLANE									0x2502
#define GL_NEAREST										0x2600
#define GL_LINEAR										0x2601
#define GL_NEAREST_MIPMAP_NEAREST						0x2700
#define GL_LINEAR_MIPMAP_NEAREST						0x2701
#define GL_NEAREST_MIPMAP_LINEAR						0x2702
#define GL_LINEAR_MIPMAP_LINEAR							0x2703
#define GL_TEXTURE_MAG_FILTER							0x2800
#define GL_TEXTURE_MIN_FILTER							0x2801
#define GL_TEXTURE_WRAP_S								0x2802
#define GL_TEXTURE_WRAP_T								0x2803
#define GL_CLAMP										0x2900
#define GL_REPEAT										0x2901

void APIENTRY glAccum(GLenum op, GLfloat value);
void APIENTRY glAlphaFunc(GLenum func, GLclampf ref);
void APIENTRY glBegin(GLenum mode);
void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap);
void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor);
void APIENTRY glCallList(GLuint list);
void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid* lists);
void APIENTRY glClear(GLbitfield mask);
void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void APIENTRY glClearDepth(GLclampd depth);
void APIENTRY glClearIndex(GLfloat c);
void APIENTRY glClearStencil(GLint s);
void APIENTRY glClipPlane(GLenum plane, const GLdouble* equation);
void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue);
void APIENTRY glColor3bv(const GLbyte* v);
void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue);
void APIENTRY glColor3dv(const GLdouble* v);
void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void APIENTRY glColor3fv(const GLfloat* v);
void APIENTRY glColor3i(GLint red, GLint green, GLint blue);
void APIENTRY glColor3iv(const GLint* v);
void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue);
void APIENTRY glColor3sv(const GLshort* v);
void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
void APIENTRY glColor3ubv(const GLubyte* v);
void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue);
void APIENTRY glColor3uiv(const GLuint* v);
void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue);
void APIENTRY glColor3usv(const GLushort* v);
void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void APIENTRY glColor4bv(const GLbyte* v);
void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void APIENTRY glColor4dv(const GLdouble* v);
void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glColor4fv(const GLfloat* v);
void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
void APIENTRY glColor4iv(const GLint* v);
void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
void APIENTRY glColor4sv(const GLshort* v);
void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void APIENTRY glColor4ubv(const GLubyte* v);
void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
void APIENTRY glColor4uiv(const GLuint* v);
void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void APIENTRY glColor4usv(const GLushort* v);
void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void APIENTRY glColorMaterial(GLenum face, GLenum mode);
void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void APIENTRY glCullFace(GLenum mode);
void APIENTRY glDeleteLists(GLuint list, GLsizei range);
void APIENTRY glDepthFunc(GLenum func);
void APIENTRY glDepthMask(GLboolean flag);
void APIENTRY glDepthRange(GLclampd zNear, GLclampd zFar);
void APIENTRY glDisable(GLenum cap);
void APIENTRY glDrawBuffer(GLenum mode);
void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void APIENTRY glEdgeFlag(GLboolean flag);
void APIENTRY glEdgeFlagv(const GLboolean* flag);
void APIENTRY glEnable(GLenum cap);
void APIENTRY glEnd(void);
void APIENTRY glEndList(void);
void APIENTRY glEvalCoord1d(GLdouble u);
void APIENTRY glEvalCoord1dv(const GLdouble* u);
void APIENTRY glEvalCoord1f(GLfloat u);
void APIENTRY glEvalCoord1fv(const GLfloat* u);
void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v);
void APIENTRY glEvalCoord2dv(const GLdouble* u);
void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v);
void APIENTRY glEvalCoord2fv(const GLfloat* u);
void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2);
void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void APIENTRY glEvalPoint1(GLint i);
void APIENTRY glEvalPoint2(GLint i, GLint j);
void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer);
void APIENTRY glFinish(void);
void APIENTRY glFlush(void);
void APIENTRY glFogf(GLenum pname, GLfloat param);
void APIENTRY glFogfv(GLenum pname, const GLfloat* params);
void APIENTRY glFogi(GLenum pname, GLint param);
void APIENTRY glFogiv(GLenum pname, const GLint* params);
void APIENTRY glFrontFace(GLenum mode);
void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint APIENTRY glGenLists(GLsizei range);
void APIENTRY glGetBooleanv(GLenum pname, GLboolean* params);
void APIENTRY glGetClipPlane(GLenum plane, GLdouble* equation);
void APIENTRY glGetDoublev(GLenum pname, GLdouble* params);
GLenum APIENTRY glGetError(void);
void APIENTRY glGetFloatv(GLenum pname, GLfloat* params);
void APIENTRY glGetIntegerv(GLenum pname, GLint* params);
void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat* params);
void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint* params);
void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble* v);
void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat* v);
void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint* v);
void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params);
void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint* params);
void APIENTRY glGetPixelMapfv(GLenum map, GLfloat* values);
void APIENTRY glGetPixelMapuiv(GLenum map, GLuint* values);
void APIENTRY glGetPixelMapusv(GLenum map, GLushort* values);
void APIENTRY glGetPolygonStipple(GLubyte* mask);
const GLubyte* APIENTRY glGetString(GLenum name);
void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params);
void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint* params);
void APIENTRY glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params);
void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params);
void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint* params);
void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels);
void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params);
void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params);
void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params);
void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params);
void APIENTRY glHint(GLenum target, GLenum mode);
void APIENTRY glIndexMask(GLuint mask);
void APIENTRY glIndexd(GLdouble c);
void APIENTRY glIndexdv(const GLdouble* c);
void APIENTRY glIndexf(GLfloat c);
void APIENTRY glIndexfv(const GLfloat* c);
void APIENTRY glIndexi(GLint c);
void APIENTRY glIndexiv(const GLint* c);
void APIENTRY glIndexs(GLshort c);
void APIENTRY glIndexsv(const GLshort* c);
void APIENTRY glInitNames(void);
GLboolean APIENTRY glIsEnabled(GLenum cap);
GLboolean APIENTRY glIsList(GLuint list);
void APIENTRY glLightModelf(GLenum pname, GLfloat param);
void APIENTRY glLightModelfv(GLenum pname, const GLfloat* params);
void APIENTRY glLightModeli(GLenum pname, GLint param);
void APIENTRY glLightModeliv(GLenum pname, const GLint* params);
void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param);
void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat* params);
void APIENTRY glLighti(GLenum light, GLenum pname, GLint param);
void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint* params);
void APIENTRY glLineStipple(GLint factor, GLushort pattern);
void APIENTRY glLineWidth(GLfloat width);
void APIENTRY glListBase(GLuint base);
void APIENTRY glLoadIdentity(void);
void APIENTRY glLoadMatrixd(const GLdouble* m);
void APIENTRY glLoadMatrixf(const GLfloat* m);
void APIENTRY glLoadName(GLuint name);
void APIENTRY glLogicOp(GLenum opcode);
void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points);
void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points);
void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points);
void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points);
void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param);
void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat* params);
void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param);
void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint* params);
void APIENTRY glMatrixMode(GLenum mode);
void APIENTRY glMultMatrixd(const GLdouble* m);
void APIENTRY glMultMatrixf(const GLfloat* m);
void APIENTRY glNewList(GLuint list, GLenum mode);
void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
void APIENTRY glNormal3bv(const GLbyte* v);
void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
void APIENTRY glNormal3dv(const GLdouble* v);
void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void APIENTRY glNormal3fv(const GLfloat* v);
void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz);
void APIENTRY glNormal3iv(const GLint* v);
void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz);
void APIENTRY glNormal3sv(const GLshort* v);
void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void APIENTRY glPassThrough(GLfloat token);
void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values);
void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values);
void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values);
void APIENTRY glPixelStoref(GLenum pname, GLfloat param);
void APIENTRY glPixelStorei(GLenum pname, GLint param);
void APIENTRY glPixelTransferf(GLenum pname, GLfloat param);
void APIENTRY glPixelTransferi(GLenum pname, GLint param);
void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor);
void APIENTRY glPointSize(GLfloat size);
void APIENTRY glPolygonMode(GLenum face, GLenum mode);
void APIENTRY glPolygonStipple(const GLubyte* mask);
void APIENTRY glPopAttrib(void);
void APIENTRY glPopMatrix(void);
void APIENTRY glPopName(void);
void APIENTRY glPushAttrib(GLbitfield mask);
void APIENTRY glPushMatrix(void);
void APIENTRY glPushName(GLuint name);
void APIENTRY glRasterPos2d(GLdouble x, GLdouble y);
void APIENTRY glRasterPos2dv(const GLdouble* v);
void APIENTRY glRasterPos2f(GLfloat x, GLfloat y);
void APIENTRY glRasterPos2fv(const GLfloat* v);
void APIENTRY glRasterPos2i(GLint x, GLint y);
void APIENTRY glRasterPos2iv(const GLint* v);
void APIENTRY glRasterPos2s(GLshort x, GLshort y);
void APIENTRY glRasterPos2sv(const GLshort* v);
void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glRasterPos3dv(const GLdouble* v);
void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glRasterPos3fv(const GLfloat* v);
void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z);
void APIENTRY glRasterPos3iv(const GLint* v);
void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z);
void APIENTRY glRasterPos3sv(const GLshort* v);
void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void APIENTRY glRasterPos4dv(const GLdouble* v);
void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void APIENTRY glRasterPos4fv(const GLfloat* v);
void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
void APIENTRY glRasterPos4iv(const GLint* v);
void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
void APIENTRY glRasterPos4sv(const GLshort* v);
void APIENTRY glReadBuffer(GLenum mode);
void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void APIENTRY glRectdv(const GLdouble* v1, const GLdouble* v2);
void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void APIENTRY glRectfv(const GLfloat* v1, const GLfloat* v2);
void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
void APIENTRY glRectiv(const GLint* v1, const GLint* v2);
void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void APIENTRY glRectsv(const GLshort* v1, const GLshort* v2);
GLint APIENTRY glRenderMode(GLenum mode);
void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY glSelectBuffer(GLsizei size, GLuint* buffer);
void APIENTRY glShadeModel(GLenum mode);
void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask);
void APIENTRY glStencilMask(GLuint mask);
void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
void APIENTRY glTexCoord1d(GLdouble s);
void APIENTRY glTexCoord1dv(const GLdouble* v);
void APIENTRY glTexCoord1f(GLfloat s);
void APIENTRY glTexCoord1fv(const GLfloat* v);
void APIENTRY glTexCoord1i(GLint s);
void APIENTRY glTexCoord1iv(const GLint* v);
void APIENTRY glTexCoord1s(GLshort s);
void APIENTRY glTexCoord1sv(const GLshort* v);
void APIENTRY glTexCoord2d(GLdouble s, GLdouble t);
void APIENTRY glTexCoord2dv(const GLdouble* v);
void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);
void APIENTRY glTexCoord2fv(const GLfloat* v);
void APIENTRY glTexCoord2i(GLint s, GLint t);
void APIENTRY glTexCoord2iv(const GLint* v);
void APIENTRY glTexCoord2s(GLshort s, GLshort t);
void APIENTRY glTexCoord2sv(const GLshort* v);
void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
void APIENTRY glTexCoord3dv(const GLdouble* v);
void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
void APIENTRY glTexCoord3fv(const GLfloat* v);
void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r);
void APIENTRY glTexCoord3iv(const GLint* v);
void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r);
void APIENTRY glTexCoord3sv(const GLshort* v);
void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void APIENTRY glTexCoord4dv(const GLdouble* v);
void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void APIENTRY glTexCoord4fv(const GLfloat* v);
void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
void APIENTRY glTexCoord4iv(const GLint* v);
void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
void APIENTRY glTexCoord4sv(const GLshort* v);
void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param);
void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params);
void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param);
void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params);
void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param);
void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble* params);
void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param);
void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params);
void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param);
void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint* params);
void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param);
void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params);
void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param);
void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params);
void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glVertex2d(GLdouble x, GLdouble y);
void APIENTRY glVertex2dv(const GLdouble* v);
void APIENTRY glVertex2f(GLfloat x, GLfloat y);
void APIENTRY glVertex2fv(const GLfloat* v);
void APIENTRY glVertex2i(GLint x, GLint y);
void APIENTRY glVertex2iv(const GLint* v);
void APIENTRY glVertex2s(GLshort x, GLshort y);
void APIENTRY glVertex2sv(const GLshort* v);
void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z);
void APIENTRY glVertex3dv(const GLdouble* v);
void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glVertex3fv(const GLfloat* v);
void APIENTRY glVertex3i(GLint x, GLint y, GLint z);
void APIENTRY glVertex3iv(const GLint* v);
void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z);
void APIENTRY glVertex3sv(const GLshort* v);
void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void APIENTRY glVertex4dv(const GLdouble* v);
void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void APIENTRY glVertex4fv(const GLfloat* v);
void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w);
void APIENTRY glVertex4iv(const GLint* v);
void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
void APIENTRY glVertex4sv(const GLshort* v);
void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#endif		/* GL_VERSION_1_0 */

#ifdef __cplusplus
}
#endif

#endif			/* _GL10_H */
