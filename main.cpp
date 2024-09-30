// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// ImGui
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

// standard
#include <iostream>
#include <sstream>

#include "MyApp.h"

int main( int argc, char* args[] )
{

	SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR);
	if ( SDL_Init( SDL_INIT_VIDEO ) == -1 )
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[SDL initialization] Error during the SDL initialization: %s", SDL_GetError());
		return 1;
	}

	std::atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef _DEBUG 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif 

	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,         32);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,            8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,          8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,           8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,          8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,		1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,          24);

	SDL_Window *win = nullptr;
	win = SDL_CreateWindow( "Hello SDL&OpenGL!",		
							100,						
							100,						
							800,						
							600,						
							SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);			


	if (win == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[Window creation] Error during the SDL initialization: %s", SDL_GetError());
		return 1;
	}

	SDL_GLContext	context = SDL_GL_CreateContext(win);
	if (context == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[OGL context creation] Error during the creation of the OGL context: %s", SDL_GetError());
		return 1;
	}	

	SDL_GL_SetSwapInterval(1);

	GLenum error = glewInit();
	if ( error != GLEW_OK )
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[GLEW] Error during the initialization of glew.");
		return 1;
	}

	int glVersion[2] = {-1, -1}; 
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]); 
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]); 

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Running OpenGL %d.%d", glVersion[0], glVersion[1]);

	if ( glVersion[0] == -1 && glVersion[1] == -1 )
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow( win );

		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[OGL context creation] Error during the inialization of the OGL context! Maybe one of the SDL_GL_SetAttribute(...) calls is erroneous.");
		
		return 1;
	}

	std::stringstream window_title;
	window_title << "OpenGL " << glVersion[0] << "." << glVersion[1];
	SDL_SetWindowTitle(win, window_title.str().c_str());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(win, context);
	ImGui_ImplOpenGL3_Init();

	{
		bool quit = false;
		SDL_Event ev;

		CMyApp app;
		if (!app.Init())
		{
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(win);
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[app.Init] Error during the initialization of the application!");
			return 1;
		}

		while (!quit)
		{
			while ( SDL_PollEvent(&ev) )
			{
				ImGui_ImplSDL2_ProcessEvent(&ev);
				bool is_mouse_captured    = ImGui::GetIO().WantCaptureMouse;    
				bool is_keyboard_captured = ImGui::GetIO().WantCaptureKeyboard;	

				switch (ev.type)
				{
					case SDL_QUIT:
						quit = true;
						break;
					case SDL_KEYDOWN:
						if ( ev.key.keysym.sym == SDLK_ESCAPE )
							quit = true;

						if ( ( ev.key.keysym.sym == SDLK_RETURN )  
							 && ( ev.key.keysym.mod & KMOD_ALT )   
							 && !( ev.key.keysym.mod & ( KMOD_SHIFT | KMOD_CTRL | KMOD_GUI ) ) )
						{
							Uint32 FullScreenSwitchFlag = ( SDL_GetWindowFlags( win ) & SDL_WINDOW_FULLSCREEN_DESKTOP ) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
							SDL_SetWindowFullscreen( win, FullScreenSwitchFlag );
						}
						if ( !is_keyboard_captured )
							app.KeyboardDown(ev.key);
						break;
					case SDL_KEYUP:
						if ( !is_keyboard_captured )
							app.KeyboardUp(ev.key);
						break;
					case SDL_MOUSEBUTTONDOWN:
						if ( !is_mouse_captured )
							app.MouseDown(ev.button);
						break;
					case SDL_MOUSEBUTTONUP:
						if ( !is_mouse_captured )
							app.MouseUp(ev.button);
						break;
					case SDL_MOUSEWHEEL:
						if ( !is_mouse_captured )
							app.MouseWheel(ev.wheel);
						break;
					case SDL_MOUSEMOTION:
						if ( !is_mouse_captured )
							app.MouseMove(ev.motion);
						break;
					case SDL_WINDOWEVENT:
						if ( ( ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ) || ( ev.window.event == SDL_WINDOWEVENT_SHOWN ) )
						{
							int w, h;
							SDL_GetWindowSize( win, &w, &h );
							app.Resize( w, h );
						}
						break;
					default:
						app.OtherEvent( ev );
				}
			}

			static Uint32 LastTick = SDL_GetTicks(); 
			Uint32 CurrentTick = SDL_GetTicks(); 
			SUpdateInfo updateInfo 
			{ 
				static_cast<float>(CurrentTick) / 1000.0f, 
				static_cast<float>(CurrentTick - LastTick) / 1000.0f 
			};
			LastTick = CurrentTick; 

			app.Update( updateInfo );
			app.Render();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(); 
			ImGui::NewFrame();
			app.RenderGUI();
			ImGui::Render();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			SDL_GL_SwapWindow(win);
		}

		app.Clean();
	} 
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow( win );

	return 0;
}
