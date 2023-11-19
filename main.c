// gcc.exe (Rev6, Built by MSYS2 project) 13.1.0
// .\res\gen.bat && gcc .\lib\hidapi\hid.c .\main.c .\res\resources.res -o katbat.exe -lsetupapi -Ihidapi -s -Os -mwindows

void katbat_tray_rebuild();

int ENV_LITE = 0;

#include "main.h"

void main(int argc, char* argv[]) {
  if (argc > 1 && strcmp(argv[1], "lite") == 0) ENV_LITE = 1;

  katbat_cfg_read();
  if (!ENV_LITE && GetFileAttributes(CONFIG_FILE) == INVALID_FILE_ATTRIBUTES) katbat_cfg_write();

  katbat_tray_rebuild();
  tray_init(&tray);

  CreateThread(NULL, 0, katbat_loop, NULL, 0, NULL);

  while (tray_loop(1) == 0);
}