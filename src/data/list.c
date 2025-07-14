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


#include <SDL3/SDL.h>

#include <list.h>

//alternatively, if creating it on stack, you can just do List list = {0}
void List_Init(List *list)
{
	if(list == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_Init: invalid list.");
		return;
	}
	list->first = list->last = NULL;
	list->size = 0;
}

bool List_AddLast(List *list, void *value)
{
	if(list == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_AddLast: invalid list.");
		return false;
	}
	ListItem *aux = NULL;
	aux = (ListItem*)SDL_malloc(sizeof(ListItem));
	if(aux == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_AddLast: cannot add new item on list, memory error.");
		return false;
	}
	aux->value = value;
	aux->next = NULL;

	if(list->first == NULL)
	{
		aux->prev = NULL;
		list->first = aux;
	}
	else
	{
		aux->prev = list->last;
		list->last->next = aux;
	}
	list->last = aux;
	list->size++;
	return true;
}

bool List_Remove(List *list, void *value)
{
	if(list == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_Remove: invalid list.");
		return false;
	}
	if(value == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_Remove: invalid value.");
		return false;
	}

	ListItem *aux, *behind;
	aux = behind = NULL;
	if(list->first != NULL)
	{
		aux = list->first;
		behind = NULL;
		while(aux != NULL)
		{
			if(aux->value == value) break;
			behind = aux;
			aux = aux->next;
		}

		if(aux == NULL)
		{
			return false;
		}

		if(list->first == list->last)
		{
			list->first = list->last = NULL;
		}
		else
		{
			if(aux == list->first)
			{
				list->first = list->first->next;
			}
			else if (aux == list->last)
			{
				list->last = behind;
				list->last->next = NULL;
			}
			else
			{
				behind->next = aux->next;
			}
		}

		SDL_free(aux);
		aux = NULL;
		list->size--;
		return true;
	}
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "List_Remove: empty list.");
	return false;
}

void List_Destroy(List *list)
{
	if(list == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "List_Destroy: invalid list.");
		return;
	}

	//remove first
	ListItem *next = list->first->next;
	SDL_free(list->first);

	while(next != NULL)
	{
		ListItem *node = next;
		next = next->next;
		SDL_free(node);
	}

	if(list->first == NULL && list->last == NULL)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "List_Destroy: successfully emptied list.");
	}
	list->size = 0;

	//if you malloc'ed the List struct, you can now safely free it
	//not going to do that here because you might have created it on the stack

	return;
}