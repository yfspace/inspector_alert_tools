/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   insp_device.c
 * Author: minoc
 * 
 * Created on February 3, 2017, 9:57 PM
 */

#include <stdlib.h>
#include <stdio.h>

#include "insp_device.h"
#include "insp_log.h"

// ---------------------------------------------------------------------------

// find the hid device with the inspector 
// this is used when a path is not specified. 
hid_device *insp_device_open_by_product(
  unsigned short vendor_id, 
  unsigned short product_id, 
  const wchar_t *serial_number) 
{
  hid_device *handle = NULL;
  insp_log(INSP_LOG_DEBUG, "Attempting to open device %us %us %ls", 
           vendor_id, product_id, 
           INSP_LOG_NULL_SAFE_LS(serial_number));

  // Open the device using the VID, PID,
  // and optionally the Serial number.
  handle = hid_open(vendor_id, product_id, serial_number);

  if (handle != NULL) {
      insp_log(INSP_LOG_NOTICE, "Successfully opened device %us %us %ls",
               vendor_id, product_id, 
               INSP_LOG_NULL_SAFE_LS(serial_number));
  } else { 
      insp_log(INSP_LOG_DEBUG, "Unable to open device %us %us %ls",
               vendor_id, product_id, 
               INSP_LOG_NULL_SAFE_LS(serial_number));
  }
  return handle;
}

// ---------------------------------------------------------------------------

hid_device *insp_device_open_by_path(const char *path) 
{
  if (path == NULL || path[0] == 0)
    return NULL;
  
  return hid_open_path(path);
}

// ---------------------------------------------------------------------------

hid_device *insp_device_open_default_device() 
{
  hid_device *handle = NULL;
  int known_devices_i;
  for (known_devices_i = 0; 
       (handle == NULL) && (known_devices_i < sizeof(insp_device_known_products));
       ++known_devices_i) {
    // try known device ids

    handle = insp_device_open_by_product(
      insp_device_known_products[known_devices_i].vendor_id, 
      insp_device_known_products[known_devices_i].product_id, 
      NULL); // serial id not specified.
  }
  return handle;
}

// ---------------------------------------------------------------------------

void insp_device_list_all_hid_devices() 
{
  // Enumerate and print the HID devices on the system
  struct hid_device_info *devs, *cur_dev;

  devs = hid_enumerate(0x0, 0x0);
  cur_dev = devs;	
  while (cur_dev) {
    printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls",
            cur_dev->vendor_id, cur_dev->product_id, 
            INSP_LOG_NULL_SAFE(cur_dev->path), 
            INSP_LOG_NULL_SAFE_LS(cur_dev->serial_number));
    printf("\n");
    printf("  Manufacturer: %ls\n", 
           INSP_LOG_NULL_SAFE_LS(cur_dev->manufacturer_string));
    printf("  Product:      %ls\n", 
           INSP_LOG_NULL_SAFE_LS(cur_dev->product_string));
    printf("\n");
    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devs);
}

// ---------------------------------------------------------------------------

#ifdef ffff
if (handle == NULL) {
      log(LOG_FATAL, "Unable to find/open device (path=%s)",
          NULL_SAFE(path));
  }
#endif
// ---------------------------------------------------------------------------


