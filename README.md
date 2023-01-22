# Telecommunication_Group2
Github repository for group 2's telecommunications project "Dont Explode"

For this code to work several things are needed:
- All the hardware
- A node red flow setup with included json file
- A wifi hotspost (easily changed in ESP code)

To start "the bomb" as intended for normal use just press start on the nodered dashboard.

If playing without wifi connection, instead connect the arduino mega 2560 to a computer with the arduino IDE installed, and use the Serial monitor to start game by writing "start". 

Game can be ended when needed to in the same way by writing "end" in serial monitor.

If module doesnt work properly they can be debugged using the "debug=X" command in the serial monitor, where X is the number corresponding to the module:
- 0 = Wires
- 1 = Supersonic
- 2 = Maintenance
- 3 = Toggles
- 4 = Sequence
- 5 = Cycle
