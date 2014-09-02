rfglow
======

All the parts for an RF controlled RGB glow stick, in the style of a Ruifan KING BLADE (キングブレード). In fact, making this work requires a diffuser from a KB since the diffuser itself cannot be 3D printed.

There are 3 basic component areas:
 * Electronics (Eagle files and BOM)
 * Microcontroller code (Arduino code)
 * Plastic handle (Inventor IPT files and STL files for 3D printing)

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

# Microcontroller Code

Arudino Code for 1.0.x.

 * rfglow_slave
  * Code for the glow sticks themselves
 * rfglow_master
  * Code for a potential master
  * Currently has 3 buttons wired up to change the hue and put it into an automatic color cycling mode.
 * rfglow_serial
  * Code for when the master is connected to a computer, in which case you can enter the hue, saturation, and brightness as tuples over the serial line to send the command.
  * This code hasn't been touched in a while, and needs to have the radio settings from rfglow_master copied into it for it to actually work with the current version of rfglow_slave

# Plastic Handle

Autodesk Inventor parts and assembly with the current design, to be FFF 3D printed (e.g. <45 degree overhangs, etc...).

This design is in early stages and my CAD skills are weak. The CAD files can serve as a dimensional guide for stuff like the screw-on diffuser coupler and are printable, but do not yet produce a fully functional part.

