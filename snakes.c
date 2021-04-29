// gcc -o snakeGame -lncurses snakes.c

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

/************************************* Game Settings*/
int FOOD = 9;
int maxSpeed = 50000;
int minSpeed = 250000;
int baseSpeed = 3;
int snakeLen = 0;
int expectedLen = 10; // if len < expected len, grow snake.
/***************************************** DataTypes*/

// DEFINED THINGS

#define LOSE "Game Over!!!"
#define WIN "You Won!!!"

enum dir
{
  right,
  left,
  up,
  down
};
enum state
{
  eraser, // to be ignored
  one,    // grow by x
  two,
  three,
  four,
  five,
  six,
  seven,
  eight,
  nine,
  head, // no food can spawn here.
  body, // collision = death, no food can spawn here
  tail  // collision = death, no food can spawn here. End of Body.
};

struct body
{
  char body; // How the body will appear
  int x;     // x coordinate of the body
  int y;     // y coordinate of the body
  int state; // unique collision
};

/*************************************** Prototypes*/

int growSeg();
struct body buildBody(int x, int y);
void printHead(WINDOW *menu_win, int x, int y, int dir);
void printSnake(WINDOW *menu_win, struct body stack[], int actLen, int dir);
void printStats(WINDOW *menu_win, int rows, int cols, int x, int y, int speed,
                int dir, int actLen, int expLen);
struct body randomFood(WINDOW *menu_win, struct body *stack, int, int, int);
int snakeSpeed(int len, int maxSpeed, int minSpeed);
void killsnake();

int collisionCheck();

int main()
{
  /* Settings for screen*/
  WINDOW *menu_win;
  initscr();
  clear();
  noecho();
  curs_set(0);
  start_color();
  cbreak(); /* Line buffering disabled. pass on everything */

  struct winsize wbuf;

  int cols, rows;
  int c;

  if (ioctl(0, TIOCGWINSZ, &wbuf) != -1)
  {
    cols = wbuf.ws_col;
    rows = wbuf.ws_row;
  }
  else
  {
    perror("Could not get screen dimensions");
    endwin();
    exit(1);
  }
  int maxSnake = (rows + cols) + FOOD; // maxium length before win condition.
  struct body BodyStack[maxSnake];
  /* Window settings */
  menu_win = newwin(rows - 1, cols - 1, 1,
                    1); // For offsets, -2, and another -1 to fit within the
                        // framework (-3 on the rows and cols)
  keypad(menu_win, TRUE);
  nodelay(menu_win, TRUE);
  box(menu_win, 0, 0);

  int myX = ((cols - 2) / 2);
  int myY = ((rows - 2) / 2);
  int dir = right;
  printStats(menu_win, rows, cols, myX, myY,
             snakeSpeed(snakeLen, maxSpeed, minSpeed), dir, snakeLen,
             expectedLen);
  struct body food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
  // printHead(menu_win, myY, myX, dir);
  wrefresh(menu_win);
  refresh();

  while (1)
  {
    if (c == 0)
      c = wgetch(menu_win);
    switch (c)
    {
    case KEY_RIGHT:
      if (dir == left)
      {
        killsnake();
      }
      else
        dir = right;
      break;
    case KEY_LEFT:
      if (dir == right)
      {
        killsnake();
      }
      else
        dir = left;
      break;
    case KEY_UP:
      if (dir == down)
      {
        killsnake();
      }
      else
        dir = up;
      break;
    case KEY_DOWN:
      if (dir == up)
      {
        killsnake();
      }
      else
        dir = down;
      break;
    case KEY_BACKSPACE:
      endwin();
      exit(1);
    }

    mvprintw(0, 150, "             ");

    if (dir == up)
    {
      // mvwprintw(menu_win, myY, myX, " ");
      if (myY >= 2)
      {
        myY--;
      }

      else
      {
        killsnake();
      }
    }

    if (dir == down)
    {
      // mvwprintw(menu_win, myY, myX, " ");
      if (myY >= rows - 3)
      {
        killsnake();
      }
      else
      {
        myY++;
      }
    }

    if (dir == left)
    {
      // mvwprintw(menu_win, myY, myX, " ");
      if (myX >= 2)
      {
        myX--;
      }
      else
      {
        killsnake();
      }
    }

    if (dir == right)
    {
      // mvwprintw(menu_win, myY, myX, " ");
      if (myX >= (cols - 3))
      {
        killsnake();
      }
      else
      {
        myX++;
      }
    }
    printStats(menu_win, rows, cols, myX, myY,
               snakeSpeed(snakeLen, maxSpeed, minSpeed), dir, snakeLen,
               expectedLen);
    //collisionCheck(BodyStack);

    snakeLen = growSeg(BodyStack, expectedLen, snakeLen, myX, myY, dir);
    printSnake(menu_win, BodyStack, snakeLen, dir);
    if (BodyStack[snakeLen].x == food.x && BodyStack[snakeLen].y == food.y)
    {
      expectedLen += food.state;
      food = randomFood(menu_win, BodyStack, FOOD, rows - 2, cols - 2);
    }
    // printHead(menu_win, myY, myX, dir);
    usleep(snakeSpeed(snakeLen, maxSpeed, minSpeed));

    if (c != 0)
    {
      c = 0;
      // flushinp();
    }
  }

  clrtoeol();
  refresh();
  endwin();
  return 0;
}

