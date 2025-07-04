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
 * @file hashtable.h
 * @brief Hashtable definitions and management
 *
 * Hashtable management, useful for asset management and similar.
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#ifndef HASHTABLE
#define HASHTABLE

#include <SDL3/SDL_stdinc.h>

#define HASH_SIZE 2048

typedef struct HashtableBucket
{
	char *key;
	void *value;
	struct HashtableBucket *next;
} HashtableBucket;

typedef struct
{
	HashtableBucket *buckets[HASH_SIZE];
} Hashtable;

Hashtable* HashtableInit();

bool HashtableInsert(Hashtable *table, const char *key, void *value);

void *HashtableFind(Hashtable *table, const char *key);

void HashtableDestroy(Hashtable *table);

unsigned int HashtableGetHashFromKey(const char *key);

#endif
