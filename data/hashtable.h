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

#endif
