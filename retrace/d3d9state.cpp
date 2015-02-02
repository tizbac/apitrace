/**************************************************************************
 *
 * Copyright 2011 Jose Fonseca
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <stdio.h>

#include <iostream>

#include "json.hpp"
#include "com_ptr.hpp"
#include "d3d9imports.hpp"
#include "d3dshader.hpp"
#include "d3dstate.hpp"


namespace d3dstate {


template< class T >
inline void
dumpShader(JSONWriter &json, const char *name, T *pShader) {
    if (!pShader) {
        return;
    }

    HRESULT hr;

    UINT SizeOfData = 0;

    hr = pShader->GetFunction(NULL, &SizeOfData);
    if (SUCCEEDED(hr)) {
        void *pData;
        pData = malloc(SizeOfData);
        if (pData) {
            hr = pShader->GetFunction(pData, &SizeOfData);
            if (SUCCEEDED(hr)) {
                com_ptr<IDisassemblyBuffer> pDisassembly;
                hr = DisassembleShader((const DWORD *)pData, &pDisassembly);
                if (SUCCEEDED(hr)) {
                    json.beginMember(name);
                    json.writeString((const char *)pDisassembly->GetBufferPointer() /*, pDisassembly->GetBufferSize() */);
                    json.endMember();
                }

            }
            free(pData);
        }
    }
}

static void
dumpShaders(JSONWriter &json, IDirect3DDevice9 *pDevice)
{
    json.beginMember("shaders");

    HRESULT hr;
    json.beginObject();

    com_ptr<IDirect3DVertexShader9> pVertexShader;
    hr = pDevice->GetVertexShader(&pVertexShader);
    if (SUCCEEDED(hr)) {
        dumpShader<IDirect3DVertexShader9>(json, "vertex", pVertexShader);
    }

    com_ptr<IDirect3DPixelShader9> pPixelShader;
    hr = pDevice->GetPixelShader(&pPixelShader);
    if (SUCCEEDED(hr)) {
        dumpShader<IDirect3DPixelShader9>(json, "pixel", pPixelShader);
    }

    json.endObject();
    json.endMember(); // shaders
}

void
dumpDevice(std::ostream &os, IDirect3DDevice9 *pDevice)
{
    JSONWriter json(os);

    /* TODO */
    json.beginMember("parameters");
    json.beginObject();
    DWORD rsvalue;
    rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ZENABLE,&rsvalue);
