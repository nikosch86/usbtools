var usb = require('usb')
var _ = require('lodash')
var shell = require('shelljs')

var usbSubSystem = '/dev/bus/usb/'
var skipClasses = [ 9, 224 ]

var whoami = _.trim(shell.exec('whoami').stdout, '\n')
if (whoami != 'root') {
  console.log('This script needs to be run as root')
  shell.exit(1)
}

shell.config.silent = true

if (shell.ls('usbreset').code !== 0) {
  console.log('need to build usbreset: "cc usbreset.c -o usbreset"')
  shell.exit(2)
}

_.each(usb.getDeviceList(), function(device) {
  if (_.indexOf(skipClasses, device.deviceDescriptor.bDeviceClass) == -1) {
    var busNumber = _.padStart(device.busNumber, 3, 0)
    var deviceAddress = _.padStart(device.deviceAddress, 3, 0)
    console.log('Resetting device '+busNumber+':'+deviceAddress+', class '+device.deviceDescriptor.bDeviceClass)
    shell.exec('./usbreset '+usbSubSystem+busNumber+'/'+deviceAddress)
  }

})
