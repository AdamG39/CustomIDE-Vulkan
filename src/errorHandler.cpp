#include "errorHandler.h"

void ExitWithError(const char* errorText, int exitCode, int errorType) {
  const char* errorPrefix;
  switch(errorType) {
  case ERROR_GENERAL:
    errorPrefix = "[Error]: ";
    break;
  case ERROR_RUNTIME:
    errorPrefix = "[Runtime Error]: ";
    break;
  default:
    errorPrefix = "[Error]: ";
    break;
  }
  std::cerr << errorPrefix << errorText << std::endl;
  exit(exitCode);
}
