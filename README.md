# PKFP (Programmable Keyboard Foot Pedal)

This is the final project of Microprocessor System Lab at NCTU in Fall 2019.

## Develop Environment

* [ARM Mbed Studio](https://os.mbed.com/studio/)
* STM32L476RG

## Hardware

* STM32L476RG
* Bluetooth Module (HC-05)
* MAX7219
* Foot Switch

## Overall Structure

![](https://i.imgur.com/f1exyVf.png)

## Pin Definition

Bluetooth Module:

* TX: PC_1 (Serial)
* RX: PC_0 (Serial)

MAX7219:

* DIN: PA_5 (GPIO Output)
* CS: PA_6 (GPIO Output)
* CLK: PA_7 (GPIO Output)

Foot Switch:

* PC_5 (GPIO Input)

Button on STM32:

* PC_13 (GPIO Input)

## HID Profile

The default firmware of HC-05 doesn't support HID profile, so follow the instructions:

* [$3 Bluetooth HID Module (HC05 With RN42 Firmware)](https://www.instructables.com/id/3-Bluetooth-HID-Module-HC05-With-RN42-Firmware/)
* [Upgrade Your $3 Bluetooth Module to Have HID Firmware](https://www.instructables.com/id/Upgrade-Your-3-Bluetooth-Module-to-Have-HID-Firmwa/)

## Usage Flow

* Bluetooth module will initially configure as SPP profile
* SPP profile can change to HID profile by clicking the button on STM32

<img src="https://i.imgur.com/ZrhzBfb.png" width=400 />

### SPP Profile

* After the connection created between bluetooth module and computer, the shell interface can be accessed through the serial terminal

<img src="https://i.imgur.com/0jAn2ry.png" width=400 />

Supported commands:

```
Usage:

Set Commands:
bsize <int>: set max key buffer size
pushf <key-name>: push key to front of buffer
pushb <key-name>: push key to back of buffer
popf: pop key from front of buffer
popb: pop key from back of buffer
del <key-id>: delete key in buffer using key-id
insert <key-name> <key-id>: insert key to position <key-id>
replace <key-name> <key-id>: replace key from position <key-id>

Get Commands:
gb: get current key buffer
gbsize: get max key buffer size
gk: get avaliable key

Settings:
hid: reset profile to hid
```

### HID Profile

* Long press the foot pedal can switch to next key storing in the buffer
* Short press the foot pedal can send a key signal to the computer, which can be treated as keyboard input
