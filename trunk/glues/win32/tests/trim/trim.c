/*                                                              */
/* This test is based on SGI's trim.c example from redbook      */
/* Copyright (c) 1993-1997, Silicon Graphics, Inc.              */
/* ALL RIGHTS RESERVED                                          */
/*                                                              */
/* initialization part on PowerVR SDK examples                  */
/*                                                              */
/* // Mike Gorchak, 2009. GLU ES test                           */
/*                                                              */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <windows.h>
#include <TCHAR.h>

#include <GLES\gl.h>
#include <GLES\egl.h>

#include "glues.h"

#define WINDOW_WIDTH   640
#define WINDOW_HEIGHT  480

GLfloat mat_ambient[4]={0.2f, 0.2f, 0.2f, 0.2f};
GLfloat mat_diffuse[]={0.7f, 0.7f, 0.7f, 1.0f};
GLfloat mat_specular[]={1.0f, 1.0f, 1.0f, 1.0f};
GLfloat mat_shininess[]={100.0f};

GLfloat ctlpoints[4][4][3];

GLfloat knots[8]={0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
/* counter clockwise */
GLfloat edgePt[5][2]={{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}};
/* clockwise */
GLfloat curvePt[4][2]={{0.25f, 0.5f}, {0.25f, 0.75f}, {0.75f, 0.75f}, {0.75f, 0.5f}};
GLfloat curveKnots[8]={0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f};
/* clockwise */
GLfloat pwlPt[4][2]={{0.75f, 0.5f}, {0.5f, 0.25f}, {0.25f, 0.5f}};

GLUnurbsObj* nurb;

#define	WINDOW_CLASS _T("GLUES")

void init_scene(int width, int height)
{
   int u, v;

   /* Setup our viewport for OpenGL ES */
   glViewport(0, 0, (GLint)width, (GLint)height);
   /* Setup our viewport for GLU ES (required when using OpenGL ES 1.0 only) */
   gluViewport(0, 0, (GLint)width, (GLint)height);

   /* Set black background color */
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);

   /* Since GL ES has no GL_AUTO_NORMAL, we implement it in GLU */
   gluEnable(GLU_AUTO_NORMAL);

   nurb=gluNewNurbsRenderer();
   gluNurbsProperty(nurb, GLU_SAMPLING_TOLERANCE, 25.0f);
   gluNurbsProperty(nurb, GLU_DISPLAY_MODE, GLU_FILL);

   for (u=0; u<4; u++)
   {
      for (v=0; v<4; v++)
      {
         ctlpoints[u][v][0]=2.0f*((GLfloat)u-1.5f);
         ctlpoints[u][v][1]=2.0f*((GLfloat)v-1.5f);

         if ((u==1 || u==2) && (v==1 || v==2))
         {
            ctlpoints[u][v][2]=3.0f;
         }
         else
         {
            ctlpoints[u][v][2]=-3.0f;
         }
      }
   }

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 3.0f, 24.0f);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0f, 0.5f, -9.0f);
   glRotatef(330.0f, 1.0f, 0.0f, 0.0f);
}

void resize(int width, int height)
{
   /* Avoid division by zero */
   if (height==0)
   {
      height=1;
   }

   /* Setup our new viewport for OpenGL ES */
   glViewport(0, 0, (GLint)width, (GLint)height);
   /* Setup our new viewport for GLU ES (required when using OpenGL ES 1.0 only) */
   gluViewport(0, 0, (GLint)width, (GLint)height);

   /* Setup new aspect ratio */
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 3.0f, 24.0f);
   glMatrixMode(GL_MODELVIEW);
}

void render_scene()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   /* Render trimmed surface */
   gluBeginSurface(nurb);
      gluNurbsSurface(nurb, 8, knots, 8, knots, 4*3, 3, &ctlpoints[0][0][0], 4, 4, GLU_MAP2_VERTEX_3);
      gluBeginTrim(nurb);
         gluPwlCurve(nurb, 5, &edgePt[0][0], 2, GLU_MAP1_TRIM_2);
      gluEndTrim(nurb);
      gluBeginTrim(nurb);
         gluNurbsCurve(nurb, 8, curveKnots, 2, &curvePt[0][0], 4, GLU_MAP1_TRIM_2);
         gluPwlCurve (nurb, 3, &pwlPt[0][0], 2, GLU_MAP1_TRIM_2);
      gluEndTrim(nurb);
   gluEndSurface(nurb);

   /* Flush all drawings */
   glFlush();
}

int TestEGLError(HWND hWnd, char* pszLocation)
{
   /*
      eglGetError returns the last error that has happened using egl,
      not the status of the last called function. The user has to
      check after every single egl call or at least once every frame.
   */

   EGLint iErr=eglGetError();
   if (iErr!=EGL_SUCCESS)
   {
      TCHAR pszStr[256];
      swprintf_s(pszStr, 256, _T("%s failed (%d).\n"), pszLocation, iErr);
      MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      return 0;
   }

   return 1;
}

