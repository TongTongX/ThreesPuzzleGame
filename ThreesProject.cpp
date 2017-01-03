/** ========================= Initialization =========================*/
#include <Arduino.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>


#include <math.h>    // log(): computes natual log

/**
    // standard libraries, which will not be used on Arduino 
    #include <iostream>    // cin: standard input stream; cout: standard output stream
    #include <unistd.h>    // provides access to the POSIX operating system API

    // srand(time(NULL)) and rand() is replaed by random() function
    #include <cstdlib>    // rand(): generates random number; srand(): initializes random number generator; system(): invokes the command processor to execute a command
    #include <ctime>    // time(): gets the current time
    int ranNum = rand()%4;
   
    // printf() is replaced by tft.print()
    #include <cstdio>    // printf(): print formatted data to stdout; NULL

    // cmath is replaced by math.h
    #include <cmath>    // log2(): computes natural log

    using namespace std;
*/

/** standard U of A library settings, assuming Atmel Mega SPI pins */
#define SD_CS    5  // Chip select line for SD card
#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

// LCD Screen
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Joystick Pins
const int VERT = 0;        // analog input
const int HORIZ = 1;    // analog input
const int SEL = 9;        // digital input

/** ====================== Initialization ends =======================*/

/** ========== Constants and other variables =========*/
const int newTile[] = {1, 2, 3, 6};

int continueGame = 1;
int noMoreAdd = 1;

int plus = 0;
int score = 0;
int grid[4][4];
int prevGrid[4][4];

int cursorX = 1024;
int cursorY = 1024;
int oldCursorX, oldCursorY;


/** ======= Constants and other variables ends =======*/


/** ==================== Functions for various uses ==================*/
int full();
void gridBackup();
void undo();
int blockMoved();

void updateTile(int shiftVerti, int shiftHoriz);

void displayGrid();
void createNew();
void initializeGrid();
void displayGameOver();
int gameOverCheck();


void gamePlay() {
    // detect the movement of the cursor
    int initHoriz = analogRead(HORIZ);
    int initVerti = analogRead(VERT);
    
   
   
    while(true) {   
        oldCursorX = cursorX;
        oldCursorY = cursorY;
       
        int finalHoriz = analogRead(HORIZ);
        int finalVerti = analogRead(VERT);
       
        int select = digitalRead(SEL);    // HIGH(1) if not pressed, LOW(0) if pressed
                                             
        int deltaHoriz = (finalHoriz - initHoriz)/100;
        int deltaVerti = -(finalVerti - initVerti)/100;
       
        cursorX += deltaHoriz;
        cursorY += deltaVerti;
       
        int whetherFull = full();
       
        //int whetherMove = blockMoved();
       
        // cursur has moved
        if (oldCursorX!=cursorX || oldCursorY!=cursorY) {
            // backup the current grid before updating the tiles, for undo() function
            gridBackup();
            updateTile(deltaVerti, deltaHoriz);
            
            // check if any further movements can be made
            if(whetherFull && noMoreAdd) {continueGame = 0;}
               
            else {
                continueGame = 1;
                createNew();
			}
            
            displayGrid();
            delay(50);
        }
       
        // Undo: button of joystick has been pressed
        if (select == 0) {
            score-=plus;
            undo();
            displayGrid();
           
        }

        // if the game is over, display game over screen and break the while loop
        int gameEnd = gameOverCheck();
        if (gameEnd) {break;}
       
        // test: print out the values of coordinates to serial-mon
        //Serial.print(" whetherMove: ");
        //Serial.print(whetherMove);
        Serial.print(" continueGame: ");
        Serial.print(continueGame);
        Serial.print(" cursorX: ");
        Serial.print(cursorX);
        Serial.print(" cursorY: ");
        Serial.print(cursorY);
        Serial.print(" SEL= ");
        Serial.print(select,DEC);
        Serial.println();
        delay(10);
       
    }
}

// check whether the game is over
int gameOverCheck() {
    int gameOver = 0;
    if(continueGame == 0) {
        displayGameOver();
        gameOver = 1;
    }
    return gameOver;
}

