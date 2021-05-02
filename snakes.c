// gcc -o snakeGame -lncurses snakes.c

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

/************************************* Game Settings*/

int FOOD = 9;         // Maximum value of the food
int maxSpeed = 50000; // Speed is sleep in Microseconds
int minSpeed = 250000;
int expectedLen = 10; // Starting Length
int startingDir = 4; // Right: 0, Left: 1, Up: 2, Down: 3, Random: 4
int extraTime = 5;   // Add additional time to the Trophy countdown (in seconds)
/************************************* Functional Globals*/

int highScore = 0;     // Persists: Longest Snake Length
int trophyCapture = 0; // Persists: Last number of Trophy captured
int trophyFade = 0;    // Persists: Prev Game Faded
int gameNum = 1;       // How many games have been played.

int snakeLen = 0;                // Actual length of the snake.
int foodExp, foodExpCeiling = 0; // lifespan of tropy

// DEFINED THINGS

#define LOSE "Game Over!!!"
#define WIN "You Won!!!"

/***************************************** DataTypes*/

enum dir // Defines the direction
{ right, // = 0
  left,  // = 1
  up,    // = 2
  down   // = 3
};

enum state // State of the body objects.
{ eraser,  // 0, to be ignored
  one,     // 1... grow by x = 1
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine, // ...9 grow by x = 9
  head, // no food can spawn here.
  body, // collision = death, no food can spawn here
  tail  // collision = death, no food can spawn here. End of Body.
};

struct body {
  char body; // How the body will appear
  int x;     // x coordinate of the body
  int y;     // y coordinate of the body
  int state; // unique collision
};

/*************************************** Prototypes*/

int growSeg();
struct body buildBody(int x, int y);
void printHead(WINDOW *menu_win, int x, int y, int dir);
void printSnake(WINDOW *menu_win, struct body stack[], int actLen, int maxLen);
void printStats(WINDOW *menu_win, int rows, int cols, int x, int y, int speed,
                int dir, int expLen, int actLen, int capt, int fade);
struct body randomFood(WINDOW *menu_win, struct body *stack, int, int, int);
int snakeSpeed(int len, int maxSpeed, int minSpeed);
void killsnake();

int collisionCheck();

