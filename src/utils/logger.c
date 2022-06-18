#include "includes/logger.h"

LOG_LEVEL current_level = DEBUG;


void setLogLevel(LOG_LEVEL newLevel) {
	if ( newLevel >= DEBUG && newLevel <= FATAL )
	   current_level = newLevel;
}

char * levelDescription(LOG_LEVEL level) {
    static char * description[] = {"DEBUG", "INFO", "ERROR", "FATAL"};
    if (level < DEBUG || level > FATAL)
        return "";
    return description[level];
}


void plog(LOG_LEVEL level, char * fmt, ...) {
    if(level >= current_level) {
        fprintf (stderr, "%s: ", levelDescription(level)); 
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        fprintf(stderr,"\n"); 
    }   
	if ( level==FATAL) exit(1);
}