// initialization of the grid, assign a value to each tile
// 1 and 2 are assigned randomly to two different tiles, the rest are set to be 0
void initializeGrid() {
    for(int i=0;i<4;i++) {   
        for(int j=0;j<4;j++) {
            grid[i][j]=0;
        }
    }
    int a = random(0, 4);    // random(min, max) generates a random number between min and max-1
    int b = random(0, 4);
    grid[a][b]=1;

    int c = random(0, 4);
    int d = random(0, 4);
    while (a==c && b==d) {
        c = random(0, 4);
        d = random(0, 4);
    }
    grid[c][d]=2;
}

// combine update_grid() and fill_space() together
void updateTile(int shiftVerti, int shiftHoriz) {
   

    plus = 0;
    noMoreAdd = 1;

    /** position of a tile: grid[i][j], where i is row, j is col */
    // case of shifting up
    if (shiftVerti > 0 && shiftHoriz == 0) {
        for(int j=0;j<4;j++) {
            for(int i=0;i<3;i++) {
                //~ swap(&(grid[i][j]), &(grid[i+1][j]));
                if((grid[i][j]==grid[i+1][j] && (grid[i][j]!=1 && grid[i][j]!=2)) || (grid[i][j]+grid[i+1][j]==3)) {
                    noMoreAdd=0;
                    grid[i][j]+=grid[i+1][j];    // addition (or double)
                    grid[i+1][j]=0;                // assign second element with "0"
                   
                    if (grid[i][j] >= 3) {
                        plus+=pow(3, log((grid[i][j])/3)+1);
                        score+=pow(3, log((grid[i][j])/3)+1);
                    }
                }
                else if(grid[i][j]==0) {    // if value is "0" (emtpy tile)
                    grid[i][j]=grid[i+1][j];   
                    grid[i+1][j]=0;
                }
            }
        }
    }
   
    // case of shifting down
    else if (shiftVerti < 0 && shiftHoriz == 0) {
        for(int j=0;j<4;j++) {
            for(int i=3;i>0;i--) {
                //~ swap(&(grid[i][j]), &(grid[i-1][j]));
                if((grid[i][j]==grid[i-1][j] && (grid[i][j]!=1 && grid[i][j]!=2)) || (grid[i][j]+grid[i-1][j]==3)) {
                    noMoreAdd=0;
                    grid[i][j]+=grid[i-1][j];
                    grid[i-1][j]=0;
                   
                    if (grid[i][j] >= 3) {
                        plus+=pow(3, log((grid[i][j])/3)+1);
                        score+=pow(3, log((grid[i][j])/3)+1);
                    }
                }
                else if(grid[i][j]==0) {    // if value is "0" (emtpy tile)
                    grid[i][j]=grid[i-1][j];   
                    grid[i-1][j]=0;
                }
            }
        }
    }
   
    // case of shifting left
    else if (shiftHoriz < 0 && shiftVerti == 0) {
        for(int i=0;i<4;i++) {
            for(int j=0;j<3;j++) {
                //~ swap(&(grid[i][j]), &(grid[i][j+1]));
                if((grid[i][j]==grid[i][j+1] && (grid[i][j]!=1 && grid[i][j]!=2)) || (grid[i][j]+grid[i][j+1]==3)) {
                    noMoreAdd=0;
                    grid[i][j]+=grid[i][j+1];
                    grid[i][j+1]=0;
                   
                    if (grid[i][j] >= 3) {
                        plus+=pow(3, log((grid[i][j])/3)+1);
                        score+=pow(3, log((grid[i][j])/3)+1);
                    }
                }
                else if(grid[i][j]==0) {    // if value is "0" (emtpy tile)
                    grid[i][j]=grid[i][j+1];   
                    grid[i][j+1]=0;
                }
            }
        }
    }
   
    // case of shifting right
    else if (shiftHoriz > 0 && shiftVerti == 0) {
        for(int i=0;i<4;i++) {
            for(int j=3;j>0;j--) {
                //~ swap(&(grid[i][j]), &(grid[i][j-1]));
                if((grid[i][j]==grid[i][j-1] && (grid[i][j]!=1 && grid[i][j]!=2)) || (grid[i][j]+grid[i][j-1]==3)) {
                    noMoreAdd=0;
                    grid[i][j]+=grid[i][j-1];
                    grid[i][j-1]=0;
                   
                    if (grid[i][j] >= 3) {
                        plus+=pow(3, log((grid[i][j])/3)+1);
                        score+=pow(3, log((grid[i][j])/3)+1);
                    }
                }
                else if(grid[i][j]==0) {    // if value is "0" (emtpy tile)
                    grid[i][j]=grid[i][j-1];   
                    grid[i][j-1]=0;
                }
            }
        }
    }
   
}

