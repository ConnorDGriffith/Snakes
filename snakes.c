// gcc -o snakeGame -lncurses snakes.c to compile

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
int startingLen = 3; // for game resets
int expectedLen = 3; // Starting Length
int startingDir = 4;  // Right: 0, Left: 1, Up: 2, Down: 3, Random: 4
int extraTime = 5; // Add additional time to the Trophy countdown (in seconds)

/************************************* Functional Globals*/

int trophyCapture = 0; // Persists: Last number of Trophy captured
int trophyFade = 0;    // Persists: Prev Game Faded
int gameCont = 1;      // Tells the game to continue.
int gameNum = 1;       // How many games have been played.

int snakeLen = 0;                           // Actual length of the snake.
int foodExp, foodExpCeiling, foodFlash = 0; // lifespan of tropy

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
  one,     // grow by x = 1
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine, // ... grow by x = 9
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

int growSeg(struct body *stack, int exlen, int actLen, int x, int y, int dir);
struct body buildBody(int x, int y);
void printHead(WINDOW *menu_win, int x, int y, int dir);
void printSnake(WINDOW *menu_win, struct body *stack, int actLen, int maxLen,
                int rows, int cols);
void printStats(WINDOW *menu_win, int rows, int cols, int x, int y, int speed,
                int dir, int expLen, int actLen, int capt, int fade);
struct body randomFood(WINDOW *menu_win, struct body *stack, int, int, int);
int snakeSpeed(int len, int maxSpeed, int minSpeed);
void endGame();

void printFood(WINDOW *menu_win, struct body food);

int main() {
  /************************************* Settings for screen*/

  WINDOW *menu_win;
  initscr();   // Initialize ncurses
  clear();     // Clear anything on screen (cut?)
  noecho();    // Turn off key echo
  curs_set(0); // Hide the cursor
  cbreak();    // Disable line Buffering

  /************************************* Define colors for use*/
  if (has_colors()) {
    start_color();

    init_pair(1, COLOR_CYAN, COLOR_GREEN);    // New Snake
    init_pair(2, COLOR_RED, COLOR_GREEN);     // Beginner Snake
    init_pair(3, COLOR_YELLOW, COLOR_GREEN);  // Intermediate Snake
    init_pair(4, COLOR_BLUE, COLOR_GREEN);    // Professional Snake
    init_pair(5, COLOR_GREEN, COLOR_WHITE);   // Food New
    init_pair(6, COLOR_MAGENTA, COLOR_WHITE); // Food Mid
    init_pair(7, COLOR_RED, COLOR_WHITE);     // Food Fading
    init_pair(8, COLOR_RED, COLOR_BLACK);     // Flash food 1
    init_pair(9, COLOR_WHITE, COLOR_RED);     // Flash food 2
  }

  /* Primary Infinit Loop */
  while (1) {

    /************************************ Primary (Local) Variables*/

    struct winsize wbuf;

    int cols, rows; // Holds the rows and columns for the screen
    int c;          // Receives input
    int trophyCount = 0;
    int trophyLost = 0;

    /* Capture window dimensions */
    if (ioctl(0, TIOCGWINSZ, &wbuf) != -1) {
      cols = wbuf.ws_col;
      rows = wbuf.ws_row;
    } else {
      perror("Could not get screen dimensions");
      endwin();
      exit(1);
    }
    // Provide a warning of game start

    mvprintw((rows / 2), (cols / 2) - 9, "Game Starting in....");
    refresh();
    for (int i = 3; i > 0; i--) {
      sleep(1);
      clear();
      mvprintw((rows / 2), cols / 2, "%d", i);
      refresh();
    }
    mvprintw((rows / 2), (cols / 2) - 3, "GO!!!!");
    sleep(1);

    time_t t;                  // Get the time
    srand((unsigned)time(&t)); // Seed the random number generator

    int maxSnake = rows + cols; // maximum length before win condition.
    struct body BodyStack[maxSnake + FOOD]; // Space for the snake to grow into
                                            // the win condition

    /**************************** Window settings */

    menu_win = newwin(rows - 1, cols - 1, 1,
                      1);   // For offsets, -2, and another -1 to fit within the
                            // framework (-3 on the rows and cols)
    keypad(menu_win, TRUE); // Enable Keypad for arrow use.
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
    struct body food =
        randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
    // printHead(menu_win, myY, myX, dir);
    // wrefresh(menu_win);
    refresh();

    while (gameCont) { // While the game should loop (inner loop)
      if (c == 0)
        c = wgetch(menu_win); // get any input
      switch (c) {

      case KEY_RIGHT:
        dir = right;
        break;

      case KEY_LEFT:
        dir = left;
        break;

      case KEY_UP:
        dir = up;
        break;

      case KEY_DOWN:
        dir = down;
        break;

      case KEY_BACKSPACE:
        endwin();
        exit(1);
      }

      /*Auto increment by direction*/
      if (dir == up) {
        myY--;
      }

      if (dir == down) {
        myY++;
      }

      if (dir == left) {
        myX--;
      }

      if (dir == right) {
        myX++;
      }

      printStats(menu_win, rows, cols, myX, myY,
                 snakeSpeed(snakeLen, maxSpeed, minSpeed), dir, expectedLen,
                 snakeLen, trophyCount, trophyLost);

      // Grow the snake

      snakeLen = growSeg(BodyStack, expectedLen, snakeLen, myX, myY, dir);
      // Print the new locations. Also test collision
      printSnake(menu_win, BodyStack, snakeLen, maxSnake, rows, cols);

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
        if (trophyCount > trophyCapture)
          trophyCapture = trophyCount;

        food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
      }
      printFood(menu_win, food); // Always print the food.

      /* check WIN condition */

      if (snakeLen >= maxSnake) {
        endGame(1);
      }

      refresh();
      usleep(snakeSpeed(snakeLen, maxSpeed, minSpeed)); // hold rendering.

      if (c != 0) {
        c = 0;
        // flushinp();
      }
    }

    /* Reset the game back to the starting condition*/
    trophyFade = trophyLost;
    trophyCapture = trophyCount;
    snakeLen = 0;              // reset the snake.
    expectedLen = startingLen; // reset the starting length
    gameNum++;                 // next game
    gameCont = 1;              // Resume game
  }

  return 0;
}