int main() {
  /************************************* Settings for screen*/

  WINDOW *menu_win;
  initscr();     // Initialize ncurses
  clear();       // Clear anything on screen (cut?)
  noecho();      // Turn off key echo
  curs_set(0);   // Hide the cursor
  cbreak();      // Disable line Buffering

 /************************************* Define colors for use*/
if (has_colors())
    {
        start_color();

        init_pair(1, COLOR_CYAN,    COLOR_GREEN);   // New Snake
        init_pair(2, COLOR_RED,   COLOR_GREEN);     // Beginner Snake
        init_pair(3, COLOR_YELLOW,  COLOR_GREEN);   // Intermediate Snake
        init_pair(4, COLOR_BLUE,    COLOR_GREEN);   // Professional Snake
        init_pair(5, COLOR_GREEN,    COLOR_WHITE);    // Food New
        init_pair(6, COLOR_MAGENTA, COLOR_WHITE);       // Food Mid
        init_pair(7, COLOR_RED, COLOR_WHITE);       // Food Fading
        init_pair(8, COLOR_RED,    COLOR_BLACK);    // Flash food 1
        init_pair(9, COLOR_WHITE, COLOR_RED);       // Flash food 2
    }



  /************************************ Primary (Local) Variables*/

  struct winsize wbuf;

  int cols, rows; // Holds the rows and columns for the screen
  int c;          // Receives input
  int trophyCount, trophyLost = 0;

  /* Capture window dimensions */
  if (ioctl(0, TIOCGWINSZ, &wbuf) != -1) {
    cols = wbuf.ws_col;
    rows = wbuf.ws_row;
  } else {
    perror("Could not get screen dimensions");
    endwin();
    exit(1);
  }

  time_t t;                  // Get the time
  srand((unsigned)time(&t)); // Seed the random number generator

  int maxSnake = rows + cols; // maximum length before win condition.
  struct body BodyStack[maxSnake + FOOD]; // Space for the snake to grow into
                                          // the win condition

  /**************************** Window settings */

  menu_win = newwin(rows - 1, cols - 1, 1,
                    1);    // For offsets, -2, and another -1 to fit within the
                           // framework (-3 on the rows and cols)
  keypad(menu_win, TRUE);  // Enable Keypad for arrow use.
  nodelay(menu_win, TRUE); // Don't wait on c input.
  box(menu_win, 0, 0);     // Draw a box around the screen.

  /********************************** Initial state */

  int myX = ((cols - 2) / 2); // Initial X
  int myY = ((rows - 2) / 2); // Initial Y
  int dir = 0;                // Direction of the movement of the snake.
  if (startingDir < 3) {
    dir = startingDir;
  } else {
    dir = (rand() % 3);
  }

  printStats(menu_win, rows, cols, myX, myY,
             snakeSpeed(snakeLen, maxSpeed, minSpeed), dir, expectedLen,
             snakeLen, trophyCount, trophyLost);
  struct body food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
  // printHead(menu_win, myY, myX, dir);
  // wrefresh(menu_win);
  refresh();

  while (1) {
    if (c == 0)
      c = wgetch(menu_win);
    switch (c) {
    case KEY_RIGHT:
      if (dir == left) {
        killsnake();
      } else
        dir = right;
      break;
    case KEY_LEFT:
      if (dir == right) {
        killsnake();
      } else
        dir = left;
      break;
    case KEY_UP:
      if (dir == down) {
        killsnake();
      } else
        dir = up;
      break;
    case KEY_DOWN:
      if (dir == up) {
        killsnake();
      } else
        dir = down;
      break;
    case KEY_BACKSPACE:
      endwin();
      exit(1);
    }

    mvprintw(0, 150, "             ");

    if (dir == up) {
      if (myY >= 2) {
        myY--;
      } else {
        killsnake();
      }
    }

    if (dir == down) {
      if (myY >= rows - 3) {
        killsnake();
      } else {
        myY++;
      }
    }

    if (dir == left) {
      if (myX >= 2) {
        myX--;
      } else {
        killsnake();
      }
    }

    if (dir == right) {
      if (myX >= (cols - 3)) {
        killsnake();
      } else {
        myX++;
      }
    }

    printStats(menu_win, rows, cols, myX, myY,
               snakeSpeed(snakeLen, maxSpeed, minSpeed), dir, expectedLen,
               snakeLen, trophyCount, trophyLost);

    snakeLen = growSeg(BodyStack, expectedLen, snakeLen, myX, myY, dir);
    printSnake(menu_win, BodyStack, snakeLen, maxSnake);

    /* This kills the trophy */
    if (foodExp > 0) {
      foodExp -= snakeSpeed(snakeLen, maxSpeed, minSpeed); // not yet
    } else {
      mvwprintw(menu_win, food.y, food.x, " "); // Erase that food in place.
      trophyLost++;
      if (trophyLost > trophyFade)
        trophyFade = trophyLost;
      
      food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2); // New
    }

    /* Check for food collision */
    if (BodyStack[snakeLen].x == food.x && BodyStack[snakeLen].y == food.y) {
      expectedLen += food.state;
      trophyCount++;
      if(trophyCount > trophyCapture)
      trophyCapture=trophyCount;

      food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
    }

    usleep(snakeSpeed(snakeLen, maxSpeed, minSpeed)); // hold rendering.

    if (c != 0) {
      c = 0;
      // flushinp();
    }
  }

  /* Reset the game back to the starting condition*/
  trophyFade = trophyLost;
  gameNum++;

  clrtoeol();
  refresh();
  endwin();
  return 0;
}

void printPit(WINDOW *menu_win, int y, int x, int dir) {}

/* Prints a single pip to act as the head */

void printSnake(WINDOW *menu_win, struct body *stack, int actLen, int maxLen) {
    int color;
    
    int snakeRank = 100*actLen/maxLen;
    if (snakeRank <=25)
    color = 1;
    else if (snakeRank >25 &&snakeRank <=50)
    color = 2;
    else if (snakeRank >50 &&snakeRank <=75)
    color = 3;
else {
color = 4;
}
    
    wattron(menu_win,COLOR_PAIR(color));
  int z = actLen;
  while (z >= 0) {
    if (z == 0)
    {
     wattroff(menu_win,COLOR_PAIR(color));
    }
    mvwprintw(menu_win, stack[z].y, stack[z].x, "%c", stack[z].body); //
    // mvprintw(0, 125, "Debug:%d  --",z );
    
    if (z > 0) {
      if (stack[z - 1].y == stack[actLen].y &&
          stack[z - 1].x == stack[actLen].x)
        killsnake();
    }

    z--;
  }

  refresh();
}

