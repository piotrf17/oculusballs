#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>

#include <GL/glew.h>

#define OVR_OS_LINUX

#include "Src/OVR_CAPI.h"
#include "Src/OVR_CAPI_GL.h"
#include "Src/Kernel/OVR_Math.h"

#include "gl.h"

using namespace OVR;

GLuint CreateRenderTexture(int width, int height) {
  GLuint tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  return tex_id;
}

void SetRenderTarget(GLuint framebuffer, GLuint color_tex_id) {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  assert(!glGetError());
  printf("color_tex_id = %d\n", color_tex_id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         color_tex_id, 0);
  printf("err0r = %d\n", glGetError());
  assert(false);
  assert(!glGetError());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, 0);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    printf("Framebu, status=%d\n", status);
  }
}

void Clear() {
  glClearColor(0, 0, 0, 0);
  glClearDepth(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SetViewport(int wh, const Recti& vp) {
  glViewport(vp.x, wh - vp.y - vp.h, vp.w, vp.h);
}


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

  // Create a texture for rendering.
  ovrSizei recommened_tex0_size = ovrHmd_GetFovTextureSize(
      hmd, ovrEye_Left, hmd_desc.DefaultEyeFov[0], 1.0f);
  ovrSizei recommened_tex1_size = ovrHmd_GetFovTextureSize(
      hmd, ovrEye_Right, hmd_desc.DefaultEyeFov[1], 1.0f);
  ovrSizei render_target_size;
  render_target_size.w = recommened_tex0_size.w + recommened_tex1_size.w;
  render_target_size.h = std::max(recommened_tex0_size.h,
                                  recommened_tex1_size.h);
  GLuint render_tex_id = CreateRenderTexture(render_target_size.w,
                                             render_target_size.h);

  // Configure OpenGL.
  std::unique_ptr<GLWindow> win(CreateGLWindow(800, 600, "balls"));
  glXMakeCurrent(win->dpy, win->win, win->ctx);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    printf("GLEW is not dry\n");
    return 1;
  }
  
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);

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

  // Setup eye textures.
  ovrGLTexture eye_texture[2];
  eye_texture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
  eye_texture[0].OGL.Header.TextureSize = render_target_size;
  eye_texture[0].OGL.Header.RenderViewport =
      eye_render_desc[0].DistortedViewport;
  eye_texture[0].OGL.TexId = render_tex_id;
  eye_texture[1] = eye_texture[0];
  eye_texture[1].OGL.Header.RenderViewport =
      eye_render_desc[1].DistortedViewport;
  
  // Render a frame.
  ovrFrameTiming hmd_frame_timing = ovrHmd_BeginFrame(hmd, 0);
  SetRenderTarget(framebuffer, render_tex_id);
  Clear();
  for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++) {
    ovrEyeType eye = hmd_desc.EyeRenderOrder[eyeIndex];
    ovrPosef eye_pose = ovrHmd_BeginEyeRender(hmd, eye);
    Quatf orientation = Quatf(eye_pose.Orientation);
    Matrix4f proj = ovrMatrix4f_Projection(eye_render_desc[eye].Fov,
                                           0.01f, 10000.0f, true);
    // Test logic - assign quaternion result directly to view; TBD: Add translation.
    Matrix4f view = Matrix4f(orientation.Inverted());
    //        *Matrix4f::Translation(-eye_pose);
    SetViewport(render_target_size.h, eye_render_desc[eye].DistortedViewport);
    //pRender->SetProjection(proj);
    //pRoomScene->Render(pRender, Matrix4f::Translation(EyeRenderDesc[eye].ViewAdjust) * view);

    // draw some shit.
    
    
    ovrHmd_EndEyeRender(hmd, eye, eye_pose, &eye_texture[eye].Texture);
  }
  
  // Let OVR do distortion rendering, Present and flush/sync.
  ovrHmd_EndFrame(hmd);

  // Shutdown.
  glDeleteFramebuffers(1, &framebuffer);
  DestroyGLWindow(win.get());
  ovrHmd_Destroy(hmd);
  ovr_Shutdown();
  
  return 0;
}
