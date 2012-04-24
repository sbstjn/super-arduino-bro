/**
 *
 *	Super Arduino Bro - Simple LED Jump'n'Run for Arduino V0.1
 *	Copyright (c) 2012 Sebastian Müller code@semu.mp, all rights reserved
 *	
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation; either
 *	version 2.1 of the License, or (at your option) any later version.
 *	
 *	This library is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *	Lesser General Public License for more details.
 *	
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with this library; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301, USA
 *
 */

/**
 * Load Colorduino
 */   
#include "Colorduino.h"
   
/**
 * Level Defintiion
 * Maximum Leeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeevel Length->|";
 */
char line0[] = "#  #####################################################   ################################################################################################################|";
char line1[] = "#                                                                             ###    #########     #########             ###                  ###                       ###|";
char line2[] = "#                                                  #   #   #   #      ##    #####          ###     #########      x      ###           x      ###                x      ###|";
char line3[] = "#              x                  ###  #  ##########   #   #   #  ##      ####             ###                   ###                  ###                       ###        |";
char line4[] = "#         #######         # # #     #              #####   #####      ##    #####          ###            x      ###      x           ###      x                ###      x |";
char line5[] = "#                       # # # # #      #                                      ###    #########       #######     ###     ###          ###     ###               ###     ###|";
char line6[] = "#       x             # # # # # #   x  # #     ###  # #   x               x                                                                                                |";
char line7[] = "#### ###################### # ####################### #################################################   #################################################################|";

/**
 * Includes
 */
#define CHAR_WALL     '#'
#define CHAR_END      '|'
#define CHAR_ENEMY    'x'

#define SIZE_X        8
#define SIZE_Y        8
   
#define PIN_LEFT      A3
#define PIN_RIGHT     A4
#define PIN_JUMP      A5

#define IS_FREE_FALLING true

int curWalker = 0;
int jumpCounter = 0;
boolean isRunning = false;
boolean hasAction = false;
boolean isMoving = false;
boolean hasWalked = false;

int posX = 0;
int posY = 0;
int ledX = 0;
int ledY = 0;
int accX = 0;
int accY = -1;

unsigned char plasma[8][8];

/**
 * Check if player has fallen to the ground. Always false if free falling is disabled!
 */
bool hasFallenDown() {
  return (IS_FREE_FALLING && posY < 1);
}

/**
 * Check if an enemy is sitting at player's current position
 */
bool positionHasEnemy() {
  return itemIsEnemy(itemAtPosition(posX, 7-posY));
}

/**
 * Set player's current position and move level walker if needed
 */
void setPlayer(int x, int y) {
  // Check if player has changed its position
  if (posX != x || posY != y) {
    hasAction = true; }
  
  // Player has reached middle of the screen, move level instead of player
  if (posX > 3) {
    x = 3;
    
    // Increase level walker for moving level background instead of player
    if (!hasWalked) {
      curWalker++;
      hasWalked++;
    }
  }
  
  // Set player's position and convert to internal LED position
  posX = x;
  posY = y;
  convert(x, y);
  
  // Set global variables for using drawPosition()
  ledX = x;
  ledY = y;
}

/**
 * Convert X/Y by reference
 */
void convert(int& x, int& y) {
  int tmp = x;
  
  x = y;
  y = abs(tmp - 7);
}

/**
 * Setup 
 */
void setup() {
  Serial.begin(9600);
  Colorduino.Init();
 
  // Set player to top left + 1 corner for start up
  setPlayer(1, 7);

  // Activate game and draw initial screen
  isRunning = true;
  hasAction = true;
  
  // Draw initial screen
  draw();
}

/**
 * Check inputs for acceleration, see defines for changing pins
 */
void checkAcceleration() {
  // Check left/right movement
  if (digitalRead(PIN_RIGHT) == 1) {
    accX = 1;
  } else if (digitalRead(PIN_LEFT) == 1) {
    accX = -1;
  } else {
    accX = 0;
  }
    
  // Check for (double–)jump and enable free falling if defined
  if (digitalRead(PIN_JUMP) == 1 && accY != -1 && jumpCounter < 2) {
    jumpCounter++;
    accY = 1;
  } else if (IS_FREE_FALLING && canFallDown()) {
    accY = -1;
  } else {
    accY = 0;
    jumpCounter = 0;
  }
}

/**
 * Does gravity exist ?
 */
bool canFallDown() {
  return movementPossible(posX, posY-1);
}