int done=0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      // Handles the close message when a user clicks the quit icon of the window
      case WM_CLOSE:
           done=1;
           PostQuitMessage(0);
           return 1;

      default:
	       break;
   }

   // Calls the default window procedure for messages we did not handle
   return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
   int i;

   // Windows variables
   HWND hWnd=0;
   HDC  hDC=0;

   // EGL variables
   EGLDisplay eglDisplay=0;
   EGLConfig  eglConfig=0;
   EGLSurface eglSurface=0;
   EGLContext eglContext=0;
   NativeWindowType eglWindow=0;
   EGLint pi32ConfigAttribs[128];
   unsigned int nWidth;
   unsigned int nHeight;
   ATOM registerClass;
   RECT sRect;
   EGLint iMajorVersion, iMinorVersion;
   int iConfigs;

   /*
      Step 0 - Create a NativeWindowType that we can use for OpenGL ES output
   */

   // Register the windows class
   WNDCLASS sWC;

   sWC.style=CS_HREDRAW | CS_VREDRAW;
   sWC.lpfnWndProc=WndProc;
   sWC.cbClsExtra=0;
   sWC.cbWndExtra=0;
   sWC.hInstance=hInstance;
   sWC.hIcon=0;
   sWC.hCursor=0;
   sWC.lpszMenuName=0;
   sWC.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
   sWC.lpszClassName=WINDOW_CLASS;

   nWidth=WINDOW_WIDTH;
   nHeight=WINDOW_HEIGHT;

   registerClass=RegisterClass(&sWC);
   if (!registerClass)
   {
      MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
   }

   // Create the eglWindow
   SetRect(&sRect, 0, 0, nWidth, nHeight);
   AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, 0, 0);
   hWnd=CreateWindow(WINDOW_CLASS, _T("SDL GLU ES Nurbs Trim test"), WS_VISIBLE | WS_SYSMENU, 0, 0, nWidth, nHeight, NULL, NULL, hInstance, NULL);
   eglWindow=hWnd;

   // Get the associated device context
   hDC=GetDC(hWnd);
   if (!hDC)
   {
      MessageBox(0, _T("Failed to create the device context"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 1 - Get the default display.
      EGL uses the concept of a "display" which in most environments
      corresponds to a single physical screen. Since we usually want
      to draw to the main screen or only have a single screen to begin
      with, we let EGL pick the default display.
      Querying other displays is platform specific.
   */
   eglDisplay=eglGetDisplay((NativeDisplayType)hDC);

   if(eglDisplay==EGL_NO_DISPLAY)
   {
      eglDisplay=eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
   }

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
      If we're not interested in the EGL version number we can just
      pass NULL for the second and third parameters.
   */

   if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
   {
      MessageBox(0, _T("eglInitialize() failed."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 3 - Specify the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   i=0;

   pi32ConfigAttribs[i++]=EGL_RED_SIZE;
   pi32ConfigAttribs[i++]=5;
   pi32ConfigAttribs[i++]=EGL_GREEN_SIZE;
   pi32ConfigAttribs[i++]=6;
   pi32ConfigAttribs[i++]=EGL_BLUE_SIZE;
   pi32ConfigAttribs[i++]=5;
   pi32ConfigAttribs[i++]=EGL_ALPHA_SIZE;
   pi32ConfigAttribs[i++]=0;
   pi32ConfigAttribs[i++]=EGL_DEPTH_SIZE;
   pi32ConfigAttribs[i++]=16;
   pi32ConfigAttribs[i++]=EGL_SURFACE_TYPE;
   pi32ConfigAttribs[i++]=EGL_WINDOW_BIT;
   pi32ConfigAttribs[i++]=EGL_NONE;

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument. In most cases we just want the first config that meets
      all criteria, so we can limit the number of configs returned to 1.
   */
   if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
   {
      MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle when available to create a window surface. A window surface
      is one that will be visible on screen inside the native display (or
      fullscreen if there is no windowing system).
      Pixmaps and pbuffers are surfaces which only exist in off-screen
      memory.
   */
   eglSurface=eglCreateWindowSurface(eglDisplay, eglConfig, eglWindow, NULL);

   if (eglSurface==EGL_NO_SURFACE)
   {
      eglGetError(); // Clear error
      eglSurface=eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
   }

   if (!TestEGLError(hWnd, "eglCreateWindowSurface"))
   {
      goto cleanup;
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context
      (or shared contexts)
   */
   eglContext=eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
   if (!TestEGLError(hWnd, "eglCreateContext"))
   {
      goto cleanup;
   }

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      Contexts are bound to a thread. This means you don't have to
      worry about other threads and processes interfering with your
      OpenGL ES application.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
   if (!TestEGLError(hWnd, "eglMakeCurrent"))
   {
      goto cleanup;
   }

   /* Initialize scene */
   init_scene(WINDOW_WIDTH, WINDOW_HEIGHT);

   /* Render stuff */
   do {
      MSG msg;

	  render_scene();
      glFinish();
      eglWaitGL();
      eglSwapBuffers(eglDisplay, eglSurface);
	  if (done)
	  {
         break;
	  }

      PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE);
      TranslateMessage(&msg);
      DispatchMessage(&msg);

   } while(1);

   /* Destroy NURBS renderer */
   gluDeleteNurbsRenderer(nurb);

   /*
      Step 8 - Terminate OpenGL ES and destroy the window (if present).
      eglTerminate takes care of destroying any context or surface created
      with this display, so we don't need to call eglDestroySurface or
      eglDestroyContext here.
   */
cleanup:
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   /*
      Step 9 - Destroy the eglWindow.
      Again, this is platform specific and delegated to a separate function.
   */

   // Release the device context
   if (hDC)
   {
      ReleaseDC(hWnd, hDC);
   }

   // Destroy the eglWindow
   if (hWnd)
   {
      DestroyWindow(hWnd);
   }

   return 0;
}
