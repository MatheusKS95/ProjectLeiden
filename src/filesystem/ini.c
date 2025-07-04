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

//temporary until i find a replacement (self developed or from SDL)
#include <string.h>
#include <SDL3/SDL.h>
#include <fileio.h>
#include <ini.h>

static INIvar *ININewVariable(INIstruct *ini, const char *section, const char *key)
{
	INIsection *sec = NULL;
	INIvar *var = NULL;

	//see if already exists
	for(int i = 0; i < ini->length; i++)
	{
		if(strcmp(ini->sections[i].name, section) == 0)
		{
			sec = &ini->sections[i];
			break;
		}
	}

	//no section found, make it
	if(sec == NULL)
	{
		SDL_strlcpy(ini->sections[ini->length].name, section, 256);
		ini->sections[ini->length].length = 0;
		ini->length++;
	}

	//oops, something bad happened
	if(sec == NULL)
	{
		return NULL;
	}

	//add the key
	var = &sec->variables[sec->length];
	SDL_strlcpy(var->key, key, 256);
	sec->length++;

	return var;
}

INIstruct *ININew()
{
	INIstruct *newini = (INIstruct*)SDL_malloc(sizeof(INIstruct));
	SDL_memset(newini, 0, sizeof(INIstruct));
	return newini;
}

bool INILoad(INIstruct *ini, const char *filepath)
{
	size_t filesize;
	char *file = FileIOReadText(filepath, &filesize);

	if(file == NULL)
	{
		return false;
	}

	//remove whitespace
	int j = 0;
	for(int i = 0; i < strlen(file); i++)
	{
		if(file[i] != ' ' && file[i] != '\t')
		{
			file[j++] = file[i];
		}
	}
	file[j] = 0;

	//token-ization
	int current_section = -1;
	char *token = strtok(file, "\t\n"); //TODO use SDL only
	while(token)
	{
		//is section?
		if(token[0] == '[' && token[strlen(token) - 1] == ']')
		{
			char section[256] = {0};
			SDL_strlcpy(section, &token[1], strlen(token) - 1);

			//try to set active section
			int i;
			for(i = 0; i < ini->length; i++)
			{
				if(SDL_strcmp(ini->sections[i].name, section) == 0)
				{
					current_section = i;
					break;
				}
			}

			//new section, if not found
			if(i == ini->length)
			{
				SDL_strlcpy(ini->sections[ini->length].name, section, 256);
				ini->sections[ini->length].length = 0;
				current_section = ini->length;
				ini->length++;
			}
		}

		//keys and values
		char *aux, *key, *value;
		if((aux = SDL_strstr(token, "=")))
		{
			key = token;
			value = &aux[1];
			aux[0] = '\0';

			if(current_section > -1)
			{
				INIsection *section = &ini->sections[current_section];

				//check if value exists
				INIvar *aux_var = NULL;
				for(int i = 0; i < section->length; i++)
				{
					//key found
					if(SDL_strcmp(section->variables[i].key, key) == 0)
					{
						aux_var = &section->variables[i];
						break;
					}
				}

				//new key, if not found
				if(aux_var == NULL)
				{
					SDL_strlcpy(section->variables[section->length].key, key, 256);
					aux_var = &section->variables[section->length];
					section->length++;
				}

				//insert key-value pair into selection
				if(aux_var != NULL)
				{
					//is a number?
					char *p;
					SDL_strtod(value, &p);
					if(*p == '\0') //yeah, it's a number
					{
						float flt = SDL_strtod(value, NULL);
						aux_var->fltv = flt;
						aux_var->type = INI_FLOAT;
					}
					else
					{
						SDL_strlcpy(aux_var->strv, value, 256);
						aux_var->type = INI_STRING;
					}
				}
			}
		}
		token = strtok(NULL, "\t\n");
	}

	SDL_free(file);
	return true;
}

void INISave(INIstruct *ini, const char *filepath)
{
	FileIOWrite(filepath, NULL, 0, false);

	//begin saving data
	char buffer[1024];
	for(int i = 0; i < ini->length; i++)
	{
		INIsection *section = &ini->sections[i];

		//write section
		SDL_snprintf(buffer, 1024, "[%s]\n", section->name);
		FileIOWrite(filepath, buffer, strlen(buffer), true);

		for(int j = 0; j < section->length; j++)
		{
			INIvar *var = &section->variables[j];

			//format buffer for writing
			switch(var->type)
			{
				case INI_STRING:
				{
					SDL_snprintf(buffer, 1024, "%s = %s\n", var->key, var->strv);
					break;
				}
				case INI_FLOAT:
				{
					SDL_snprintf(buffer, 1024, "%s = %.2f\n", var->key, var->fltv);
					break;
				}
				case INI_UNDEFINED:
				{
					break;
				}
			}
			FileIOWrite(filepath, buffer, strlen(buffer), true);
		}
		FileIOWrite(filepath, "\n", 1, true);
	}
}

INIvar *INIGetVar(INIstruct *ini,
						const char *section,
						const char *key)
{
	for(int i = 0; i < ini->length; i++)
	{
		INIsection *sec = &ini->sections[i];

		if(SDL_strncmp(sec->name, section, 256) != 0)
		{
			continue;
		}
		for(int j = 0; j < sec->length; j++)
		{
			INIvar *var = &sec->variables[j];
			if(SDL_strncmp(var->key, key, 256) == 0)
			{
				return var;
			}
		}
	}

	//if nothing is found, make a new one
	return ININewVariable(ini, section, key);
}

char *INIGetString(INIstruct *ini, const char *section, const char *key)
{
	INIvar *var = INIGetVar(ini, section, key);

	if(var != NULL && var->type == INI_STRING)
	{
		return var->strv;
	}
	return "";
}

float INIGetFloat(INIstruct *ini, const char *section, const char *key)
{
	INIvar *var = INIGetVar(ini, section, key);

	if(var != NULL && var->type == INI_FLOAT)
	{
		return var->fltv;
	}
	return 0.0f;
}

void INISetString(INIstruct *ini,
					const char *section,
					const char *key,
					const char *value)
{
	INIvar *var = INIGetVar(ini, section, key);
	if(var != NULL)
	{
		var->type = INI_STRING;
		SDL_strlcpy(var->strv, value, 256);
	}
}

void INISetFloat(INIstruct *ini,
					const char *section,
					const char *key,
					float value)
{
	INIvar *var = INIGetVar(ini, section, key);
	if(var != NULL)
	{
		var->type = INI_FLOAT;
		var->fltv = value;
	}
}

void INIDestroy(INIstruct **ini)
{
	SDL_free(*ini);
}
