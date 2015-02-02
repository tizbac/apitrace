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


#include <string.h>

#include "os_string.hpp"

#include "d3dstate.hpp"
#include "retrace.hpp"
#include "d3dretrace.hpp"
#include <d3d9.h>
#include <stdio.h>
static bool supportsElapsed = true;
IDirect3DDevice9* dx9_dev = NULL;
IDirect3DQuery9* dx9_freq = NULL;
IDirect3DQuery9* dx9_start = NULL;
IDirect3DQuery9* dx9_end = NULL;
uint64_t callstart = 0;
void
retrace::setFeatureLevel(const char *featureLevel) {
    /* TODO: Allow to override D3D feature level. */
}

static inline uint64_t getCurrentTime()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    unsigned long long tt = ft.dwHighDateTime;
    tt <<=32;
    tt |= ft.dwLowDateTime;
    tt /=10;
    tt -= 11644473600000000ULL;
    return tt;
}

void
retrace::setUp(void) {
}


void
retrace::addCallbacks(retrace::Retracer &retracer)
{
    retracer.addCallbacks(d3dretrace::ddraw_callbacks);
    retracer.addCallbacks(d3dretrace::d3d8_callbacks);
    retracer.addCallbacks(d3dretrace::d3d9_callbacks);
    retracer.addCallbacks(d3dretrace::dxgi_callbacks);
}


void
retrace::flushRendering(void) {
}

void
retrace::finishRendering(void) {
}

void
retrace::waitForInput(void) {
    /* TODO */
}
extern d3dretrace::D3DDumper<IDirect3DDevice9> d3d9Dumper;
void d3dretrace::beginProfileDX9(trace::Call &call, bool isDraw)
{
    if (!dx9_start)
    {
        d3d9Dumper.pLastDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP,&dx9_start);
        d3d9Dumper.pLastDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP,&dx9_end);
        d3d9Dumper.pLastDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ,&dx9_freq);
    }
   /* dx9_freq->Issue(D3DISSUE_BEGIN);
    dx9_freq->Issue(D3DISSUE_END);*/
    dx9_start->Issue(D3DISSUE_BEGIN);
    dx9_start->Issue(D3DISSUE_END);
    callstart = getCurrentTime();
}

void d3dretrace::endProfileDX9(trace::Call &call, bool isDraw)
{
    UINT64 freq=10000,ttstart,ttend;
    UINT64 cpuend = getCurrentTime();
    dx9_end->Issue(D3DISSUE_BEGIN);
    dx9_end->Issue(D3DISSUE_END);

   // while( dx9_freq->GetData(&freq, sizeof(UINT64),0) != S_OK) {}
    while( dx9_start->GetData(&ttstart, sizeof(UINT64),0) != S_OK) {}
    while( dx9_end->GetData(&ttend, sizeof(UINT64),0) != S_OK) {}
    retrace::profiler.addCall(call.no,call.sig->name,0,0,((double)ttstart/(double)freq)*1000000,(((double)(ttend-ttstart))/(double)freq)*1000000,callstart*1000,(cpuend-callstart)*1000,0,0,0,0);
}



void
retrace::cleanUp(void) {
}
