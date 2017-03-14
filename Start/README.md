# Lua auto start script

Download the latest version: https://raw.githubusercontent.com/RealTadango/FrSky/master/Start/Start.lua

Installation:
Copy Start.lua to SCRIPTS\MIXER on the SD card and select the script in the Custom Scripts menu.

This script can automate a start to help with planes that have to be hand launched. It requires an autopilot / autolevel receiver (like the S6R / S8R) to maintain a level flight during takeoff.

It has the folowing inputs:
 - Activate: The source for activating the mixer
 - Activ_Low: The activate source value has to be above this value for the script to be active
 - Activ_High: The activate source value has to be below this value for the script to be active
 - Delay: The time to wait before applying throttle
 - Duration: The duration of the auto throttle period
 
Outputs:
 - A_ON: Autostart script active (-100 to 100 range)
 - A_TH: Autostart throttle control (-100 to 100 range)
 
When the script is activated the delay period starts. The seconds left until auto throttle are called. During the delay period A_ON is set to 1024 and can be used to offset the elevator for a save climb. It also can be used to select / force the correct stabilize mode on the receiver / flight controller. When the delay period is over A_TH is increased to 1024 which can be used to apply throttle. When the duration period is over both A_ON and A_TH are set to -1024.
 
 