void printPit(WINDOW *menu_win, int y, int x, int dir) {}

/* Prints a single pip to act as the head */

void printSnake(WINDOW *menu_win, struct body *stack, int actLen, int dir)
{
  int z = actLen;
  mvprintw(0, 225, "Debug:%d %c--", z, stack[z].body);
  while (z >= 0)
  {
    //   mvprintw(0, 125, "Debug:%d ",z );
    mvwprintw(menu_win, stack[z].y, stack[z].x, "%c", stack[z].body); //
    // mvprintw(0, 125, "Debug:%d  --",z );
    if (z > 0)
    {
      if (stack[z - 1].y == stack[actLen].y && stack[z - 1].x == stack[actLen].x)
        killsnake();
    }

    z--;
  }
  /*
    if (expectedLen == actLen) {
      switch (dir) {
      case up:
        mvwprintw(menu_win, stack[1].y - 1, stack[1].x, " ");
        break;
      case down:
        mvwprintw(menu_win, stack[1].y + 1, stack[1].x, " ");
        break;
      case right:
        mvwprintw(menu_win, stack[1].y, stack[1].x - 1, " ");
        break;
      case left:
        mvwprintw(menu_win, stack[1].y, stack[1].x + 1, " ");
        break;
      }
    }
*/

  refresh();
}

void printHead(WINDOW *menu_win, int y, int x, int dir)
{
  char head[] = "O";

  mvwprintw(menu_win, y, x, "%s", head);
  refresh();
  // wrefresh(menu_win);
}

/* Tosses random food (1~9) around the arena */

struct body randomFood(WINDOW *menu_win, struct body *stack, int quant, int row,
                       int col)
{
  int x, y, z;
  time_t t;
  struct body food;
  srand((unsigned)time(&t));

  food.x = rand() % (col - 2) + 1;
  food.y = rand() % (row - 2) + 1;
  z = snakeLen;
  while (z > 0)
  {
    if (stack[z - 1].y == stack[snakeLen].y && stack[z - 1].x == stack[snakeLen].x)
    {
      food.x = rand() % (col - 2) + 1;
      food.y = rand() % (row - 2) + 1;
      z = snakeLen;
    }
    z--;
  }
  food.state = 1 + (rand() % quant);
  food.body = food.state + 48;
  mvwprintw(menu_win, food.y, food.x, "%c", food.body);

  return food;
}

/* returns a sleep value. The higher the number the slower the speed. */

int snakeSpeed(int len, int maxSpeed, int minSpeed)
{
  // Speed is inversely represented as wait time. The higher the return, the
  // slower the speed.

  int speed = 275000;
  speed = speed - (len * 1500);

  if (speed <= maxSpeed)
  {
    return maxSpeed;
  }

  if (speed >= minSpeed)
  {
    return minSpeed;
  }
  return speed;
}

/* Conditions to kill the snake have been met */
void killsnake(int endgame)
{

  clear();
  switch (endgame)
  {
  case 1:
    mvprintw(LINES / 2, COLS / 2 - sizeof(WIN) / 2, WIN); // print msg in center of screen
    break;
  default:
    mvprintw(LINES / 2, COLS / 2 - sizeof(LOSE) / 2, LOSE); // print msg in center of screen
  }

  refresh();

  usleep(2000000);
  endwin();
  exit(1);
}

/* Create a body segment for the snake */
int growSeg(struct body *stack, int exlen, int actLen, int x, int y, int dir)
{
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
  }
  else
  {
    {
      while (z >= 0)
      {
        swapX = stack[z].x;
        swapY = stack[z].y;
        stack[z].y = y;
        stack[z].x = x;
        y = swapY;
        x = swapX;

        if (z == 1 && actLen > 1)
        {
          stack[z].state = tail;
          stack[z].body = 'o';
        }
        if (z == 0 && actLen > 1)
        {
          stack[z].state = eraser;
          stack[z].body = ' ';
        }

        z--;
      }
    }

    /* handle subsequent grows */

    // stack[z - 1].body = stack[z].body;

    if (z == 1 && actLen > 1)
    {
      stack[z].state = tail;
      stack[z].body = ' ';
    }
    else
    {
      stack[z].state = stack[z].state;
    }
    z--;
  }

  return actLen;
}

struct body buildBody(int x, int y)
{
  struct body builder;
  builder.body = 'O';
  builder.x = x;
  builder.y = y;
  builder.state = body;

  return builder;
}

/* Draw the screen stats for the game */
void printStats(WINDOW *menu_win, int rows, int cols, int x, int y, int speed,
                int dir, int expLen, int actLen)
{
  mvprintw(0, 0,
           "Snakes! Backspace to quit. Number of rows: %d, Cols: %d.  My x is: "
           "%d. My y is:%d. My speed is: %d. My Actual Length is: %d My "
           "Expected Length is: %d:           ",
           rows, cols, x, y, speed, actLen, expLen);

  if (dir == left)
  {
    mvprintw(0, 200, "Direction: Left        ");
  }
  if (dir == right)
  {
    mvprintw(0, 200, "Direction: Right        ");
  }
  if (dir == up)
  {
    mvprintw(0, 200, "Direction: Up        ");
  }
  if (dir == down)
  {
    mvprintw(0, 200, "Direction: Down        ");
  }

  refresh();
}
