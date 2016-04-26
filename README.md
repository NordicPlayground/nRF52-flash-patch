# nrf52-flash-patch
This repo contains a Flash Patch library and an example demonstrating it's use on Nordic Semiconductor's nRF52 Series Device.

#Why is the Flash Patch and Breakpoint unit (FPB) useful?
The FPB is a component in the ARM Cortex-M3/M4 architecture. It is normally used behind the scenes during debugging to set breakpoints. However, it can also be used to fix problems in final products. Using the FPB unit, a patch can be applied to the firmware of an in-the-field product without the cost of changing the entire application. This is usually done to fix bugs in in-the-field products when a device firmware upgrade (DFU) is not possible for some reason (i.e. 'replacing' buggy_function(...) with fixed_function(...)).

#Cautions
Normally the FPB is required by the debugger during software development. The FPB's registers are used to set breakpoints and it may overwrite your application's configuration if you aren't careful. In my example I use FPB.COMP[4:5] registers so I can set some breakpoints when debugging (each breakpoint uses one FPB.COMP register starting at FPB.COMP[0].

#Requirements
For a flash patch to be possible in an in-the-field product, a few steps must be taken during development.

* A small section in Code FLASH must be reserved for the FPB setup routine, fpb_setup(), as well as the patched code.
* A small (32 byte) section in RAM must be reserved to be used as the patch table.
* fpb_setup() must be unconditionally called by the application's startup code. As long as a flash patch is not required fpb_setup() will just return without doing anything (note there are options for this step).

To illustrate these requirements imagine this example: A product has been developed and it follows the above requirements. It is shipped and is operating in-the-field. A bug is discovered in buggy_function(). The developers fix the function and call it fixed_function(). They compile fixed_function() and the setup routine that will configure the FPB to redirect buggy_function() to fixed_function(), and they store this binary in the device's memory in the section of Code FLASH that was reserved for the FPB during development. The device is restarted and now the call to fpb_setup() configures the FPB for this patch. All calls to buggy_function() are redirected to fixed_function().

#The example
The example in this repo is simple, but similar to a real world scenario. We reserve the last page in Code FLASH (0x7F000-0x80000) for fpb_setup() and any patched code. We reserve 32 bytes in RAM (0x20007000-0x20007010) for our patch table. We define fpb_setup() and 'fixed_function()' which is turn_led_on() in our example in fpb_lib.c which is placed at 0x7F000. fpb_setup() configures the FPB unit to replace all calls to turn_led_off() with turn_led_on(). main() calls fpb_setup() and instead of all LEDs being OFF after the program executes, LEDs 1-3 are turned ON.

#The library
This library is not complete or fully tested. It is provided as is and will help you get started. Feel free to create issues or make pull requests.
