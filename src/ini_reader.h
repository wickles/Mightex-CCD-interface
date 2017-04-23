/**
 * Mightex CCD Interface
 * Copyright 2011 Alexander Wickes
 * This work is licensed under the MIT license,
 * subject to all terms as reproduced in the included LICENSE file.
 */

#include <cstdio>

int IniGetInt(FILE* file, const char* param, int notfound);

float IniGetFloat(FILE* file, const char* param, float notfound);

const char* IniGetString(FILE* file, const char* param, char* out, const char* notfound);

bool IniGetBool(FILE* file, const char* param, bool notfound);