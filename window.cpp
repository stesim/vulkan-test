#include "window.h"

Window::Window()
    : m_pWindow( nullptr ),
      m_iWidth( 800 ),
      m_iHeight( 600 ),
      m_pCallback( nullptr ),
      m_pCallbackUserData( nullptr ),
      m_ResizeCallback()
{
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	m_pWindow = glfwCreateWindow( m_iWidth, m_iHeight, "vulkan-test", nullptr, nullptr );

	glfwSetWindowUserPointer( m_pWindow, this );
	glfwSetWindowSizeCallback( m_pWindow, Window::onResizeStatic );
}

Window::~Window()
{
	if( m_pWindow != nullptr )
	{
		glfwDestroyWindow( m_pWindow );
	}
}


bool Window::initializeWindowSystem()
{
	return ( glfwInit() == GLFW_TRUE );
}

void Window::terminateWindowSystem()
{
	glfwTerminate();
}

std::vector<const char*> Window::queryRequiredRendererExtensions()
{
	uint32_t numExtensions;
	const char** extensions = glfwGetRequiredInstanceExtensions( &numExtensions );

	return std::vector<const char*>( extensions, extensions + numExtensions );
}

void Window::startMainLoop()
{
	while( !glfwWindowShouldClose( m_pWindow ) )
	{
		glfwPollEvents();

		if( m_pCallback != nullptr )
		{
			m_pCallback( *this, m_pCallbackUserData );
		}
	}
}

uint32_t Window::getWidth()
{
	int width;
	glfwGetFramebufferSize( m_pWindow, &width, nullptr );
	return width;
}

uint32_t Window::getHeight()
{
	int height;
	glfwGetFramebufferSize( m_pWindow, nullptr, &height );
	return height;
}

void Window::onResizeStatic( GLFWwindow* window, int width, int height )
{
	reinterpret_cast<Window*>( glfwGetWindowUserPointer( window ) )->onResize( width, height );
}

void Window::onResize( int width, int height )
{
	// TODO: remove m_iWidth/Height members?
	// getWidth/Height query GLFW directly
	m_iWidth  = width;
	m_iHeight = height;

	if( m_ResizeCallback )
	{
		m_ResizeCallback( *this, width, height );
	}
}

