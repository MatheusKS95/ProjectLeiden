#include <leiden.h>

//TODO a lot of stuff
//settings file could be at root (relative to asset path)
bool Leiden_Init(LeidenInitDesc *initdesc)
{
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: %s", SDL_GetError());
		return -1;
	}
	if(!SDL_InitSubSystem(SDL_INIT_EVENTS))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	const int linked = SDL_GetVersion();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Using SDL version %d.%d.%d", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

	if(!FileIOInit(initdesc->argv, initdesc->asset_path, NULL, initdesc->org_name, initdesc->app_name))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Content not found. Stopping.");
		SDL_Quit();
		return -1;
	}

	//TODO improve this, this is terrible (but works)
	int conf_found = 0;
	INIstruct *ini = ININew();
	if(!INILoad(ini, "settings/settings.ini"))
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Failed to read default default config files.");
		conf_found++;
	}
	if(!INILoad(ini, "usersettings.ini"))
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Failed to read user config files.");
		conf_found++;
	}
	if(conf_found >= 2)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to set configurations. Unable to continue.");
		FileIODeinit();
		SDL_Quit();
		return -1;
	}

	INISave(ini, "usersettings.ini");

	bool fullscreen = (INIGetFloat(ini, "graphics", "fullscreen") == 0.0f) ? false : true;
	int width = (int)INIGetFloat(ini, "graphics", "screen_width");
	int height = (int)INIGetFloat(ini, "graphics", "screen_heigth");
	INIDestroy(&ini);
	SDL_Window *window;
	if(fullscreen)
	{
		window = SDL_CreateWindow(initdesc->app_name, width, height, SDL_WINDOW_FULLSCREEN);
	}
	else
	{
		window = SDL_CreateWindow(initdesc->app_name, width, height, 0);
	}

	if (window == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "CreateWindow failed: %s", SDL_GetError());
		return -1;
	}
	Graphics_SetContext(window, width, height);
	if(!Graphics_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init graphics module");
		SDL_DestroyWindow(window);
		FileIODeinit();
		SDL_Quit();
		return 0;
	}

	if(!Audio_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init audio module");
	}
	return false;
}

void Leiden_Deinit()
{
	Audio_Deinit();
	Graphics_Deinit();
	SDL_DestroyWindow(context.window);
	FileIODeinit();
	SDL_Quit();
	return;
}