// create a new tile
void createNew() {
    int i,j,k;
   
    // use do/while to make sure the cell selected is empty
    do
    {
        i=random(0, 4);    // row position
        j=random(0, 4);    // col position
        k=random(0, 4);    // determine the value of the new tile

    }while(grid[i][j]);

    grid[i][j] = newTile[k];   
}


void gridBackup() {
    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
                prevGrid[i][j]=grid[i][j];
        }
    }
}

// Undo functionality: copy previous grid
void undo() {
    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
            grid[i][j]=prevGrid[i][j];
        }
    }
}


// check whether the grid is full
int full() {
    int isFull=1;

    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
            if(!grid[i][j])    {isFull=0;} // check whether all tiles are non-"0"
        }
    }
   
    return isFull;
}

// check whether current grid is the same as previous grid (i.e. whether a tile has moved)
int blockMoved() {
    int hasMoved=0;

    for(int i=0;i<4;i++) {
        for(int j=0;j<4;j++) {
            if(prevGrid[i][j]!=grid[i][j]) {
                hasMoved=1;
                break;
            }
        }
    }
    return hasMoved;
}   


void displayGrid() {
    tft.fillScreen(0x0000);    // clear the screen with black
    tft.setCursor(0,0);        // set the cursor, indicates where to display
    tft.setTextWrap(false);    // no wrap
   

    tft.print("\n THREES! PUZZLE GAME  \n\n");
    tft.print("      SCORE: ");
    tft.print(score);
    tft.print("\n");


    if (plus) {
        tft.print("            +");
        tft.print(plus);
        tft.print("\n\n");
    }
    else {
        tft.print(" ");
        tft.print("\n\n");
    }

    for(int i=0;i<4;i++) {
        tft.print("  |");

        for(int j=0;j<4;j++) {
            if(grid[i][j]) {
                // use sapce " " to make the interface clear since a number with more digits occupies more space
                if(grid[i][j] < 10) {
                    tft.print(" ");
                    tft.print(grid[i][j]);
                    tft.print(" |");
                }
                else if (grid[i][j] >= 10 && grid[i][j] < 100) {
                    tft.print("");
                    tft.print(grid[i][j]);
                    tft.print(" |");
                }
                else {
                    tft.print("");
                    tft.print(grid[i][j]);
                    tft.print("|");
                }
               
               
            }
            else {
                tft.print(" ");
                tft.print(" ");
                tft.print(" |");
            }
        }
        tft.print("\n\n");
    }
}


void displayGameOver() {
    tft.fillScreen(0x0000);    // clear the screen with black
    tft.setCursor(0,0);        // set the cursor, indicates where to display
    tft.setTextWrap(false);    // no wrap
    tft.setTextSize(3);
    tft.print("\n GAME     \n\n");
    tft.print(" OVER\n\n");
    tft.setTextSize(1);
    tft.print("     SCORE: ");
    tft.print(score);
    //~ tft.print("\n TOP 5 OF THE SCORE LIST ");
    // TO_DO: print out the score list, which needs to be saved in SD cardc
   
}

/** ================= Functions for various uses ends ================*/

/** ========================= Main function ==========================*/
int main (){
    init();
    Serial.begin(9600);
   
    randomSeed(analogRead(3)); // randomSeed() initializes the pseudo-random number generator
   
    pinMode(SEL, INPUT);
    digitalWrite(SEL, HIGH);
    tft.initR(INITR_BLACKTAB); // initialize a ST7735R chip
   
    initializeGrid();
    displayGrid();
    gamePlay();
  
    return 0;
}
/** ======================= Main function ends =======================*/
