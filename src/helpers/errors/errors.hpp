#ifndef CUSTOM_ERROR_HANDLER_H
#define CUSTOM_ERROR_HANDLER_H

#define ERROR_GENERAL 0
#define ERROR_RUNTIME 1

void ExitWithError(const char* errorText, int exitCode, int errorType = ERROR_GENERAL);

#endif

