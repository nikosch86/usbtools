/**
 * USB Reset Tool - C Implementation
 * Enumerates and resets USB devices (except hubs and app-specific)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <libusb-1.0/libusb.h>
#include <linux/usbdevice_fs.h>

// Device classes to skip
#define USB_CLASS_HUB 9
#define USB_CLASS_APP_SPEC 254

/**
 * Reset a USB device using ioctl
 */
int reset_usb_device(const char *device_path) {
    int fd;
    int rc;

    fd = open(device_path, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening %s: %s\n", device_path, strerror(errno));
        return -1;
    }

    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
        fprintf(stderr, "Error resetting %s: %s\n", device_path, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/**
 * Get device class from device descriptor
 */
int get_device_class(libusb_device *dev) {
    struct libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(dev, &desc);

    if (r < 0) {
        fprintf(stderr, "Failed to get device descriptor: %s\n", libusb_error_name(r));
        return -1;
    }

    // If bDeviceClass is 0, check the first interface
    if (desc.bDeviceClass == 0) {
        libusb_device_handle *handle;
        struct libusb_config_descriptor *config;
        int device_class = 0;

        r = libusb_open(dev, &handle);
        if (r < 0) {
            // Can't open device, skip it
            return -1;
        }

        r = libusb_get_active_config_descriptor(dev, &config);
        if (r == 0 && config->interface != NULL && config->interface->altsetting != NULL) {
            device_class = config->interface->altsetting->bInterfaceClass;
            libusb_free_config_descriptor(config);
        }

        libusb_close(handle);
        return device_class;
    }

    return desc.bDeviceClass;
}

/**
 * Construct device path from bus and device numbers
 */
void construct_device_path(char *path, size_t path_size, uint8_t bus, uint8_t device) {
    snprintf(path, path_size, "/dev/bus/usb/%03d/%03d", bus, device);
}

/**
 * Main function - enumerate and reset USB devices
 */
int main(void) {
    libusb_device **devs;
    libusb_context *ctx = NULL;
    int r;
    ssize_t cnt;
    int i;
    int reset_count = 0;
    int skip_count = 0;

    // Initialize libusb
    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(r));
        return 1;
    }

    // Set verbosity level (optional)
    // libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_INFO);

    // Get the list of USB devices
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get device list: %s\n", libusb_error_name((int)cnt));
        libusb_exit(ctx);
        return 1;
    }

    printf("Found %ld USB devices\n", cnt);
    printf("----------------------------------------\n");

    // Iterate through all USB devices
    for (i = 0; devs[i]; i++) {
        libusb_device *dev = devs[i];
        struct libusb_device_descriptor desc;
        char device_path[256];
        uint8_t bus_num;
        uint8_t dev_addr;
        int device_class;

        // Get device descriptor
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "Failed to get device descriptor: %s\n", libusb_error_name(r));
            continue;
        }

        // Get bus and device numbers
        bus_num = libusb_get_bus_number(dev);
        dev_addr = libusb_get_device_address(dev);

        // Get device class
        device_class = get_device_class(dev);

        // Construct device path
        construct_device_path(device_path, sizeof(device_path), bus_num, dev_addr);

        printf("Device %d:\n", i);
        printf("  Bus: %03d, Device: %03d\n", bus_num, dev_addr);
        printf("  VID: 0x%04x, PID: 0x%04x\n", desc.idVendor, desc.idProduct);
        printf("  Class: %d\n", device_class);
        printf("  Path: %s\n", device_path);

        // Skip hubs and app-specific devices
        if (device_class == USB_CLASS_HUB) {
            printf("  Status: SKIPPED (Hub)\n");
            skip_count++;
        } else if (device_class == USB_CLASS_APP_SPEC) {
            printf("  Status: SKIPPED (Application Specific)\n");
            skip_count++;
        } else if (device_class < 0) {
            printf("  Status: SKIPPED (Cannot determine class)\n");
            skip_count++;
        } else {
            // Reset the device
            if (reset_usb_device(device_path) == 0) {
                printf("  Status: RESET SUCCESS\n");
                reset_count++;
            } else {
                printf("  Status: RESET FAILED\n");
            }
        }

        printf("----------------------------------------\n");
    }

    // Summary
    printf("\nSummary:\n");
    printf("  Total devices: %ld\n", cnt);
    printf("  Devices reset: %d\n", reset_count);
    printf("  Devices skipped: %d\n", skip_count);

    // Free the device list
    libusb_free_device_list(devs, 1);

    // Exit libusb
    libusb_exit(ctx);

    return 0;
}