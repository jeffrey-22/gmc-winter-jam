#define SDL_MAIN_USE_CALLBACKS
#include "main.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

Engine engine;

SDL_AppResult SDL_AppInit(void**, int argc, char** argv) { return engine.init(argc, argv); }

SDL_AppResult SDL_AppIterate(void*) { return engine.iterate(); }

SDL_AppResult SDL_AppEvent(void*, SDL_Event* event) { return engine.handleEvent(*event); }

void SDL_AppQuit(void*, SDL_AppResult) { engine.shutdown(); }
