# inspector_alert_tools


# Summary of Project

I bought an inspectorUSB Alert, and found the available Windows only flaky ObserverUSB product weak.  I want to be able to plug in the device at my house and monitor and record radiation information 24x7, storing the data into mysql for easy web access with off the shelf HTML5 graphing of current and historical data. As I already have an active weather station and GM45 detector similarly configured, this should be strait forward.

# Project Deliveries

This project will deliver:
 
* libinst_tools.a - relatively simple cross-platform library to access the Alert Inspector USB. 

  Initial features of the library will include:
  *  the ability to write applications to poll and display the immediate cps (and other values)
  *  the ability to read and write the calibration information of the device
  (I am not attacking the 'logging' features of the device yet.)

* inspect_mon - CLI tool to allow basic device access

  Initial features of this tool will include:
  * the ability to show all hid devices
  * the ability to open a device, by path, by vendor_id & product_id, or by known product ids.
  * the ability to show to stdout the value of the binary form of the report descriptors
  * the ability to monitor the device, displaying the clicks recieved every second
  * the ability to monitor the device, and store the resulting clicks per unit time -> simple mysql table
  * the ability to store and load configuration from file

* [future - inspect_tool] 

  Features may include:
  * graphical display of some or all of the features exposed in inspect_mon
  * graphical clicks/time display of real time data

# Install Directions

1) First, get hidapi, a general cross-platform hid library
  * info: http://www.signal11.us/oss/hidapi/
  * github: https://github.com/signal11/hidapi
  * install directions: https://github.com/signal11/hidapi/blob/master/README.txt, specifically be sure to install the Fox-Toolkit 

2) If you get this code from github -
  * be sure to have all the GNU autoconf stuff installed (see the hidapi readme above to get this stuff.)
  * autoreconf --install

3) Compile the code (linux)
  * configure # you can pass --prefix, location to hidapi libs, etc here.
  * make

4) Install the product
  * make install # or sudo make install, if installing on the whole system

5) Enjoy :)

NOTE: currently I am developing on ubuntu linux amd64 and raspbian - both debian variants of linux, 
but will verify OSX shortly. ]    

# Open Source is Open

Feel free to fork / send pull requests for updating this. 

Or fork and enjoy in any way you want, pursuant to the standard licensing (see LICENCE) constraints.

If you have other devices, and can help me identify the vendor_id/product_ids, and/or changes in the protocols, please do so.  My contact info is below.




# Personal Reasons for this project

I will be using this project to:
* figure out the kinks of working with github. I use git/stash at work, but have not delivered open source code to github before.
* figure out the kinks of autoconf.  I have been meaning to do this for centuries :)
* learn about USB and HID protocols more in depth
* have fun. 

# Maintainer Info

Michael Wolenetz 
wolenetz@gmail.com

