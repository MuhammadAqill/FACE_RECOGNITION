#ifndef APP_HTTPD_H
#define APP_HTTPD_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// initialize in startCameraServer() existing in app_httpd.cpp
// return last matched face id, or -1 if none
int app_get_last_matched_id();

// return pointer to null-terminated name for given id, or "Unknown"
const char* app_name_for_id(int id);

// return last matched name (thread-safe)
const char* app_get_last_matched_name();

#ifdef __cplusplus
}
#endif

#endif // APP_HTTPD_H
