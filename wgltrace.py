##########################################################################
#
# Copyright 2008-2009 VMware, Inc.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/


"""WGL tracing code generator."""


from stdapi import API
from glapi import glapi
from wglapi import wglapi
from dispatch import function_pointer_type, function_pointer_value
from gltrace import GlTracer
from codegen import *


class WglTracer(GlTracer):

    extensions = [
        # GL_VERSION_1_2
        "GL_EXT_bgra",
        "GL_EXT_draw_range_elements",
        "GL_EXT_packed_pixels",
        "GL_EXT_rescale_normal",
        "GL_EXT_separate_specular_color",
        "GL_EXT_texture3D",
        "GL_SGIS_texture_edge_clamp",
        "GL_SGIS_texture_lod",

        # GL_VERSION_1_3
        "GL_ARB_multisample",
        "GL_ARB_multitexture",
        "GL_ARB_texture_border_clamp",
        "GL_ARB_texture_compression",
        "GL_ARB_texture_cube_map", # GL_EXT_texture_cube_map
        "GL_ARB_texture_env_add", #"GL_EXT_texture_env_add",
        "GL_ARB_texture_env_dot3",
        "GL_ARB_texture_env_combine",
        "GL_ARB_transpose_matrix",

        # GL_VERSION_1_4
        "GL_ARB_depth_texture",
        "GL_ARB_point_parameters", #"GL_EXT_point_parameters",
        "GL_ARB_shadow",
        "GL_ARB_texture_env_crossbar",
        "GL_ARB_texture_mirrored_repeat",
        "GL_ARB_window_pos",
        "GL_EXT_blend_color",
        "GL_EXT_blend_func_separate",
        "GL_EXT_blend_minmax",
        "GL_EXT_blend_subtract",
        "GL_EXT_fog_coord",
        "GL_EXT_multi_draw_arrays",
        "GL_EXT_secondary_color",
        "GL_EXT_stencil_wrap",
        "GL_EXT_texture_lod_bias",
        "GL_SGIS_generate_mipmap",
        "GL_NV_blend_square",

        # GL_VERSION_1_5
        "GL_ARB_occlusion_query",
        "GL_ARB_vertex_buffer_object",
        "GL_EXT_shadow_funcs",

        # GL_VERSION_2_0
        "GL_ARB_draw_buffers",
        "GL_ARB_fragment_program",
        "GL_ARB_fragment_shader",
        "GL_ARB_point_sprite",
        "GL_ARB_shader_objects",
        "GL_ARB_shading_language_100",
        "GL_ARB_texture_non_power_of_two",
        "GL_ARB_vertex_program",
        "GL_ARB_vertex_shader",
        "GL_EXT_blend_equation_separate",
        "GL_EXT_stencil_two_side",
    
        # GL_VERSION_2_1
        "GL_ARB_pixel_buffer_object", "GL_EXT_pixel_buffer_object",
        "GL_EXT_texture_sRGB",

        # XXX
        "GL_ARB_framebuffer_object", "GL_EXT_framebuffer_object",
        "GL_EXT_framebuffer_multisample",
        "GL_EXT_packed_depth_stencil",
        #"GL_EXT_texture_compression_s3tc",
        #"GL_S3_s3tc",
    ]

    wgl_extensions = [
        "WGL_ARB_extensions_string",
        "WGL_ARB_multisample",
        "WGL_ARB_pbuffer",
        "WGL_ARB_pixel_format",
        "WGL_EXT_extensions_string",
    ]

    def dispatch_function(self, function):
        GlTracer.dispatch_function(self, function)

        if function.name == 'glGetString':
            print '    if (__result) {'
            print '        switch (name) {'
            print '        case GL_VENDOR:'
            print '            __result = (const GLubyte*)"VMware, Inc.";'
            print '            break;'
            print '        case GL_VERSION:'
            print '            __result = (const GLubyte*)"2.1";'
            print '            break;'
            print '        case GL_SHADING_LANGUAGE_VERSION:'
            print '            __result = (const GLubyte*)"1.2";'
            print '            break;'
            print '        case GL_EXTENSIONS:'
            print '            __result = (const GLubyte*)"%s";' % ' '.join(self.extensions)
            print '            break;'
            print '        default:'
            print '            break;'
            print '        }'
            print '    }'

        if function.name == 'wglGetExtensionsStringARB':
            print '    if (__result) {'
            print '        __result = "%s";' % ' '.join(self.wgl_extensions)
            print '    }'

    def wrap_ret(self, function, instance):
        GlTracer.wrap_ret(self, function, instance)

        if function.name == "wglGetProcAddress":
            print '    if (%s) {' % instance
        
            func_dict = dict([(f.name, f) for f in glapi.functions + wglapi.functions])

            def handle_case(function_name):
                f = func_dict[function_name]
                ptype = function_pointer_type(f)
                pvalue = function_pointer_value(f)
                print '    %s = (%s)%s;' % (pvalue, ptype, instance)
                print '    %s = (%s)&%s;' % (instance, function.type, f.name);
        
            def handle_default():
                print '    OS::DebugMessage("apitrace: warning: unknown function \\"%s\\"\\n", lpszProc);'
                print '    %s = (%s)NULL;' % (instance, function.type);

            string_switch('lpszProc', func_dict.keys(), handle_case, handle_default)
            print '    }'


if __name__ == '__main__':
    print
    print '#define _GDI32_'
    print
    print '#include <string.h>'
    print '#include <windows.h>'
    print
    print '#include "trace_writer.hpp"'
    print '#include "os.hpp"'
    print
    print '''
static HINSTANCE g_hDll = NULL;

static PROC
__getPublicProcAddress(LPCSTR lpProcName)
{
    if (!g_hDll) {
        char szDll[MAX_PATH] = {0};
        
        if (!GetSystemDirectoryA(szDll, MAX_PATH)) {
            return NULL;
        }
        
        strcat(szDll, "\\\\opengl32.dll");
        
        g_hDll = LoadLibraryA(szDll);
        if (!g_hDll) {
            return NULL;
        }
    }
        
    return GetProcAddress(g_hDll, lpProcName);
}

    '''
    print '// To validate our prototypes'
    print '#define GL_GLEXT_PROTOTYPES'
    print '#define WGL_GLXEXT_PROTOTYPES'
    print
    print '#include "glproc.hpp"'
    print '#include "glsize.hpp"'
    print
    api = API()
    api.add_api(glapi)
    api.add_api(wglapi)
    tracer = WglTracer()
    tracer.trace_api(api)