json.beginMember("D3DRS_ZENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FILLMODE,&rsvalue);
json.beginMember("D3DRS_FILLMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SHADEMODE,&rsvalue);
json.beginMember("D3DRS_SHADEMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ZWRITEENABLE,&rsvalue);
json.beginMember("D3DRS_ZWRITEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ALPHATESTENABLE,&rsvalue);
json.beginMember("D3DRS_ALPHATESTENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_LASTPIXEL,&rsvalue);
json.beginMember("D3DRS_LASTPIXEL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SRCBLEND,&rsvalue);
json.beginMember("D3DRS_SRCBLEND");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DESTBLEND,&rsvalue);
json.beginMember("D3DRS_DESTBLEND");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CULLMODE,&rsvalue);
json.beginMember("D3DRS_CULLMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ZFUNC,&rsvalue);
json.beginMember("D3DRS_ZFUNC");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ALPHAREF,&rsvalue);
json.beginMember("D3DRS_ALPHAREF");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ALPHAFUNC,&rsvalue);
json.beginMember("D3DRS_ALPHAFUNC");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DITHERENABLE,&rsvalue);
json.beginMember("D3DRS_DITHERENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE,&rsvalue);
json.beginMember("D3DRS_ALPHABLENDENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGENABLE,&rsvalue);
json.beginMember("D3DRS_FOGENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SPECULARENABLE,&rsvalue);
json.beginMember("D3DRS_SPECULARENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGCOLOR,&rsvalue);
json.beginMember("D3DRS_FOGCOLOR");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGTABLEMODE,&rsvalue);
json.beginMember("D3DRS_FOGTABLEMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGSTART,&rsvalue);
json.beginMember("D3DRS_FOGSTART");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGEND,&rsvalue);
json.beginMember("D3DRS_FOGEND");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGDENSITY,&rsvalue);
json.beginMember("D3DRS_FOGDENSITY");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_RANGEFOGENABLE,&rsvalue);
json.beginMember("D3DRS_RANGEFOGENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILENABLE,&rsvalue);
json.beginMember("D3DRS_STENCILENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILFAIL,&rsvalue);
json.beginMember("D3DRS_STENCILFAIL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILZFAIL,&rsvalue);
json.beginMember("D3DRS_STENCILZFAIL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILPASS,&rsvalue);
json.beginMember("D3DRS_STENCILPASS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILFUNC,&rsvalue);
json.beginMember("D3DRS_STENCILFUNC");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILREF,&rsvalue);
json.beginMember("D3DRS_STENCILREF");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILMASK,&rsvalue);
json.beginMember("D3DRS_STENCILMASK");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_STENCILWRITEMASK,&rsvalue);
json.beginMember("D3DRS_STENCILWRITEMASK");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_TEXTUREFACTOR,&rsvalue);
json.beginMember("D3DRS_TEXTUREFACTOR");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP0,&rsvalue);
json.beginMember("D3DRS_WRAP0");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP1,&rsvalue);
json.beginMember("D3DRS_WRAP1");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP2,&rsvalue);
json.beginMember("D3DRS_WRAP2");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP3,&rsvalue);
json.beginMember("D3DRS_WRAP3");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP4,&rsvalue);
json.beginMember("D3DRS_WRAP4");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP5,&rsvalue);
json.beginMember("D3DRS_WRAP5");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP6,&rsvalue);
json.beginMember("D3DRS_WRAP6");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP7,&rsvalue);
json.beginMember("D3DRS_WRAP7");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CLIPPING,&rsvalue);
json.beginMember("D3DRS_CLIPPING");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_LIGHTING,&rsvalue);
json.beginMember("D3DRS_LIGHTING");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_AMBIENT,&rsvalue);
json.beginMember("D3DRS_AMBIENT");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_FOGVERTEXMODE,&rsvalue);
json.beginMember("D3DRS_FOGVERTEXMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_COLORVERTEX,&rsvalue);
json.beginMember("D3DRS_COLORVERTEX");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_LOCALVIEWER,&rsvalue);
json.beginMember("D3DRS_LOCALVIEWER");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_NORMALIZENORMALS,&rsvalue);
json.beginMember("D3DRS_NORMALIZENORMALS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,&rsvalue);
json.beginMember("D3DRS_DIFFUSEMATERIALSOURCE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SPECULARMATERIALSOURCE,&rsvalue);
json.beginMember("D3DRS_SPECULARMATERIALSOURCE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_AMBIENTMATERIALSOURCE,&rsvalue);
json.beginMember("D3DRS_AMBIENTMATERIALSOURCE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,&rsvalue);
json.beginMember("D3DRS_EMISSIVEMATERIALSOURCE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_VERTEXBLEND,&rsvalue);
json.beginMember("D3DRS_VERTEXBLEND");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CLIPPLANEENABLE,&rsvalue);
json.beginMember("D3DRS_CLIPPLANEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSIZE,&rsvalue);
json.beginMember("D3DRS_POINTSIZE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSIZE_MIN,&rsvalue);
json.beginMember("D3DRS_POINTSIZE_MIN");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSPRITEENABLE,&rsvalue);
json.beginMember("D3DRS_POINTSPRITEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSCALEENABLE,&rsvalue);
json.beginMember("D3DRS_POINTSCALEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSCALE_A,&rsvalue);
json.beginMember("D3DRS_POINTSCALE_A");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSCALE_B,&rsvalue);
json.beginMember("D3DRS_POINTSCALE_B");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSCALE_C,&rsvalue);
json.beginMember("D3DRS_POINTSCALE_C");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS,&rsvalue);
json.beginMember("D3DRS_MULTISAMPLEANTIALIAS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_MULTISAMPLEMASK,&rsvalue);
json.beginMember("D3DRS_MULTISAMPLEMASK");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_PATCHEDGESTYLE,&rsvalue);
json.beginMember("D3DRS_PATCHEDGESTYLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DEBUGMONITORTOKEN,&rsvalue);
json.beginMember("D3DRS_DEBUGMONITORTOKEN");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POINTSIZE_MAX,&rsvalue);
json.beginMember("D3DRS_POINTSIZE_MAX");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE,&rsvalue);
json.beginMember("D3DRS_INDEXEDVERTEXBLENDENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_COLORWRITEENABLE,&rsvalue);
json.beginMember("D3DRS_COLORWRITEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_TWEENFACTOR,&rsvalue);
json.beginMember("D3DRS_TWEENFACTOR");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_BLENDOP,&rsvalue);
json.beginMember("D3DRS_BLENDOP");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_POSITIONDEGREE,&rsvalue);
json.beginMember("D3DRS_POSITIONDEGREE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_NORMALDEGREE,&rsvalue);
json.beginMember("D3DRS_NORMALDEGREE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SCISSORTESTENABLE,&rsvalue);
json.beginMember("D3DRS_SCISSORTESTENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,&rsvalue);
json.beginMember("D3DRS_SLOPESCALEDEPTHBIAS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ANTIALIASEDLINEENABLE,&rsvalue);
json.beginMember("D3DRS_ANTIALIASEDLINEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_MINTESSELLATIONLEVEL,&rsvalue);
json.beginMember("D3DRS_MINTESSELLATIONLEVEL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_MAXTESSELLATIONLEVEL,&rsvalue);
json.beginMember("D3DRS_MAXTESSELLATIONLEVEL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ADAPTIVETESS_X,&rsvalue);
json.beginMember("D3DRS_ADAPTIVETESS_X");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ADAPTIVETESS_Y,&rsvalue);
json.beginMember("D3DRS_ADAPTIVETESS_Y");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ADAPTIVETESS_Z,&rsvalue);
json.beginMember("D3DRS_ADAPTIVETESS_Z");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ADAPTIVETESS_W,&rsvalue);
json.beginMember("D3DRS_ADAPTIVETESS_W");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_ENABLEADAPTIVETESSELLATION,&rsvalue);
json.beginMember("D3DRS_ENABLEADAPTIVETESSELLATION");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_TWOSIDEDSTENCILMODE,&rsvalue);
json.beginMember("D3DRS_TWOSIDEDSTENCILMODE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CCW_STENCILFAIL,&rsvalue);
json.beginMember("D3DRS_CCW_STENCILFAIL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CCW_STENCILZFAIL,&rsvalue);
json.beginMember("D3DRS_CCW_STENCILZFAIL");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CCW_STENCILPASS,&rsvalue);
json.beginMember("D3DRS_CCW_STENCILPASS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_CCW_STENCILFUNC,&rsvalue);
json.beginMember("D3DRS_CCW_STENCILFUNC");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_COLORWRITEENABLE1,&rsvalue);
json.beginMember("D3DRS_COLORWRITEENABLE1");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_COLORWRITEENABLE2,&rsvalue);
json.beginMember("D3DRS_COLORWRITEENABLE2");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_COLORWRITEENABLE3,&rsvalue);
json.beginMember("D3DRS_COLORWRITEENABLE3");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_BLENDFACTOR,&rsvalue);
json.beginMember("D3DRS_BLENDFACTOR");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE,&rsvalue);
json.beginMember("D3DRS_SRGBWRITEENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DEPTHBIAS,&rsvalue);
json.beginMember("D3DRS_DEPTHBIAS");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP8,&rsvalue);
json.beginMember("D3DRS_WRAP8");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP9,&rsvalue);
json.beginMember("D3DRS_WRAP9");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP10,&rsvalue);
json.beginMember("D3DRS_WRAP10");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP11,&rsvalue);
json.beginMember("D3DRS_WRAP11");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP12,&rsvalue);
json.beginMember("D3DRS_WRAP12");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP13,&rsvalue);
json.beginMember("D3DRS_WRAP13");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP14,&rsvalue);
json.beginMember("D3DRS_WRAP14");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_WRAP15,&rsvalue);
json.beginMember("D3DRS_WRAP15");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SEPARATEALPHABLENDENABLE,&rsvalue);
json.beginMember("D3DRS_SEPARATEALPHABLENDENABLE");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_SRCBLENDALPHA,&rsvalue);
json.beginMember("D3DRS_SRCBLENDALPHA");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_DESTBLENDALPHA,&rsvalue);
json.beginMember("D3DRS_DESTBLENDALPHA");
json.writeInt(rsvalue);
json.endMember();

rsvalue = 0xffffffff;
pDevice->GetRenderState(D3DRS_BLENDOPALPHA,&rsvalue);
json.beginMember("D3DRS_BLENDOPALPHA");
json.writeInt(rsvalue);
json.endMember();

    
    json.endObject();
    json.endMember(); // parameters

    dumpShaders(json, pDevice);

    dumpTextures(json, pDevice);

    dumpFramebuffer(json, pDevice);
}


} /* namespace d3dstate */
