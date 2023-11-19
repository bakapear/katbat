#include "lib/hidapi/hidapi.h"

#define KATANA_NAME "Katana Superlight"

#define KATANA_DIV_HZ 1000
#define KATANA_DIV_DPI 19000

struct katana {
    hid_device* device;

    int frame;

    int sleeping;
    int charging;
    int charge;

    int dpi;
    int pollingRate;
    int debounce;
    int liftOff;
    int idleTime;
} katana;

int katana_count() {
  katana.frame = katana.frame == 255 ? 0 : katana.frame + 1;
  return katana.frame;
}

void katana_set_dpi(int dpi) {
  if (dpi < 50) dpi = 50;
  else if (dpi > 19000) dpi = 19000;

  if (dpi > 10000) dpi = (int)(dpi / 100) * 100;
  else dpi = (int)(dpi / 50) * 50;

  int d = dpi - 10000;
  if (d < 0) d = 0;

  d = dpi / 50 - (d / 100) - 1;

  // if ever need to extend program to support multiple dpi slots, here is an explanation of these 3 bytes: https://pastebin.com/raw/0Q0Hnxzz
  char a[] = { 0x06, 0x11, 0x01 }; // first dpi slot, rest disabled
  char b[] = { d - 256, d >> 8 }; // split dpi value into 2 bytes / convert to uint16le

  hid_send_feature_report(katana.device, (unsigned char[66]) { 
    0x0c, 0x01, 0x05, 0x00, katana_count(), 0x01, 
    a[0], a[1], a[2], // dpi slot state
    b[0], b[1], 0, 0, 0, 0, 0, 0, // dpi value pairs for each slot
    0, 0, 0, 0, // 0 padding
    b[0], b[1], 0, 0, 0, 0, 0, 0 // dpi value pairs for each slot
  }, 66);
}

void katana_set_hz(int hz) {
  if (hz < 25) hz = 25;
  else if(hz > 1000) hz = 1000;
  hz = (int)(hz / 5) * 5;
  hid_send_feature_report(katana.device, (unsigned char[66]) { 0x0c, 0x01, 0x07, 0x00, katana_count(), 0x01, 0x01, KATANA_DIV_HZ / hz }, 66);
}

void katana_set_debounce(int ms) {
  if (ms < 0) ms = 0;
  else if (ms > 16) ms = 16;
  hid_send_feature_report(katana.device, (unsigned char[66]) { 0x0c, 0x01, 0x0a, 0x00, katana_count(), 0x01, 0x01, ms }, 66);
}

void katana_set_liftoff(int dist) {
  if (dist < 1) dist = 1;
  else if(dist > 2) dist = 2;
  hid_send_feature_report(katana.device, (unsigned char[66]) { 0x0c, 0x01, 0x09, 0x00, katana_count(), 0x01, 0x01, dist }, 66);
}

void katana_set_idletime(int m) {
  if (m < 0) m = 0;
  else if (m > 60) m = 60;
  hid_send_feature_report(katana.device, (unsigned char[66]) { 0x0c, 0x01, 0x0b, 0x00, katana_count(), 0x01, 0x02, m, 0x01 }, 66);
}

void katana_update() {
  unsigned char payload[66] = { 0x0c, 0x01, 0x20, 0x00, katana_count() };

  hid_send_feature_report(katana.device, payload, 66);
  if (hid_get_feature_report(katana.device, payload, 33) == -1) {
    katana.device = NULL;
  } else {
    katana.charging = payload[14];
    katana.charge = payload[15];
    katana.sleeping = !payload[16];
  }  
}

// initialize katana device by vendorId, productId and usagePage
void katana_init(int vendorId, int productId, int usagePage) {
  struct hid_device_info *cur_dev = hid_enumerate(vendorId, productId);
  while (cur_dev) {
    if(cur_dev->usage_page == usagePage) break;
    cur_dev = cur_dev->next;
  }

  if (cur_dev == NULL) return;

  katana.device = hid_open_path(cur_dev->path);
  katana.frame = 0;

  katana_update();
}
