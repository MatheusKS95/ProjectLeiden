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
 * @file hashtable.c
 * @brief Hashtable (hashtable.h) implementation file
 *
 * This file TODO FIXME
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>

#include <hashtable.h>

static unsigned int hashtable_hash(const char *key)
{
	unsigned int hash = 0;
	while (*key)
	{
		hash = (hash * 31) + *key++;
	}
	return hash % HASH_SIZE;
}

HashtableBucket *create_node(const char *key, void *value)
{
	HashtableBucket *node = (HashtableBucket *)SDL_malloc(sizeof(HashtableBucket));
	if (node)
	{
		node->key = strdup(key);
		node->value = value;
		node->next = NULL;
	}
	return node;
}

Hashtable* HashtableInit()
{
	Hashtable *table = (Hashtable *)SDL_malloc(sizeof(Hashtable));
	if (table == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to alloc memory for the hash table.");
		return NULL;
	}
	else
	{
		for(int i = 0; i < HASH_SIZE; i++)
		{
			table->buckets[i] = NULL;
		}
		return table;
	}
}

bool HashtableInsert(Hashtable *table, const char *key, void *value)
{
	if(HashtableFind(table, key) != NULL)
	{
		//error
		return false;
	}
	unsigned int index = hashtable_hash(key);
	HashtableBucket *node = create_node(key, value);

	node->next = table->buckets[index];
	table->buckets[index] = node;
	return true;
}

void *HashtableFind(Hashtable *table, const char *key)
{
	unsigned int index = hashtable_hash(key);
	HashtableBucket *node = table->buckets[index];

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
		{
			return node->value;
		}
		node = node->next;
	}
	return NULL;
}

void HashtableDestroy(Hashtable *table)
{
	for (int i = 0; i < HASH_SIZE; i++)
	{
		HashtableBucket *node = table->buckets[i];
		while (node != NULL) {
			HashtableBucket *temp = node;
			node = node->next;
			SDL_free(temp->key);
			SDL_free(temp);
		}
	}
	SDL_free(table);
}

unsigned int HashtableGetHashFromKey(const char *key)
{
	return hashtable_hash(key);
}
