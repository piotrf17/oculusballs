#include <algorithm>
#include <cstdio>
#include <memory>

#define OVR_OS_LINUX

#include "Src/OVR_CAPI.h"
#include "Src/OVR_CAPI_GL.h"
#include "Src/Kernel/OVR_Math.h"

#include "gl.h"

using namespace OVR;

/*
Texture* CreateTexture(int format, int width, int height, const void* data, int mipcount)
{
    GLenum   glformat, gltype = GL_UNSIGNED_BYTE;
    switch(format & Texture_TypeMask)
    {
    case Texture_RGBA:  glformat = GL_RGBA; break;
    case Texture_R:     glformat = GL_RED; break;
    case Texture_Depth: glformat = GL_DEPTH_COMPONENT32F; gltype = GL_FLOAT; break;
    case Texture_DXT1:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
    case Texture_DXT3:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
    case Texture_DXT5:  glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    default:
        return NULL;
    }
    Texture* NewTex = new Texture(this, width, height);
    glBindTexture(GL_TEXTURE_2D, NewTex->TexId);
    OVR_ASSERT(!glGetError());
    
    if (format & Texture_Compressed)
    {
        const unsigned char* level = (const unsigned char*)data;
        int w = width, h = height;
        for (int i = 0; i < mipcount; i++)
        {
            int mipsize = GetTextureSize(format, w, h);
            glCompressedTexImage2D(GL_TEXTURE_2D, i, glformat, w, h, 0, mipsize, level);

            level += mipsize;
            w >>= 1;
            h >>= 1;
            if (w < 1) w = 1;
            if (h < 1) h = 1;
        }
    }
    else if (format & Texture_Depth)
        glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, GL_DEPTH_COMPONENT, gltype, data);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, gltype, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (format == (Texture_RGBA|Texture_GenMipmaps)) // not render target
    {
        int srcw = width, srch = height;
        int level = 0;
        UByte* mipmaps = NULL;
        do
        {
            level++;
            int mipw = srcw >> 1; if (mipw < 1) mipw = 1;
            int miph = srch >> 1; if (miph < 1) miph = 1;
            if (mipmaps == NULL)
                mipmaps = (UByte*)OVR_ALLOC(mipw * miph * 4);
            FilterRgba2x2(level == 1 ? (const UByte*)data : mipmaps, srcw, srch, mipmaps);
            glTexImage2D(GL_TEXTURE_2D, level, glformat, mipw, miph, 0, glformat, gltype, mipmaps);
            srcw = mipw;
            srch = miph;
        } while (srcw > 1 || srch > 1);
        if (mipmaps)
            OVR_FREE(mipmaps);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount-1);
    }

    OVR_ASSERT(!glGetError());
    return NewTex;
}
*/

int main(int argc, char** argv) {
  // Initializes LibOVR.
  ovr_Initialize();
  
  ovrHmd hmd = ovrHmd_Create(0);
  ovrHmdDesc hmd_desc;
  if (hmd) {
    // Get more details about the HMD
    ovrHmd_GetDesc(hmd, &hmd_desc);
    printf("Got something! It's a %s made by %s.\n", hmd_desc.ProductName,
           hmd_desc.Manufacturer);
  } else {
    printf("Boners everywhere.\n");
    return 1;
  }

  /*
  // Configure Stereo settings.
  ovrSizei recommenedTex0Size = ovrHmd_GetFovTextureSize(
      hmd, ovrEye_Left, hmdDesc.DefaultEyeFov[0], 1.0f);
  ovrSizei recommenedTex1Size = ovrHmd_GetFovTextureSize(
      hmd, ovrEye_Right, hmdDesc.DefaultEyeFov[1], 1.0f);
  ovrSizei renderTargetSize;
  renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
  renderTargetSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);
  const int eyeRenderMultisample = 1;
  pRendertargetTexture = pRender->CreateTexture(
      Texture_RGBA | Texture_RenderTarget | eyeRenderMultisample,
      renderTargetSize.Width, renderTargetSize.Height, NULL);
  // The actual RT size may be different due to HW limits.
  renderTargetSize.w = pRendertargetTexture->GetWidth();
  renderTargetSize.h = pRendertargetTexture->GetHeight();
  */

  // Configure OpenGL.
  std::unique_ptr<GLWindow> win(CreateGLWindow(800, 600, "balls"));
  glXMakeCurrent(win->dpy, win->win, win->ctx);

  // Configure Oculus OpenGL integration.
  ovrGLConfig cfg;
  cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
  cfg.OGL.Header.RTSize = Sizei(hmd_desc.Resolution.w,
                                hmd_desc.Resolution.h);
  cfg.OGL.Header.Multisample = 1;
  cfg.OGL.Win = win->win;
  cfg.OGL.Disp = win->dpy;
  ovrEyeRenderDesc eye_render_desc[2];
  if (!ovrHmd_ConfigureRendering(hmd,
                                 &cfg.Config,
                                 hmd_desc.DistortionCaps,
                                 hmd_desc.DefaultEyeFov,
                                 eye_render_desc)) {
    printf("failed to configure rendering, tits.\n");
    return 1;
  }

  // Shutdown.
  DestroyGLWindow(win.get());
  ovrHmd_Destroy(hmd);
  ovr_Shutdown();
  
  return 0;
}
