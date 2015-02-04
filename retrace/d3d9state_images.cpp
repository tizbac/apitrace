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


#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "image.hpp"
#include "json.hpp"
#include "com_ptr.hpp"
#include "d3d9state.hpp"
#include "d3dstate.hpp"
#include <d3dx9.h>

namespace d3dstate {


static image::Image *
getSurfaceImage(IDirect3DDevice9 *pDevice,
                IDirect3DSurface9 *pSurface) {
    image::Image *image = NULL;
    HRESULT hr;

    if (!pSurface) {
        return NULL;
    }

    D3DSURFACE_DESC Desc;
    hr = pSurface->GetDesc(&Desc);
    assert(SUCCEEDED(hr));

    
    
    D3DLOCKED_RECT LockedRect;
    hr = pSurface->LockRect(&LockedRect, NULL, D3DLOCK_READONLY);
    if (FAILED(hr)) {
        return NULL;
    }

    image = ConvertImage(Desc.Format, LockedRect.pBits, LockedRect.Pitch, Desc.Width, Desc.Height);

    pSurface->UnlockRect();

    if (!image){ //Convert failed , do stretchrect to a known format to retrieve data
        
        IDirect3DSurface9 * dumpSurf;
        IDirect3DTexture9 * tex;
        pDevice->CreateTexture(Desc.Width, Desc.Height,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT, &tex, NULL);
        tex->GetSurfaceLevel(0,&dumpSurf);
        assert(dumpSurf);
        RECT srect;
        srect.left = 0;
        srect.top = 0;
        srect.right = Desc.Width;
        srect.bottom = Desc.Height;
        RECT drect;
        drect.left = 0;
        drect.top = 0;
        drect.right = Desc.Width;
        drect.bottom = Desc.Height;
        hr = pDevice->StretchRect(pSurface,&srect,dumpSurf,&drect,D3DTEXF_NONE);
        if (SUCCEEDED(hr)) {
        dumpSurf->LockRect(&LockedRect, NULL, D3DLOCK_READONLY);
        image = ConvertImage(D3DFMT_A8R8G8B8, LockedRect.pBits, LockedRect.Pitch, Desc.Width, Desc.Height);
        dumpSurf->UnlockRect();
        
        }else{
            dumpSurf->Release();
            tex->Release();
            pDevice->CreateTexture(Desc.Width, Desc.Height,1,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM, &tex, NULL);
            tex->GetSurfaceLevel(0,&dumpSurf);
            pDevice->GetRenderTargetData(pSurface,dumpSurf);
            dumpSurf->LockRect(&LockedRect, NULL, D3DLOCK_READONLY);
            image = ConvertImage(D3DFMT_A8R8G8B8, LockedRect.pBits, LockedRect.Pitch, Desc.Width, Desc.Height);
            dumpSurf->UnlockRect();
            dumpSurf->Release();
            tex->Release();
        }
        
    }
    return image;
}


static image::Image *
getRenderTargetImage(IDirect3DDevice9 *pDevice,
                     IDirect3DSurface9 *pRenderTarget) {
    HRESULT hr;

    if (!pRenderTarget) {
        return NULL;
    }

    D3DSURFACE_DESC Desc;
    hr = pRenderTarget->GetDesc(&Desc);
    assert(SUCCEEDED(hr));

    com_ptr<IDirect3DSurface9> pStagingSurface;
    hr = pDevice->CreateOffscreenPlainSurface(Desc.Width, Desc.Height, Desc.Format, D3DPOOL_SYSTEMMEM, &pStagingSurface, NULL);
    if (FAILED(hr)) {
        return NULL;
    }

    hr = pDevice->GetRenderTargetData(pRenderTarget, pStagingSurface);
    if (FAILED(hr)) {
        std::cerr << "warning: GetRenderTargetData failed\n";
        return NULL;
    }

    return getSurfaceImage(pDevice, pStagingSurface);
}


image::Image *
getRenderTargetImage(IDirect3DDevice9 *pDevice) {
    HRESULT hr;

    com_ptr<IDirect3DSurface9> pRenderTarget;
    hr = pDevice->GetRenderTarget(0, &pRenderTarget);
    if (FAILED(hr)) {
        return NULL;
    }
    assert(pRenderTarget);

    return getRenderTargetImage(pDevice, pRenderTarget);
}


static image::Image *
getTextureImage(IDirect3DDevice9 *pDevice,
                IDirect3DBaseTexture9 *pTexture,
                D3DCUBEMAP_FACES FaceType,
                UINT Level)
{
    HRESULT hr;

    if (!pTexture) {
        return NULL;
    }

    com_ptr<IDirect3DSurface9> pSurface;

    D3DRESOURCETYPE Type = pTexture->GetType();
    switch (Type) {
    case D3DRTYPE_TEXTURE:
        assert(FaceType == D3DCUBEMAP_FACE_POSITIVE_X);
        hr = reinterpret_cast<IDirect3DTexture9 *>(pTexture)->GetSurfaceLevel(Level, &pSurface);
        break;
    case D3DRTYPE_CUBETEXTURE:
        hr = reinterpret_cast<IDirect3DCubeTexture9 *>(pTexture)->GetCubeMapSurface(FaceType, Level, &pSurface);
        break;
    default:
        /* TODO: support volume textures */
        return NULL;
    }
    if (FAILED(hr) || !pSurface) {
        return NULL;
    }

    D3DSURFACE_DESC Desc;
    hr = pSurface->GetDesc(&Desc);
    assert(SUCCEEDED(hr));

    if (Desc.Pool != D3DPOOL_DEFAULT ||
        Desc.Usage & D3DUSAGE_DYNAMIC) {
        // Lockable texture
        return getSurfaceImage(pDevice, pSurface);
    } else if (Desc.Usage & D3DUSAGE_RENDERTARGET) {
        // Rendertarget texture
        return getRenderTargetImage(pDevice, pSurface);
    } else {
        // D3D constraints are such there is not much else that can be done.
        return NULL;
    }
}


void
dumpTextures(JSONWriter &json, IDirect3DDevice9 *pDevice)
{
    HRESULT hr;

    json.beginMember("textures");
    json.beginObject();

    for (DWORD Stage = 0; Stage < 16; ++Stage) {
        com_ptr<IDirect3DBaseTexture9> pTexture;
        hr = pDevice->GetTexture(Stage, &pTexture);
        if (FAILED(hr)) {
            continue;
        }

        if (!pTexture) {
            continue;
        }

        D3DRESOURCETYPE Type = pTexture->GetType();

        DWORD NumFaces = Type == D3DRTYPE_CUBETEXTURE ? 6 : 1;
        DWORD NumLevels = pTexture->GetLevelCount();

        for (DWORD Face = 0; Face < NumFaces; ++Face) {
            for (DWORD Level = 0; Level < NumLevels; ++Level) {
                image::Image *image;
                image = getTextureImage(pDevice, pTexture, static_cast<D3DCUBEMAP_FACES>(Face), Level);
                if (image) {
                    char label[128];
                    if (Type == D3DRTYPE_CUBETEXTURE) {
                        _snprintf(label, sizeof label, "PS_RESOURCE_%lu_FACE_%lu_LEVEL_%lu", Stage, Face, Level);
                    } else {
                        _snprintf(label, sizeof label, "PS_RESOURCE_%lu_LEVEL_%lu", Stage, Level);
                    }
                    json.beginMember(label);
                    json.writeImage(image);
                    json.endMember(); // PS_RESOURCE_*
                }
            }
        }
    }

    json.endObject();
    json.endMember(); // textures
}

#define FOURCC_RESZ ((D3DFORMAT)(MAKEFOURCC('R','E','S','Z')))
#define RESZ_CODE 0x7fa05000
void
dumpFramebuffer(JSONWriter &json, IDirect3DDevice9 *pDevice)
{
    HRESULT hr;

    json.beginMember("framebuffer");
    json.beginObject();

    D3DCAPS9 Caps;
    pDevice->GetDeviceCaps(&Caps);

    for (UINT i = 0; i < Caps.NumSimultaneousRTs; ++i) {
        com_ptr<IDirect3DSurface9> pRenderTarget;
        hr = pDevice->GetRenderTarget(i, &pRenderTarget);
        if (FAILED(hr)) {
            continue;
        }

        if (!pRenderTarget) {
            continue;
        }

        image::Image *image;
        image = getRenderTargetImage(pDevice, pRenderTarget);
        if (image) {
            char label[64];
            _snprintf(label, sizeof label, "RENDER_TARGET_%u", i);
            json.beginMember(label);
            json.writeImage(image);
            json.endMember(); // RENDER_TARGET_*
        }
    }

    IDirect3DSurface9 * pDepthStencil = NULL;

    BOOL bRESZSupported = 1;
    D3DVIEWPORT9 vp;
    pDevice->GetViewport(&vp);
    hr = pDevice->GetDepthStencilSurface(&pDepthStencil);
    if ( bRESZSupported )
    {
        IDirect3DTexture9 * oldtex;
        std::cerr << "Doing resz" << std::endl;
        IDirect3DTexture9 * lockabletex;
        pDevice->CreateTexture(vp.Width,vp.Height, 1, D3DUSAGE_DEPTHSTENCIL, D3DFMT_D16_LOCKABLE,D3DPOOL_DEFAULT,&lockabletex, NULL);
        pDevice->GetTexture(0, (IDirect3DBaseTexture9**)(&oldtex));
        pDevice->SetTexture(0, lockabletex);
        D3DXVECTOR3 vDummyPoint(0.0f, 0.0f, 0.0f);
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
        pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, 1, vDummyPoint, 
        sizeof(D3DXVECTOR3));
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0F);
        
        pDevice->SetRenderState(D3DRS_POINTSIZE, RESZ_CODE);
        pDevice->SetRenderState(D3DRS_POINTSIZE, 0x0);
        pDevice->SetTexture(0, oldtex);
        
        lockabletex->GetSurfaceLevel(0,&pDepthStencil);
    }
    std::cerr << "SUCC:" << SUCCEEDED(hr) << " " << pDepthStencil << std::endl;
    if (pDepthStencil) {
        image::Image *image;
        image = getSurfaceImage(pDevice, pDepthStencil);
        if (image) {
            json.beginMember("DEPTH_STENCIL");
            json.writeImage(image);
            json.endMember(); // RENDER_TARGET_*
        }
    }

    json.endObject();
    json.endMember(); // framebuffer
}


} /* namespace d3dstate */
