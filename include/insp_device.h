/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   insp_device.h
 * Author: minoc
 *
 * Created on February 3, 2017, 9:57 PM
 */

#ifndef INSP_DEVICE_H
#define INSP_DEVICE_H

#include <stdint.h>
#include <hidapi/hidapi.h>


#ifdef __cplusplus
extern "C" {
#endif
  
//! @file insp_protocol.h
//! NOTE: I might abstract the LL API with the exported APIs if other 
//! inspectors use different protocols and reports.  Might even replace
//! the LL layer with a generic schema translator
//! .. but for now., for my device, I will keep this simple 
  
//! simple structure containing list of known products.
typedef struct {
  uint16_t   vendor_id;
  uint16_t   product_id; 
} insp_device_known_products_t;

//! known inspector versions which work with this library
const insp_device_known_products_t insp_device_known_products[] = { 
  {0x1781, 0x08e9} /*!< Product:INSPECTORUSB, ASIN:B00EZBOSK8 
                        Radiation Alert SEI Inspector USB Handheld 
                        Digital Radiation Detector with LCD Display */
};
  
//! inspector modes, determined by switch on front of inspector
//! TODO: Determine if other modes (if, say, cps, uSv mode, ...)
typedef enum {
  INSP_DEVICE_MODE_CPM   = 0x0000,     //!< Clicks Per Minute Mode  
  INSP_DEVICE_MODE_MR_HR = 0x0002,     //!< mR/Hr Mode  
  INSP_DEVICE_MODE_TOTAL = 0x0014,     //!< Timer Total Mode  
} INSP_MODE_e;
  
//! Inspector Default Report 
//!   Fields types here derived from the descriptor specification, 
//!   however clicks_delta can go negative (~ffff)...
typedef struct insp_device_read_report_0_s {
  uint16_t     clicks_delta;  //!< clicks since last report, may be negative... 
  uint16_t     unknown_1;     //!< unknown, might be high bits for clicks_delta
  uint8_t      report_counter;//!< increments every unique report, wraps ff->00  
  uint16_t     current_value; //!< value, based on device_mode, shown on display 
  uint16_t     unknown_2;     //!< unknown, might be high bits for current_value
  INSP_MODE_e  device_mode;   //!< mode of the device (usually set by switch)
  uint16_t     unknown_3;     //!< unknown, so far 0s
  uint16_t     unknown_4;     //!< unknown, so far 0s  
} insp_read_report_0_t;
    
//! insp_report default typedef
typedef insp_read_report_0_t insp_read_report_t;     
    
//! write report (so far, I see the ObserverUSB software emitting all 0'ed 
//! reports every so often.  I believe that this might contain a field 
//! stating how often to create reports - which would be awesome - but 
//! I have no idea... As such, I will emit these periodically too, as 
//! at least it is consistent with the Observer software. 
typedef struct insp_write_report_0_s {
  uint8_t      unknown[8];    //!< unknown, descriptor says 8 type as 8 uint8_t
} insp_write_report_0_t; 

// TODO: add the configuration / calibration descriptor r/w class

// ---------------------------------------------------------------------------

//! Open a device, by vendor_id, product_id, and optional serial_number
//! @return NULL | newly opened hid device
//! Note, the serial number logic of linux w/ the hidraw does not work 
//!       with my device.  It is likely that the device serial number is not
//!       properly implemented, so for now, I am passing NULL here always.
//!       To access multiple identical devices, using the path open call 
//!         should work.
hid_device *insp_device_open_by_product(
  unsigned short vendor_id,      //!< vendor id
  unsigned short product_id,     //!< product id
  const wchar_t *serial_number); //!< optional serial number

//! Open a device, by path to hidraw device (on linux, usually /dev/hidraw#)
//! @return NULL | newly opened hid device
hid_device *insp_device_open_by_path(
  const char *path);             //!< path to device

//! Open a device based on known inspector devices
//! @return NULL | newly opened hid device
hid_device *insp_device_open_default_device();

//! List all HID devices (and associated vendor_id, product_id, path) to stdout
void insp_device_list_all_hid_devices();

#ifdef __cplusplus
}
#endif

#endif /* INSP_DEVICE_H */
