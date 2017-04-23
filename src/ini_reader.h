#include <cstdio>

int IniGetInt(FILE* file, const char* param, int notfound);

float IniGetFloat(FILE* file, const char* param, float notfound);

const char* IniGetString(FILE* file, const char* param, char* out, const char* notfound);

bool IniGetBool(FILE* file, const char* param, bool notfound);