#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <usbhsfs.h>

namespace nx::hdd {
static const u32 MAX_DEVICES = 32;
static UEvent *g_statusChangeEvent = NULL, g_exitEvent = {0};

static u32 g_usbDeviceCount = 0;
UsbHsFsDevice g_usbDevices[MAX_DEVICES];

static thrd_t g_thread = {0};
static std::mutex g_mutex;

static int entry(void *arg) {
  (void)arg;

  Result rc = 0;
  int idx = 0;
  u32 listed_device_count = 0;

  Waiter status_change_event_waiter = waiterForUEvent(g_statusChangeEvent);
  Waiter exit_event_waiter = waiterForUEvent(&g_exitEvent);

  while (true) {
    rc = waitMulti(&idx, -1, status_change_event_waiter, exit_event_waiter);
    if (R_FAILED(rc))
      continue;

    std::scoped_lock lock(g_mutex);

    if (idx == 1) {
      break;
    }

    g_usbDeviceCount = usbHsFsGetMountedDeviceCount();

    if (!g_usbDeviceCount)
      continue;

    if (!(listed_device_count = usbHsFsListMountedDevices(g_usbDevices, std::min(g_usbDeviceCount, MAX_DEVICES)))) {
      continue;
    }
  }

  g_usbDeviceCount = 0;

  return 0;
}

u32 count() { return std::min(g_usbDeviceCount, MAX_DEVICES); }

const char *rootPath(u32 index) {
  if (index >= MAX_DEVICES) {
    return nullptr;
  }

  std::scoped_lock lock(g_mutex);

  if (index < usbHsFsGetMountedDeviceCount()) {
    return g_usbDevices[index].name;
  }

  return nullptr;
}

bool init() {
  if (g_statusChangeEvent) {
    return true;
  }

  if (usbHsFsInitialize(0)) {
    return false;
  }

  g_statusChangeEvent = usbHsFsGetStatusChangeUserEvent();

  ueventCreate(&g_exitEvent, true);

  thrd_create(&g_thread, entry, NULL);

  return true;
}

bool exit() {
  if (!g_statusChangeEvent) {
    return false;
  }

  ueventSignal(&g_exitEvent);

  thrd_join(g_thread, NULL);

  g_statusChangeEvent = NULL;

  usbHsFsExit();

  return true;
}
} // namespace nx::hdd
