/*
 * Copyright (C) 2025 Matheus Klein Schaefer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>. 
 */

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
