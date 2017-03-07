# Lua log viewer for FrSky Horus with OpenTX

![Screenshot](screenshot.png)

Download the latest version: https://raw.githubusercontent.com/RealTadango/FrSky/master/LView/LView.lua

Installation:
Copy LView.lua to any location on the SD card and execute it through the SD browser.

Features:
 - Allows selection of a log file from the current model
 - Has different speed modes to read the logs
 - Allows up to 4 variables to be shown in the graph at the same time
 - Support a cursor, zoom and scroll mode
 - Indexes log files for faster loading the second time

This lua script is a log viewer for the FrSky Horus transmitter running OpenTX. It can be placed on the SD card in any location and is started from the SD explorer. Navigation during the menus:
 - Scroll left / right: Change a value
 - PgDn / PgUp: Select another option
 - Enter: Confirm the selection
 - Rtn: Return to the previous step

Navigation during the graph:
 - Scroll left / right: Move the cursor, zoom in / out, scroll the graph
 - Enter: Change the mode: Cursor, Zoom, Scroll
 - Rtn: Return to the previous step

After starting a log file must be selected. The script only shows the log files from the last 100 days for the current model. If you want to view a log from another model you have to select that model first in OpenTX. Below the log file is a choice for speed:
 - Accurate: Reads every line from the logfile to check if a session has ended or started.
 - Fast: This mode reads every 5th line from the logfile. This is faster but you might loose 5 lines in the beginning or end of the session.
 - Use index: If you have read the logfile before, a session index is availabe and the log file is not analysed again. If you have added a new session to a current logfile you must choose one of the other options to index it again.
 
After the logfile has been analysed a session must be selected. Below the selected session the start time and duration is shown. Below the sessions up to 4 variables can be selected from the log file. Date and time are not available. The value must be a number to be able to be displayed in the graph. The folowing speed modes can be selected:
 - Accurate: Reads every value from the session
 - Faster: Reads every 2nd value from the session
 - Fast: Reads every 5th value from the session
 - Fastest: Reads every 10th value from the session
 
Note: Only accurate mode finds the min / max values at the correct locations. All other modes might skip over the real min / max value and show a different location in the graph if the peak was small.

The graph loads 100 points for each variable from the selected session. The min / max values are determined by the entire session (see note abote). When zooming / scrolling 100 poins are used to match the zoom / scroll location. When zooming out many points from the session are skipped in the graph. If there are 1000 points in the session every 10th point is used for the graph. Peaks can be skipped and may not be visible in the graph. To indicate the real min / max position in the session a vertical line is placed above and below the graph for every variable in the color of the variable. If 2 variables have the min / max on the same location only the line for the last variable is shown.

In the graph the min / max values are show on the y access on both sides of the graph. On the center line the cursor time and current mode is shown. The bottom line shows the variable labels and the value at the cursor location.
