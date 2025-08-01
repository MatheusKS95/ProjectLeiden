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

/**
 * @file fileio.c
 * @brief fileio.h implementation file
 * 
 * This file uses PhysicsFS.
 * Copyright (c) 2001-2022 Ryan C. Gordon <icculus@icculus.org> and others.
 * PhysicsFS is licensed under Zlib license.
 * This file uses MiniPhysFS.
 * Copyright (c) 2020 Eduardo Bart <edub4rt@gmail.com>
 * MiniPhysFS is licensed under Zlib license.
 * 
 * @author
 * - Matheus Klein Schaefer (email here)
*/

//NOTE, if you want to create a folder, you need to use PHYSFS_DECL int PHYSFS_mkdir(const char *dirName);
//"settings/somefile.ini" doesn't work if you don't create the folder before

#include <SDL3/SDL.h>

#define PHYSFS_IMPL
#define PHYSFS_PLATFORM_IMPL
#include "miniphysfs.h"

#include "fileio.h"

bool FileIOInit(char **argv, const char *readdir, const char *writedir,
				const char *company, const char *appname)
{
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing filesystem");
	PHYSFS_init(argv[0]);

	//read-only dir
	if(readdir == NULL || readdir[0] == '\0')
	{
		PHYSFS_deinit();
		return false;
	}

	if(PHYSFS_mount(readdir, NULL, 1) == 0)
	{
		PHYSFS_deinit();
		return false;
	}

	//writable dir
	if(writedir == NULL || writedir[0] == '\0')
	{
		const char *default_rw_path = PHYSFS_getPrefDir(company, appname);

		if(default_rw_path == NULL)
		{
			PHYSFS_deinit();
			return false;
		}

		PHYSFS_setWriteDir(default_rw_path);
		if(PHYSFS_mount(default_rw_path, NULL, 1) == 0)
		{
			PHYSFS_deinit();
			return false;
		}
	}
	else
	{
		PHYSFS_setWriteDir(writedir);
		if(PHYSFS_mount(writedir, NULL, 1))
		{
			PHYSFS_deinit();
			return false;
		}
	}

	return true;
}

void FileIODeinit()
{
	PHYSFS_deinit();
}

char *FileIOReadText(const char *filename, size_t *len)
{
	if(!PHYSFS_exists(filename))
	{
		//TODO log
		return NULL;
	}
	
	PHYSFS_file *file;
	file = PHYSFS_openRead(filename);

	size_t size = PHYSFS_fileLength(file);
	if(len != NULL)
	{
		*len = size;
	}

	char *buffer = (char*)SDL_malloc(size + 1);

	PHYSFS_readBytes(file, buffer, size);

	buffer[size] = '\0';

	PHYSFS_close(file);

	return buffer;
}

uint8_t *FileIOReadBytes(const char *filename, size_t *len)
{
	if(!PHYSFS_exists(filename))
	{
		//TODO log
		return NULL;
	}

	PHYSFS_file *file;
	file = PHYSFS_openRead(filename);

	size_t size = PHYSFS_fileLength(file);
	if(len != NULL)
	{
		*len = size;
	}

	uint8_t *buffer = (uint8_t*)SDL_malloc(size + 1);

	PHYSFS_readBytes(file, buffer, size);

	buffer[size] = '\0';

	PHYSFS_close(file);

	return buffer;
}

bool FileIOWrite(const char *filename, const void *data, size_t len,
				bool append)
{
	PHYSFS_file *file;
	file = NULL;
	
	if(append)
	{
		file = PHYSFS_openAppend(filename);
	}
	else
	{
		file = PHYSFS_openWrite(filename);
	}
	
	if(file == NULL)
	{
		//printf("\nPHYSFS ERROR %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}
	
	if(PHYSFS_writeBytes(file, (void*)data, len) < len)
	{
		//printf("\nPHYSFS ERROR %s\n", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
		return false;
	}
	
	PHYSFS_close(file);
	return true;
}

const char* FileIOGetWritableDir()
{
	if(PHYSFS_isInit())
	{
		return PHYSFS_getWriteDir();
	}
	else
	{
		return NULL;
	}
}

char *FileIOGetDirName(char *path)
{
	char *slash = SDL_strrchr(path, '/');
	if (!slash)
		return NULL;

	/* Length includes '\0' */
	size_t length = slash - path;
	char *dir = (char*)SDL_malloc(length + 1);

	SDL_memcpy(dir, path, length);
	dir[length] = '\0';

	return dir;
}
