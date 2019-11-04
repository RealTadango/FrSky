# Lua stall warning / landing speed guide script

Download the latest version: https://raw.githubusercontent.com/RealTadango/FrSky/master/Stall/Stall.lua

Installation:
Copy Stall.lua to SCRIPTS\MIXER on the SD card and select the script in the Custom Scripts menu.

This scripts has 2 functions: It sounds a stall warning when the speed is below X and calls the speed when it is above Y. The stall warning can be used to maintain a minimum airspeed during landings while the high speed callout indicates that you are over the ideal landing speed. This leeves a window of the ideal landing speed during which the script is quiet. It is best to use a pitot sensor for this.

It has the folowing inputs:
 - Activate: The source for activating the mixer
 - Activ_Low: The activate source value has to be above this value for the script to be active
 - Activ_High: The activate source value has to be below this value for the script to be active
 - Input: Speed sensor for input
 - Stall_SPD: When the speed source value is below this value and the script is active the stall warning sounds is played
 - High_SPD: When the speed source value is above this value the speed is called without the unit
 
 