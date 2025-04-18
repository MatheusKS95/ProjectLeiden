#ifndef LEIDEN_H
#define LEIDEN_H

#include <SDL3/SDL.h>
#include <fileio.h>
#include <ini.h>
#include <graphics.h>
#include <audio.h>
#include <input.h>

/********************************************************************
 * INIT/DEINIT ******************************************************
 ********************************************************************/

typedef struct LeidenInitDesc
{
	int argc;
	char **argv;
	char asset_path[128];
	char org_name[64];
	char app_name[64];
} LeidenInitDesc;

bool Leiden_Init(LeidenInitDesc *initdesc);

void Leiden_Deinit();

#endif
