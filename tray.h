#include "lib/tinytray.h"

static struct tray tray = { .icon = "", .tooltip = "" };

char** ts;
int ts_size;
int ts_index;

int tray_index;

void tray_menu_header(char* text) {
  tray.menu = realloc(tray.menu, sizeof(struct tray_menu) * (tray_index + 2));
  tray.menu[tray_index++] = (struct tray_menu) { .text = text, .disabled = 1 };
  tray.menu[tray_index++] = (struct tray_menu) { .text = "-" };
}

void tray_menu_item(char* name, char* unit, int* steps, int stepSize, int* value, void* cb) {

  tray.menu = realloc(tray.menu, sizeof(struct tray_menu) * (tray_index + 1));

  tray.menu[tray_index] = (struct tray_menu) { .text = name };

  tray.menu[tray_index].submenu = malloc(sizeof(struct tray_menu) * (stepSize + 1));
  
  ts = realloc(ts, stepSize * 16 * 2);
  for (int i = 0; i < stepSize; i++) { 
    ts[ts_index] = malloc(16);
    sprintf(ts[ts_index], "%i %s", steps[i], unit);
    tray.menu[tray_index].submenu[i] = (struct tray_menu) { 
      .text = ts[ts_index], 
      .disabled = steps[i] == *value, 
      .checked = steps[i] == *value, 
      .context = value, 
      .data = &steps[i],
      .cb = cb
    };
    ts_index++;
  }
  tray.menu[tray_index].submenu[stepSize] = (struct tray_menu) { .text = NULL };

  tray_index++;
}

void tray_menu_footer(int lite) {
  tray.menu = realloc(tray.menu, sizeof(struct tray_menu) * (tray_index + 2 + !lite));
  if (!lite) tray.menu[tray_index++] = (struct tray_menu) { .text = "-" };
  tray.menu[tray_index++] = (struct tray_menu) { .text = "Quit", .cb = tray_exit };
  tray.menu[tray_index++] = (struct tray_menu) { .text = NULL };
  
  tray_index = 0;
  ts_index = 0;
}