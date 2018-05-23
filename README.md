rfglow
======

All the parts for an RF controlled RGB glow stick, in the style of a Ruifan KING BLADE (キングブレード http://ruifan.co.jp/pro_kb_x.html). In fact, making this work requires a diffuser from a KB since the diffuser itself cannot be 3D printed.

There are 3 basic component areas:
 * Electronics (Eagle files and BOM)
 * Microcontroller code (Arduino code)
 * Plastic handle (Inventor IPT files and STL files for 3D printing)

## Project Chat

[![Join the chat at https://gitter.im/rfglow/Lobby](https://badges.gitter.im/rfglow/Lobby.svg)](https://gitter.im/rfglow/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Status
This project is fairly usable, although could use some additional polish. All of the areas are sufficiently functional to demonstrate a proof of concept, but requires more testing before a device could actually be used in a physically demanding environment.

The primary items that need work are:
 * App for the BLE master
 * Handle design - Refactor to be more compact and with more practical design, especially for the coupler and attachemnt of the two halves
 * RF Master - Integrate a 0.96" color OLED screen, then design a PCB for it
 * RF Slave - Code something for the button to do

Then there are other interesting things that one could do with a microcontroller:
 * Fade commands
 * Color cycling commands
 * Slaves also act as command repeaters

Contributions welcome in all areas.

# Electronics
The PCB design is in the eagle folder and contains the schematic, PCB layout, and a set of gerber files (zipped together) for sending to a PCB fabrication service like Osh Park or Seeedstudio. (I had my first set made at Osh Park.)

The PCB is really a kind of motherboard for a variety of sub-assemblies (other boards sold as modules), principally:
 * RFBee from Seeedstudio
  * A Panstamp from panstamp.com can also be used as it has the same hardware (and is more compact, even). However, I don't have  board design to accomodate one (yet!)
 * Pololu S7V8A Adjustable Regulator (http://www.pololu.com/product/2118)
  * The potentiometer on this should be adjusted to output 3.7V before putting in place on the board
 * STP04CM05 LED Driver
  * The one I used for this design is a 14-DIP package not officially mentioned on the datasheet. I got mine from Digi-key (http://www.digikey.com/product-detail/en/STP04CM05B1R/497-6940-5-ND/1852497).
 * Vollong 3W RGB High Power Led and a matching lens
  * I got mine from http://www.superbrightleds.com/moreinfo/component-leds/vollong-3w-rgb-high-power-led/899/2214/
  * You'll need to bridge together the - leads into a common cathode configuration (the STP04CM05 is a sink driver)
 * Grove connectors from Seeedstudio cut up
  * I'm sure there's some spec for these, but I had a bunch of these lying around and used them for this purpose. Since you'll have to order an RFBee from them, you might as well get a bunch of cables from them (male on both sides so each cable can make two connectors) and PCB receptacles.
  * The color convention I'm using is:
    * Cable Red --> LED Red
    * Cable White --> LED Green
    * Cable Yellow --> LED Blue
    * Cable Black --> LED Common Ground
 * Adafruit nRF8001 (https://www.adafruit.com/products/1697)
  * Used for the rfglow_ble code which will relay commands from the nRF8001's BLE interface to the glow sticks.
  * Would allow a mobile app (e.g. iOS, Android) on a BLE capable device to control the glow sticks with a more advanced interface
  * A panstamp must be used as the microcontroller to build this device because the nRF8001 must be wired to the hardware SPI interface, and the RFBee does not expose the pins necessary to do this.

The version of the BLE master in this repository is for an unamplified version that uses a Panstamp. There is also an amplified designed that uses a CC1101+CC1190 evaluation module. The schematics and board layout for that were done in Upverter and are available at https://upverter.com/jkao/e86d81299623de9d/RFGlow-BLE-CC1109-Relay/

# Microcontroller Code

Arudino Code for 1.0.x.

 * rfglow_slave
  * Code for the glow sticks themselves
 * rfglow_master
  * Code for a potential master
  * Currently supported gestures:
   * Color change - short press on left and right
   * LED on/off - long press on the center to toggle
   * Auto color cycling mode - short press on center to toggle
   * Brightness change - long press on left or right
 * rfglow_ble
  * BLE based master that will relay color change commands over BLE to the RF glow sticks
  * Usable today for simple color changes with the iOS nRF UART or LightBlue apps
   * For nRF UART, send ASCII of 3 numbers separated by spaces, like "270 255 32\n". This is an HSV value, with the first digit as hue (0-359), second digit as saturation (0-255), and third digit as brightness (0-255)
   * For LightBlue (and really for any app you design yourself), send 4 bytes to the write characteristic of the service. The first 2 bytes are a 16-bit unsigned int for hue (only valid from 0-355), the 3rd byte is the unsigned 8-bit int for saturation, the 4th byte is the unsigned 8-bit int for brightness.
 * rfglow_serial
  * Code for when the master is connected to a computer, in which case you can enter the hue, saturation, and brightness as tuples over the serial line to send the command.
  * This code hasn't been touched in a while, and needs to have the radio settings from rfglow_master copied into it for it to actually work with the current version of rfglow_slave

The Arduino code depends on the following libraries:
 * ShiftPWM from https://github.com/elcojacobs/ShiftPWM
 * Panstamp for Arudino 1.0.x, currently panstamp_library.zip from http://www.panstamp.com/downloads
  * Uses only the CC1101 libraries and not the SWAP stack
 * ClickButton from https://code.google.com/p/clickbutton/downloads/list
  * Only needed by rfglow_master currently for the button behavior

# Plastic Handle

Autodesk Inventor parts and assembly with the current design, to be FFF 3D printed (e.g. <45 degree overhangs, etc...).

This design is quite messy becuase my CAD skills are weak, and the parameters are difficult to adjust. However, it is functional with the following features:
* Lip-and-groove attachment between top and bottom halves to facilitate alignment
 * There is some clearance, so the groove is deeper than the lip sticks out
* Coupler to screw on a king blade diffuser on one end, then attach to the handle
* Cups for magnets to help hold the top and bottom together (0.25" X 0.125" X 0.0625" rare earth)