void printHead(WINDOW *menu_win, int y, int x, int dir) // Not in Use
{

  char head[] = "O";

  mvwprintw(menu_win, y, x, "%s", head);
  refresh();
  // wrefresh(menu_win);
}

/* Tosses random food (1~9) around the arena */

struct body randomFood(WINDOW *menu_win, struct body *stack, int quant, int row,
                       int col) {
  int x, y, z;
  struct body food;
  time_t t;
  srand((unsigned)time(&t));

  foodExp = (1 + (rand() % quant)+extraTime) * 1000000; // Food life in Microseconds.
  foodExpCeiling = foodExp;

  food.x = rand() % (col - 2) + 1;
  food.y = rand() % (row - 2) + 1;
  z = snakeLen;
  while (z > 0) {
    if (stack[z - 1].y == stack[snakeLen].y &&
        stack[z - 1].x == stack[snakeLen].x) {
      food.x = rand() % (col - 2) + 1;
      food.y = rand() % (row - 2) + 1;
      z = snakeLen;
    }
    z--;
  }
  food.state = 1 + (rand() % quant); // Between 1 and foodMax
  food.body = food.state + 48;       // number to char of that number
  mvwprintw(menu_win, food.y, food.x, "%c",
            food.body); // Print that food in place.

  return food;
}

/* returns a sleep value. The higher the number the slower the speed. */

int snakeSpeed(int len, int maxSpeed, int minSpeed) {
  // Speed is inversely represented as wait time. The higher the return, the
  // slower the speed.

  int speed = 275000;
  speed = speed - (len * 1500);

  if (speed <= maxSpeed) {
    return maxSpeed;
  }

  if (speed >= minSpeed) {
    return minSpeed;
  }
  return speed;
}

/* Conditions to kill the snake have been met */
void killsnake(int endgame) {
  clear();
  switch (endgame) {
  case 1:
    mvprintw(LINES / 2, COLS / 2 - sizeof(WIN) / 2,
             WIN); // print msg in center of screen
    break;
  default:
    mvprintw(LINES / 2, COLS / 2 - sizeof(LOSE) / 2,
             LOSE); // print msg in center of screen
  }

  refresh();

  usleep(2000000);
  endwin();
  exit(1);
}

/* Create a body segment for the snake */
int growSeg(struct body *stack, int exlen, int actLen, int x, int y, int dir) {
  // struct body *segment;
  int z = actLen;
  int swapX, swapY;
  /* Handle first grow */

  if (exlen > actLen) // Grow by one
  {
    z++; // add one to the stack
    stack[z] = buildBody(x, y);
    stack[z].state = head;
    z--;
    actLen++;
  } else {
    {
      while (z >= 0) {
        swapX = stack[z].x;
        swapY = stack[z].y;
        stack[z].y = y;
        stack[z].x = x;
        y = swapY;
        x = swapX;

        if (z == 1 && actLen > 1) {
          stack[z].state = tail;
          stack[z].body = 'o';
        }
        if (z == 0 && actLen > 1) {
          stack[z].state = eraser;
          stack[z].body = ' ';
        }

        z--;
      }
    }

    /* handle subsequent grows */

    // stack[z - 1].body = stack[z].body;

    if (z == 1 && actLen > 1) {
      stack[z].state = tail;
      stack[z].body = ' ';
    } else {
      stack[z].state = stack[z].state;
    }
    z--;
  }

  return actLen;
}

struct body buildBody(int x, int y) {
  struct body builder;
  builder.body = 'O';
  builder.x = x;
  builder.y = y;
  builder.state = body;

  return builder;
}

/* Draw the screen stats for the game */
void printStats(WINDOW *menu_win, int rows, int cols, int x, int y, int speed,
                int dir, int expLen, int actLen, int capt, int fade) {

  if (cols > 50 && cols < 100) {
    mvprintw(
        0, 0,
        "Round: %d | %d rem. Score: %d. Eaten: %d/%d | Missed: %d/%d.       ",
        gameNum, (rows + cols - actLen), actLen, capt, trophyCapture, fade,
        trophyFade);
  } else if (cols >= 100) {
    mvprintw(0, 0,
             "Snakes game: %d | %d to win. Current Score: %d. Trophies "
             "Captured: %d/%d | Trophies Lost: %d/%d.          ",
             gameNum, (rows + cols - actLen), actLen, capt, trophyCapture, fade,
             trophyFade);

  } else {
    mvprintw(0, 0, "G: %d | S: %d. T: %d/%d |   ", gameNum, actLen, capt,
             trophyCapture);
  }

  // refresh();
}