/**
 * Check if it's possible to move to coordinate
 *
 * @param nextX x coord
 * @param nextY y coord 
 */
bool movementPossible(int nextX, int nextY) {
  return !itemIsWall(itemAtPosition(nextX, 7-nextY));
}

/**
 * Check if given item is a wall or level end
 *
 * @param item level item to check (#, x, etc.)
 */
bool itemIsWall(char item) {
  return (item == CHAR_WALL || item == CHAR_END);
}

/**
 * Check if given item is enemy
 */
bool itemIsEnemy(char item) {
  return (item == CHAR_ENEMY);
}

/**
 * Move player on X axis based on already read input values
 */
void moveX() {
  if (accX == 0) {
    return; }  
  
  int curPositionX = posX;
  int nextPositionX = curPositionX + accX;
  
  if (movementPossible(nextPositionX, posY)) {
    setPlayer(nextPositionX, posY); }
  else {
    accX = 0; }
}

/**
 * Move player on Y axis based on already read input values
 */
void moveY() {
  if (accY == 0) {
    return; }
  
  int curPositionY = posY;
  int nextPositionY = curPositionY + accY;
  
  if (movementPossible(posX, nextPositionY)) {
    setPlayer(posX, nextPositionY); } 
  else {
    accY = 0; }
}

/**
 * Loop
 */
void loop() {
  if (isRunning) {
    // Check controller inputs and set up gravity
    checkAcceleration();
    
    // Check for movement
    if (accX != 0 || accY != 0) {
      moveX();
      moveY();
    }
  
    // Draw screen if needed
    if (hasAction) {
      draw();
      checkCollisions();
      delay(200);

      hasAction = false;
      hasWalked = false;
      accY = 0;
      accX = 0;
    }
  }
}

/**
 * Check if player has fallen to ground or ran into an enemy 
 */
void checkCollisions() {
  if (hasFallenDown() || positionHasEnemy()) {
    dieScreen(); }
}

/**
 * Draw screen with level and player position
 */
void draw() {
  clearScreen();
  drawLevel();
  drawPosition();

  Colorduino.FlipPage();  
}

/**
 * Draw level item on display coordinate
 *
 * @param item character
 * @param x coord
 * @param y coord
 */
void drawItem(char item, int x, int y) {
  if (itemIsWall(item)) {
    // Draw Walls
    Colorduino.SetPixel(x, y, 0, 0, 250);
  } else if (item == CHAR_ENEMY) {
    // Draw Enemies
    Colorduino.SetPixel(x, y, 255, 0, 0);
  } else {
    // Draw Whitespaces
    Colorduino.SetPixel(x, y, 0, 0, 0);
  }
  
  if (item == CHAR_END) {
    // Stop if end of level is reached
    isRunning = false; 
  }  
}

/**
 * Fill screen with whitespaces
 */
void clearScreen() {
  for (unsigned char y = 0; y < SIZE_Y; y++) {
    for (unsigned char x = 0; x <SIZE_X; x++) {
      Colorduino.SetPixel(x, y, 0, 0, 0);
    }
  }
}

/**
 * Show player died and stop game engine
 */
void dieScreen() {
  for (unsigned char y = 0; y < SIZE_Y; y++) {
    for (unsigned char x = 0; x <SIZE_X; x++) {
      Colorduino.SetPixel(x, y, 250, 0, 0);
    }
  }
  
  isRunning = false;
  Colorduino.FlipPage();  
}


/**
 * Draw given line
 *
 * @param row line
 */
void drawLine(int row) {
  for (int i = 0; i < 8; i++) {
    int x = 7 - row;
    int y = 7 - i;
    
    drawItem(itemAtPosition(i, row) , x, y);
  }
}

char itemAtPosition(int x, int y) {
  int j = x + curWalker;
    
  // TODO: Create a better solution, this cannot be the final one…
  switch (y) {
    case 0:
      return line0[j];
      break;
    case 1:
      return line1[j];
      break;
    case 2:
      return line2[j];
      break;
    case 3:
      return line3[j];
      break;
    case 4:
      return line4[j];
      break;
    case 5:
      return line5[j];
      break;
    case 6:
      return line6[j];
      break;
    case 7:
      return line7[j];
      break;
  }
}

/**
 * Draw level
 */
void drawLevel() {
  for (int i = 0; i < 8; i++) {
    drawLine(i); }
}

/**
 * Draw the players position
 */
void drawPosition() {
  Colorduino.SetPixel(ledX, ledY, 0, 255, 0);
}

