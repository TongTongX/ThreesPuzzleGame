CMPUT 274

Project Title: Threes Arduino

Team Members:	Alan (Xutong) Zhao	1430631
		Yue Ma			1434071

Description: 
We developed the puzzle video game Threes using the Arduino Mega Board, with the interface displayed on the small LCD screen. 

On the LCD screen a 4*4 grid is displayed. Before that we need to initialize the grid. We assign a ceertain value to each tile: 1 and 2 are assigned randomly to two different tiles, and the rest tiles are set to be 0. Then the program displays the grid to the LCD screen mainly using the tft.print() function. In order to make the interface clear, we seperate each tile by printing " | ". Also, when the value of one tile is 0, we print out a space " " instead of the number 0. 

When the game starts, a user will be able to slide numbered tiles in four different directions (up, down, left or right) using a joystick so that addends and multiples of three will be combined.  For instance, “1” and “2” merge to become a “3”, two “3”s merge into a “6”, two “6”s merges into a “12”, etc. Once the user moves the joystick in one direction, all of the tiles on the grid will slide to the corresponding direction one block unless some tiles are stopped by another tile or the boundary of the grid. A new tile with value “1”, “2”, or “3” will be created in the same direction (yet on the opposite side of the grid). In order to enable the joystick to control the movement of tiles, at the very beginning of the program the pins are sut up so that we are able to keep track of the movement of the cursor. As soon as the joystick moves to one direction, the very first thing we do is to backup the grid by applying a function gridBackup() so that every tile of the current grid is copied to the other array. The reason why we do this before moving the tiles is because we need to copy the previous grid back to the current grid when we use the undo() function. Then we can move the tiles. Since moving a tile with a non-zero number to an empty block (whose value is 0) is basically adding this number to zero and setting the sum (which is the number itself) to be the value for the tile where 0 previously locates and setting 0 to the tile where the number previous locates, we can consider moving tiles is also adding numbers. In the updateTile() function, therefore, moving and adding are combined together. Moreover, a scoreboard is also included on the LCD, which calculates the score the user received after a movement is done as well as the total score. The score for each tile is calculated by 3^ (log2(x/3)+1).

Within the while loop, we always check whether the user can still play the game. The game is over if no further movement can be made, i.e., none of the numbered tile can be combined with the other one next to it. If there is still further movement that can be made, a new tile will be generated. The value of the new tile is randomly selected from {1, 2, 3, 6}. 

If the user presses the joystick, the undo() function will run. Here's essentially how it works: it copies from the previous grid created using gridBackup() to the current grid, meanwhile subtracts the score received from the last iteration from the total score. 
