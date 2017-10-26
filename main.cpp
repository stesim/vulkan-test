#include "common.h"
#include "window.h"
#include "windowsurface.h"
#include "renderer.h"

#include <string>

void renderCallback( Window& window, void* userData )
{
	Renderer& renderer = *reinterpret_cast<Renderer*>( userData );

	uint32_t imageIndex = renderer.renderFrame();

	if( imageIndex != Renderer::INVALID_FRAME )
	{
		renderer.presentFrame( imageIndex );
	}
	else
	{
		log_warning( "Dropping invalid frame." );
	}

	renderer.waitForIdle(); // TODO: remove?

	renderer.updateUniforms();
}

void resizeCallback( Window& window, uint32_t width, uint32_t height, void* userData )
{
	log_info( "Window resized to:" );
	log_info( std::to_string( width ) + "x" + std::to_string( height ) );

	if( width > 0 && height > 0 )
	{
		//Renderer& renderer = *reinterpret_cast<Renderer*>( userData );

		// window resize results in out-of-date swap chain
		// recreating here may result in unnecessary swap chain recreation
		// (possibly depends on windowing system?)

		//renderer.recreateSwapchain();
	}
}

int main()
{
	if( Window::initializeWindowSystem() &&
	    Renderer::initializeRenderingSystem( Window::queryRequiredRendererExtensions() ) )
	{
		Window window;

		{
			WindowSurface windowSurface( window );
			Renderer renderer( windowSurface );

			window.setMainLoopCallback( renderCallback, &renderer );
			window.setResizeCallback( resizeCallback, &renderer );

			window.startMainLoop();

			renderer.waitForIdle();
		}

		Renderer::terminateRenderingSystem();
		Window::terminateWindowSystem();
	}
	else
	{
		log_error( "Cannot initialize GLFW." );
	}

	return 0;
}