/* Prints the snake and tests collision */

void printSnake(WINDOW *menu_win, struct body *stack, int actLen, int maxLen,
                int rows, int cols) {
  // Color Snake based on growth.
  int color;
  int z = actLen;
  int snakeRank = 100 * actLen / maxLen;
  if (snakeRank <= 25)
    color = 1;
  else if (snakeRank > 25 && snakeRank <= 50)
    color = 2;
  else if (snakeRank > 50 && snakeRank <= 75)
    color = 3;
  else {
    color = 4;
  }

  wattron(menu_win, COLOR_PAIR(color)); // change color based on progress.
  // Do the actual printing. If eraser, turn off color.
  while (z >= 0) {
    if (z == 0) {
      wattroff(menu_win, COLOR_PAIR(color));
    }
    mvwprintw(menu_win, stack[z].y, stack[z].x, "%c", stack[z].body); //

    /******************** Collisions handling */
    // if x is out of bounds
    if (stack[actLen].x <= 0 || stack[actLen].x >= cols - 2) {
      endGame();
      z = 0;
    }
    // if y is out of bounds
    if (stack[actLen].y <= 0 || stack[actLen].y >= rows - 2) {
      endGame();
      z = 0;
    }
    z--;

    // if check for a body collision.
    if (z > 0) {
      if (stack[z].y == stack[actLen].y && stack[z].x == stack[actLen].x) {
        endGame();
        z = 0;
      }
    }
  }
}

/* Tosses random food (1~FOOD) around the arena */

struct body randomFood(WINDOW *menu_win, struct body *stack, int quant, int row,
                       int col) {
  int x, y, z;
  struct body food;
  time_t t;

  srand((unsigned)time(&t));

  foodExp = (1 + (rand() % quant) + extraTime) *
            1000000; // Food life in Microseconds.
  foodExpCeiling = foodExp;

  // Randomize coordinates for food.
  food.x = rand() % (col - 2) + 1;
  food.y = rand() % (row - 2) + 1;
  z = snakeLen;

  // Make sure the food is not spawned on a body.
  while (z > 0) {
    if (stack[z].y == food.y && stack[z].x == food.x) {
      food.x = rand() % (col - 2) + 1;
      food.y = rand() % (row - 2) + 1;
      z = snakeLen;
    }
    z--;
  }
  // Generate value of food.
  food.state = 1 + (rand() % quant); // Between 1 and foodMax
  food.body = food.state + 48;       // number to char of that number

  return food;
}

/* Prints food and stylizes it based on food lifespan. */

void printFood(WINDOW *menu_win, struct body food) {
  // Color food based on lifespan
  int color;
  int foodHealth = 100 * foodExp / foodExpCeiling;

  if (foodHealth <= 25) {
    // Flash nearly expired food
    if (foodFlash == 0) {
      color = 8;
      foodFlash = 1;
    } else {
      color = 9;
      foodFlash = 0;
    }
  } else if (foodHealth > 25 && foodHealth <= 50) {
    color = 7;
  } else if (foodHealth > 50 && foodHealth <= 75) {
    color = 6;
  } else {
    color = 5;
  }

  // Activate color attributes
  wattron(menu_win, A_BOLD);
  wattron(menu_win, COLOR_PAIR(color));
  // Print the actual food.
  mvwprintw(menu_win, food.y, food.x, "%c",
            food.body); // Print that food in place.
  wattroff(menu_win, A_BOLD);
  wattroff(menu_win, COLOR_PAIR(color));
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

/* Conditions to end the game have been met */

void endGame(int endgame) {
  usleep(100000);

  gameCont = 0; // This will break the internal while loop (end game).
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

  // Begin Countdown to restart
  refresh();
  usleep(1000000);
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

    z--;
  }

  return actLen;
}

/* Build a basic body unit. */

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
}
