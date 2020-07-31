// Debugging

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG_FILE "C:\\SCZ_CP.LOG"

void debug(const char *fmt, ...) {
    FILE* f;
    
    if (fopen_s(&f, DEBUG_FILE, "a+") == 0) {
        va_list args;
        
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);

        if (fmt && *fmt && *(fmt+strlen(fmt)-1) != '\n') {
        	fputc('\n', f);
        }

        fclose(f);
    }
}
