#include <windows.h>
#include <stdio.h>

#define CONFIG_FILE "./config.ini"

// string separated by commas to int array
int *cvt(char *input, int *level) {
  char *cp = strtok(input, ",");
  if (cp == NULL) return (int *) malloc(sizeof(int) * *level);
  int index = -1;
  int n;
  if (sscanf(cp, "%d", &n) == 1) {
    index = *level;
    *level += 1;
  }
  int *array = cvt(NULL, level);
  if (index >= 0) array[index] = n;
  return array;
}

BOOL cfg_get_str(const char* section, const char* name, const char* defaultValue, char* value, int size) {
  return GetPrivateProfileStringA((LPCSTR) section, (LPCSTR) name, defaultValue, value, size, (LPCSTR) CONFIG_FILE) > 0;
}

BOOL cfg_set_str(const char* section, const char* name, char* value) {
  return WritePrivateProfileStringA((LPCSTR) section, (LPCSTR) name, value, (LPCSTR) CONFIG_FILE);
}

int cfg_get_int(const char* section, const char* name, int defaultValue) {
  return GetPrivateProfileIntA((LPCSTR) section, (LPCSTR) name, defaultValue, (LPCSTR) CONFIG_FILE);
}

void cfg_set_int(const char* section, const char* name, int value) {
  char val[255]; 
  itoa(value, val, 10);
  cfg_set_str(section, name, val);
}

void cfg_set_int_arr(const char* section, const char* name, int *arr, int arrSize) {
  char val[255] = ""; 
  for (int i = 0; i < arrSize; i++) sprintf(val, "%s%s%i", val, (i > 0 ? "," : ""), arr[i]);
  cfg_set_str(section, name, val);
}

int* cfg_get_int_arr(const char* section, const char* name, const char* defaultValue, int *arrSize) {
  char value[255]; 
  cfg_get_str(section, name, defaultValue, value, sizeof(value));
  return cvt(value, arrSize);
}

