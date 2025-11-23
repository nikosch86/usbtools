# USB Resetter

This is supplementary to the android testing station  
It goes through USB devices, filters by class and sends `USBDEVFS_RESET` via ioctl  
This will mimick unplugging the USB device and (hopefully) mitigate the flapping of devices we sometimes see  

## Features

- Enumerates all USB devices using libusb
- Filters devices by class (skips hubs and app-specific devices)
- Resets devices using `USBDEVFS_RESET` ioctl
- Mimics unplugging/replugging USB devices
- Minimal footprint (~30KB binary vs 40MB+ for Node.js)

## Installation

Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install libusb-1.0-0-dev

# Fedora/RHEL
sudo dnf install libusb1-devel

# Arch
sudo pacman -S libusb
```

Build the tool:
```bash
make
```

# Legacy 
The original `usbreset.c` utility is preserved for single-device reset operations:
```bash
gcc usbreset.c -o usbreset
sudo ./usbreset /dev/bus/usb/001/002
```
