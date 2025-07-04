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
 * @file fileio.h
 * @brief File IO operations management
 * 
 * Here we can use the game's filesystem in a safer manner thanks to
 * PhysFS. The game uses a read-only zip file with game data and a
 * read-write directory separate from the game for user data.
 * 
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#ifndef FILEIO
#define FILEIO

#include <stdbool.h>
#include <SDL3/SDL_stdinc.h>

/**
 * Since we use PhysFS, we need this function to init it. Besides, this
 * function also mounts the preferred read and write directories. Read
 * and write directories are optional and can be left NULL or blank, in
 * this case, we'll use PhysFS defaults.
 * @brief Init file manager
 * @param argv (char**) arguments
 * @param readdir (const char*) read-only file locations
 * @param writedir (const char*) writable file locations
 * @param company (const char*) company name
 * @param appname (const char*) application name
 * @return bool
*/
bool FileIOInit(char **argv, const char *readdir, const char *writedir,
				const char *company, const char *appname);

/**
 * De-init PhysFS. Simple as that.
 * @brief De-init file manager (PhysFS)
*/
void FileIODeinit();

/**
 * Reads a text file from a mounted directory and stores it in a char
 * array. Also, the file size is stored in len. WARNING: this array is
 * malloc'd and must be freed after use.
 * @brief Read a text file
 * @param filename (const char*) directory + filename
 * @param len (size_t*) file lenght (initialized)
 * @return char *
*/
char *FileIOReadText(const char *filename, size_t *len);

/**
 * Reads a binary file from a mounted read-only or a read-write
 * directory and stores it in a 8-bit unsigned int array. Also, the file
 * size is stored in len. WARNING: this array is malloc'd and must be
 * freed after use.
 * @brief Read a binary file
 * @param filename (const char*) directory + filename
 * @param len (size_t*) file lenght (initialized)
 * @return uint8_t*
*/
uint8_t *FileIOReadBytes(const char *filename, size_t *len);

/**
 * Writes a file in a mounted read-write directory. Returns true if
 * everything is alright. Every time you call this function you can make
 * it replace the existing file or append to it.
 * @brief Write a file
 * @param filename (const char*) directory + filename
 * @param data (const void*) data stream to be written
 * @param len (size_t) size of data stream
 * @param append (bool) append data to end of file or replace file?
*/
bool FileIOWrite(const char *filename, const void *data, size_t len,
				bool append);


/**
 * Returns the location of the writable directory. This is useful if
 * you want to write files without PhysFS (or can't, like SQLite).
 * @brief Returns the location of the writable directory.
 * @return const char*
*/
const char* FileIOGetWritableDir();

char *FileIOGetDirName(char *path);

#endif
