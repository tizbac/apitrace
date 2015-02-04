import sys
import subprocess
import random
def getParameter(s,parameter):
  x = s.split("%s = "%parameter)
  if len(x) < 2:
    return None
  return x[1].split(",")[0].split(")")[0].strip("& \t\n")

tracefile = sys.argv[1]
frame = int(sys.argv[2])

current_rendertargets = [None]*16
current_textures = [None]*16
curframe = 0
p = subprocess.Popen(["apitrace","dump",tracefile],stdout=subprocess.PIPE)
texture_surface_ownership = {} # Texture -> Surface
surface_texture_ownership = {} # Texture -> Surface
texture_format = {}
surface_format = {}
texture_sizes = {}
surface_sizes = {}
orphanrt = []
curshader = "NULL"
rt_usages = {}
resz = []
renderdeps = []
rt = None
ds = "DEFAULT_DEPTH"
for line in iter(p.stdout.readline,''):
  s = line.split(" ")
  if len(s) < 2:
    continue
  callname = s[1].split("(")[0]
  if not callname.startswith("IDirect"):
    continue
  callno = int(s[0])
  if callname == "IDirect3DDevice9::Present" or callname == "IDirect3DDevice9::PresentEx":
    if curframe == frame:
      graphsurf = []
      usedlinks = []
      emittedshaders = []
      print "digraph \"G\" {"
      for x in renderdeps:
        if not x[0] in graphsurf:
          graphsurf.append(x[0])
        if not x[2] in graphsurf:
          graphsurf.append(x[2])
        if not (x[0],x[1]) in usedlinks:
          usedlinks.append((x[0],x[1]))
          print "\"%s\" -> \"%s\" [color=\"#%02x%02x%02x\"];"%(x[0],x[1],random.randint(0,200),random.randint(0,200),random.randint(0,200))
        if not (x[1],x[2]) in usedlinks:
          usedlinks.append((x[1],x[2]))
          print "\"%s\" -> \"%s\" [color=\"#%02x%02x%02x\"];"%(x[1],x[2],random.randint(0,200),random.randint(0,200),random.randint(0,200))
        if not x[1] in emittedshaders:
          print "\"%s\" [label=\"Shader%s\" shape=box];"%(x[1],x[1])
          emittedshaders.append(x[1])
      for x in resz:
        if not x[0] in graphsurf:
          graphsurf.append(x[0])
        if not x[1] in graphsurf:
          graphsurf.append(x[1])  
        print "\"%s\" -> \"%s\" [label=\"RESZ\"];"%(x[0],x[1])
      print "\"%s\" [label=\"Backbuffer\"];"%rt
      for x in graphsurf:
        if x == rt:
          continue
        print "\"%s\" [label=\"%s\\n%s\\n%dx%d\"];"%(x,x,surface_format[x] if x in surface_format else "UNKFMT",
                                                     surface_sizes[x][0] if x in surface_sizes else -1,
                                                     surface_sizes[x][1] if x in surface_sizes else -1
                                                     )
      print "}"
      p.kill()
      exit(0)
    sys.stderr.write("\rFrame %6d    "%(curframe+1))
    sys.stderr.flush()
    curframe += 1
  if callname == "IDirect3DDevice9::CreateTexture":
    fmt = getParameter(line,"Format")
    tex = getParameter(line,"ppTexture")
    w = int(getParameter(line,"Width"))
    h = int(getParameter(line,"Height"))
    if "D3DUSAGE_RENDERTARGET" in getParameter(line,"Usage"):
      rt_usages[tex] = True
    texture_format[tex] = fmt
    texture_sizes[tex] = (w,h)
  if callname == "IDirect3DDevice9::CreateDepthStencilSurface":
    fmt = getParameter(line,"Format")
    surf = getParameter(line,"ppSurface")
    w = int(getParameter(line,"Width"))
    h = int(getParameter(line,"Height"))
    surface_format[surf] = fmt
    surface_sizes[surf] = (w,h)
  if callname == "IDirect3DTexture9::GetSurfaceLevel":
    texture_surface_ownership[getParameter(line,"this")] = getParameter(line,"ppSurfaceLevel")
    surface_texture_ownership[getParameter(line,"ppSurfaceLevel")] = getParameter(line,"this")
    level = int(getParameter(line,"Level"))
    surface_format[getParameter(line,"ppSurfaceLevel")] = texture_format[getParameter(line,"this")]
    surface_sizes[getParameter(line,"ppSurfaceLevel")] = (texture_sizes[getParameter(line,"this")][0]/(2**level),texture_sizes[getParameter(line,"this")][1]/(2**level))
  #print callno,callname
  if curframe == frame:
    if callname == "IDirect3DDevice9::SetRenderTarget":
      rt = getParameter(line,"pRenderTarget")
      if not rt in surface_texture_ownership:
        orphanrt.append(rt)
    if callname == "IDirect3DDevice9::SetPixelShader":
      curshader = getParameter(line,"pShader")
    if callname == "IDirect3DDevice9::SetDepthStencilSurface":
      zs = getParameter(line,"pNewZStencil")
      if not zs in surface_texture_ownership:
        orphanrt.append(zs)
    if callname == "IDirect3DDevice9::SetRenderState":
      #print getParameter(line,"State"),getParameter(line,"Value")
      if getParameter(line,"State") == "D3DRS_POINTSIZE" and getParameter(line,"Value") == "2141212672":#RESZ
        
        if zs and zs != "NULL":
          resz.append((current_textures[0],zs))
    if callname == "IDirect3DDevice9::SetTexture":
      tex = getParameter(line,"pTexture")
      stage = int(getParameter(line,"Stage"))
      if stage > 16:
        sys.stderr.write("Warning: the game is trying to use invalid stage %d\n"%stage)
      else:
        current_textures[stage] = tex
    if callname.startswith("IDirect3DDevice9::Draw"):
      for tex in current_textures:
        if tex != "NULL" and tex in rt_usages and tex in texture_surface_ownership:
          if rt and rt != "NULL":
            renderdeps.append((texture_surface_ownership[tex],curshader,rt))
          if zs and zs != "NULL":
            renderdeps.append((texture_surface_ownership[tex],curshader,zs))
    
    

p.wait()