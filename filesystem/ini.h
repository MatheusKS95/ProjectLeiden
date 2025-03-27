#ifndef FSINI_H
#define FSINI_H

#include <SDL3/SDL.h>

typedef enum
{
	INI_UNDEFINED,
	INI_STRING,
	INI_FLOAT
} INItype;

typedef struct
{
	char key[256];
	INItype type;
	union
	{
		char strv[256];
		float fltv;
	};
} INIvar;

typedef struct
{
	INIvar variables[256];
	char name[256];
	size_t length;
} INIsection;

typedef struct
{
	INIsection sections[256];
	size_t length;
} INIstruct;

INIstruct *ININew();

bool INILoad(INIstruct *ini, const char *filepath);

void INISave(INIstruct *ini, const char *filepath);

INIvar *INIGetVar(INIstruct *ini,
						const char *section,
						const char *key);

char *INIGetString(INIstruct *ini, const char *section, const char *key);

float INIGetFloat(INIstruct *ini, const char *section, const char *key);

void INISetString(INIstruct *ini,
					const char *section,
					const char *key,
					const char *value);

void INISetFloat(INIstruct *ini,
					const char *section,
					const char *key,
					float value);

void INIDestroy(INIstruct **ini);

#endif
