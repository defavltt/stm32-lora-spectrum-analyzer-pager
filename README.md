# STM32 based LoRa spectrum analyzer & pager

<!-- <img src="images/DSC_1560.JPG" width="800"> -->
![](images/DSC_1560.JPG)

## Features

 - Sending and receiving messages through LoRa
 
 - Configuring most of the RFM95's settings
 
 - Spectrum analyzer
 
 - Direct view of RFM95's registers
 
 - Battery measurement
 
 - Backlight
 
 - Vibration
 
 - Sleep modes
 
## Menu description

### *All 10 menus (with the last one being an optional clock)*
<img src="images/menus/main1.jpg" width="300"> <img src="images/menus/main2.jpg" width="300"> <img src="images/menus/main3.jpg" width="300">  
 
### *Send message*
<img src="images/menus/sendmsg.jpg" width="300">

 - Screen is split into two halves
   - Text field
   - Scrollable keyboard with SEND / CLEAR / BACKSPACE buttons at the bottom

### *Display settings*
<img src="images/menus/dispsett.jpg" width="300">

 - Backlight mode - timeout / no timeout (will not go to sleep) / on startup (turn backlight on when starting up)
 - Adjust contrast

### *RF settings*
<img src="images/menus/rfsett1.jpg" width="300"> <img src="images/menus/rfsett2.jpg" width="300">

 - Setting frequency
   - Select a digit to change by entering in the option and moving the digit cursor left / right
 - Continuous TX - doesnt transmit actual data, used for testing other devices

### *Feedback*
<img src="images/menus/fdbk.jpg" width="300">

 - Select ringing type when a message is received - silent / single beep / beep & vibration / only vibration
 - Select key press feedback - silent / only click / click & vibration / only vibration

### *Sleep settings*
<img src="images/menus/sleepsett.jpg" width="300">

 - Enable / disable sleep
 - Enable / disable receiving (when enabled, device wakes up every 5 seconds to listen for a incoming message)

### *Battery level*
<img src="images/menus/battlvl.jpg" width="300">

 - Shows battery level in volts

### *Spectrum analyzer*
<img src="images/menus/spctr.jpg" width="300">

 - Marker indicates RSSI for center frequency
 - 4 levels of vertical zooming
 - Navigation through parameters is done by pressing enter
 - Editing starting and ending frequency of the window
   - Up / down adjusts frequency by 1
   - Left / right adjusts frequency by 5
 - Showing parameters and/or marker is done by pressing multi key

### *Registers debug*
<img src="images/menus/regdbg.jpg" width="300">

 - Top left number showing address of the first register of the group
 - Top right is showing current selected register in binary format
 - Values at left are registers in HEX format
 - Values at right are the addresses corresponding to the registers
 - Navigating the 8 register group is done by pressing up / down
 - Navigating outside of the group is done by pressing left / right

### *Save settings*
<img src="images/menus/main3.jpg" width="300">

 - Saves settings into flash (don't overdo it to prevent flash memory wear)

### *Time and date*
<img src="images/menus/settime.jpg" width="300">

 - Setting time
   - Select hour / minute by entering in the option, moving the digit cursor left / right and changing it by pressing up / down
 - Setting date
   - Select day / month / year by entering in the option, moving the digit cursor left / right and changing it by pressing up / down
 - Confirm by pressing enter on set time / date

 - *Requires an i2c clock board(MCP7940)* <img src="images/mcp7940-module.jpg" height="80"> 
