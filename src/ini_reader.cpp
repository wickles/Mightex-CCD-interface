/**
 * Mightex CCD Interface
 * Copyright 2011 Alexander Wickes
 * This work is licensed under the MIT license,
 * subject to all terms as reproduced in the included LICENSE file.
 */

#include "ini_reader.h"
#include <cstring>
#include <cstdlib>

static char buffer[256];

int IniGetInt(FILE* file, const char* param, int notfound)
{
    if (!file || !param)
        return notfound;
    rewind(file);
    while(!feof(file))
    {
        fgets(buffer, sizeof(buffer), file);
        if (strncmp(buffer, param, strlen(param)) == 0)
		{
			int r;
			sscanf(buffer, "%*s = %i", &r);
			return r;
        }
    }
    return notfound;
}

float IniGetFloat(FILE* file, const char* param, float notfound)
{
    if (!file || !param)
        return notfound;
    rewind(file);
    while(!feof(file))
    {
        fgets(buffer, sizeof(buffer), file);
        if (strncmp(buffer, param, strlen(param)) == 0)
        {
			float r;
			sscanf(buffer, "%*s = %f", &r);
			return r;
        }
    }
    return notfound;
}

const char* IniGetString(FILE* file, const char* param, char* out, const char* notfound)
{
    if (!out)
        return NULL;
    if (!file || !param)
    {
        if (notfound)
            strcpy(out, notfound);
        return out;
    }
    rewind(file);
    while(!feof(file))
    {
        fgets(buffer, sizeof(buffer), file);
        if (strncmp(buffer, param, strlen(param)) == 0)
        {
            if (sscanf(buffer, "%*s = %s", out) != 1)
            {
                if (notfound)
                    strcpy(out, notfound);
                else
                    out[0] = '\0';
			}
			return out;
        }
	}
	if (notfound)
		strcpy(out, notfound);
	else
		out[0] = '\0';
    return out;
}

bool IniGetBool(FILE* file, const char* param, bool notfound)
{
	if (!file || !param)
		return notfound;
	char buffer[256];
	IniGetString(file, param, buffer, "");

	if (buffer[0] == 't' || buffer[0] == 'T' || buffer[0] == '1')
		return true;
	if (buffer[0] == 'f' || buffer[0] == 'F' || buffer[0] == '0')
		return true;

	return notfound;
}
