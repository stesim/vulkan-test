#ifndef WINDOW_H
#define WINDOW_H

#include "common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class Window
{
public:
	typedef void ( *LoopCallback )( Window&, void* );
	//typedef void ( *ResizeCallback )( Window&, uint32_t, uint32_t, void* );
	typedef PayloadCallback<void*, void, Window&, uint32_t, uint32_t> ResizeCallback;

public:
	Window();
	~Window();

	static bool initializeWindowSystem();
	static void terminateWindowSystem();

	static std::vector<const char*> queryRequiredRendererExtensions();

	GLFWwindow* getNativeHandle()
	{
		return m_pWindow;
	}

	void     startMainLoop();

	uint32_t getWidth();
	uint32_t getHeight();

	void     setMainLoopCallback( LoopCallback callback, void* userData )
	{
		m_pCallback         = callback;
		m_pCallbackUserData = userData;
	}

	void     setResizeCallback( ResizeCallback::FunctionPtr callback, void* userData )
	{
		m_ResizeCallback = { callback, userData };
	}

private:
	static void   onResizeStatic( GLFWwindow* window, int width, int height );

	void          onResize( int width, int height );

private:
	GLFWwindow*   m_pWindow;
	int           m_iWidth;
	int           m_iHeight;

	LoopCallback  m_pCallback;
	void*         m_pCallbackUserData;

	ResizeCallback m_ResizeCallback;
};

#endif // WINDOW_H
