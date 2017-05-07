# USB Resetter

This is supplementary to the android testing station  
It goes through USB devices, filters by class and sends `USBDEVFS_RESET` via ioctl  
This will mimick unplugging the USB device and (hopefully) mitigate the flapping of devices we sometimes see  


Remember to build `usbreset` by issuing:  
`cc usbreset.c -o usbreset`

This script obviously needs to run as root
