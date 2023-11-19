#include <windows.h>
#include "config.h"
#include "katana.h"
#include "tray.h"

typedef struct steps {
  int size;
  int* steps;
} steps;

struct cfg {
  unsigned int updateRate;
  unsigned int maxRetries;

  steps dpi;
  steps pollingRate;
  steps debounce;
  steps liftOff;
  steps idleTime;
} cfg;

char header[64] = "Katana Superlight";
char icon[64] = "bat_0.ico";

void katbat_cfg_read() {
  cfg.updateRate = cfg_get_int("Settings", "UpdateRate", 10000);
  cfg.maxRetries = cfg_get_int("Settings", "MaxRetries", 5);

  cfg.dpi.steps = cfg_get_int_arr("Config", "dpiSteps", "400,600,800,1600,1800,2000,2400,3600", &cfg.dpi.size);;
  cfg.pollingRate.steps = cfg_get_int_arr("Config", "pollingRateSteps", "125,250,500,1000", &cfg.pollingRate.size);
  cfg.debounce.steps = cfg_get_int_arr("Config", "debounceSteps", "0,2,4,6,8,10,12,14", &cfg.debounce.size);
  cfg.liftOff.steps = cfg_get_int_arr("Config", "liftOffSteps", "1,2", &cfg.liftOff.size);
  cfg.idleTime.steps = cfg_get_int_arr("Config", "idleTimeSteps", "1,2,3,4,5,6,7,8,9,10", &cfg.idleTime.size);

  katana.dpi = cfg_get_int("Data", "dpi", -1);
  katana.pollingRate = cfg_get_int("Data", "pollingRate", -1);
  katana.debounce = cfg_get_int("Data", "debounce", -1);
  katana.liftOff = cfg_get_int("Data", "liftOff", -1);
  katana.idleTime = cfg_get_int("Data", "idleTime", -1);
}

void katbat_cfg_write() {
  cfg_set_int("Settings", "UpdateRate", cfg.updateRate);
  cfg_set_int("Settings", "MaxRetries", cfg.maxRetries);

  cfg_set_int_arr("Config", "dpiSteps", cfg.dpi.steps, cfg.dpi.size);
  cfg_set_int_arr("Config", "pollingRateSteps", cfg.pollingRate.steps, cfg.pollingRate.size);
  cfg_set_int_arr("Config", "debounceSteps", cfg.debounce.steps, cfg.debounce.size);
  cfg_set_int_arr("Config", "liftOffSteps", cfg.liftOff.steps, cfg.liftOff.size);
  cfg_set_int_arr("Config", "idleTimeSteps", cfg.idleTime.steps, cfg.idleTime.size);

  cfg_set_int("Data", "dpi", katana.dpi);
  cfg_set_int("Data", "pollingRate", katana.pollingRate);
  cfg_set_int("Data", "debounce", katana.debounce);
  cfg_set_int("Data", "liftOff", katana.liftOff);
  cfg_set_int("Data", "idleTime", katana.idleTime);
}

void katbat_tray_update() {
  if (katana.device == NULL) {
    strcpy(header, "Not connected");

    tray.icon = "bat_d_0.ico";
  }
  else {
    sprintf(header, "%s - %i%% %s", KATANA_NAME, katana.charge, katana.charging ? "(Charging)" : katana.sleeping ? "(Sleeping)" : "");

    byte c = ((katana.charge / 10) * 10) + 10;
    if (c > 100 || katana.charging == 2) c = 100;

    if (katana.charge < 5) tray.icon = "bat_0.ico";
    else {
      sprintf(icon, "bat_%c_%i.ico", katana.charging ? 'c' : katana.sleeping ? 's' : 'n', c);
      tray.icon = icon;
    }
  }
  
  tray.menu[0].text = header;
  tray.tooltip = header;
  tray_update(&tray);
}

void katbat_write(int* ptr) {
  if (ptr == &katana.dpi) return (void) katana_set_dpi(katana.dpi);
  if (ptr == &katana.pollingRate) return (void) katana_set_hz(katana.pollingRate);
  if (ptr == &katana.debounce) return (void) katana_set_debounce(katana.debounce);
  if (ptr == &katana.liftOff) return (void) katana_set_liftoff(katana.liftOff);
  if (ptr == &katana.idleTime) return (void) katana_set_idletime(katana.idleTime);
}

void katbat_tray_cb(struct tray_menu* item) {
  int* value = item->context;
  *value = *(int*)item->data;

  katbat_write(value);
  katbat_cfg_write();

  katbat_tray_rebuild();
  tray_update(&tray);
}

void katbat_tray_rebuild() {
  tray.icon = icon;
  tray_menu_header(header);
  if (ENV_LITE == 0) {
    tray_menu_item("DPI", "DPI", cfg.dpi.steps, cfg.dpi.size, &katana.dpi, &katbat_tray_cb);
    tray_menu_item("Polling Rate", "Hz", cfg.pollingRate.steps, cfg.pollingRate.size, &katana.pollingRate, &katbat_tray_cb);
    tray_menu_item("Debounce Time", "ms", cfg.debounce.steps, cfg.debounce.size, &katana.debounce, &katbat_tray_cb);
    tray_menu_item("Lift-off Distance", "mm", cfg.liftOff.steps, cfg.liftOff.size, &katana.liftOff, &katbat_tray_cb);
    tray_menu_item("Idle Sleep Time", "m", cfg.idleTime.steps, cfg.idleTime.size, &katana.idleTime, &katbat_tray_cb);
  }
  tray_menu_footer(ENV_LITE);
}

DWORD WINAPI katbat_loop() {
  byte retries = 0;
  while (1) {
    if (katana.device == NULL) katana_init(0x248a, 0x5b2f, 0xff00);
    if (katana.device != NULL) katana_update();

    if (katana.device == NULL && ++retries < cfg.maxRetries) continue;
    retries = 0;

    katbat_tray_update();

    Sleep(cfg.updateRate);
  }
  return 0;
}