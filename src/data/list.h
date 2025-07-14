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

#ifndef LIST_H
#define LIST_H

#include<SDL3/SDL_stdinc.h>

typedef struct ListItem
{
	struct ListItem *prev;
	void *value;
	struct ListItem *next;
} ListItem;

typedef struct List
{
	ListItem *first;
	ListItem *last;
	int size;
} List;

void List_Init(List *list);

bool List_AddLast(List *list, void *value);

bool List_Remove(List *list, void *value);

void List_Destroy(List *list);

#endif
