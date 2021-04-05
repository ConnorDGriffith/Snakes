// gcc -o arrows_auto -lncurses arrows_auto.c
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

int FOOD = 1;
enum dir { right, left, up, down };
void printHead(WINDOW *menu_win, int x, int y, int dir);
void randomFood(WINDOW *menu_win, int, int, int);
int snakeSpeed(int snakeLen, int maxSpeed, int minSpeed);

WINDOW *menu_win;
int main() {
  /* Settings for screen*/
  initscr();
  clear();
  noecho();
  curs_set(0);
  start_color();
  cbreak(); /* Line buffering disabled. pass on everything */

  struct winsize wbuf;
  int cols, rows;
  int c;

  if (ioctl(0, TIOCGWINSZ, &wbuf) != -1) {
    cols = wbuf.ws_col;
    rows = wbuf.ws_row;
  } else {
    perror("Could not get screen dimensions");
    endwin();
    exit(1);
  }

/* Window settings */ 
  menu_win = newwin(rows - 1, cols - 1, 1, 1);
  keypad(menu_win, TRUE);
  nodelay(menu_win, TRUE);
  box(menu_win, 0, 0);



  int myX = ((cols - 2) / 2);
  int myY = ((rows - 2) / 2);
  int dir = right;
  mvprintw(0, 0,
           "Snakes! Backspace to quit. Number of rows: %d, Cols: %d.  My x is: "
           "%d. My y is:%d             ",
           rows, cols, myX, myY);

  randomFood(menu_win, FOOD, rows - 2, cols - 2);
  printHead(menu_win, myY, myX, dir);
  wrefresh(menu_win);
  refresh();

  while (1) {
    c = wgetch(menu_win);
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

    mvprintw(0, 150, "             ");

    if (dir == up) {

      mvprintw(0, 150, "Direction: Up     ");
      mvwprintw(menu_win, myY, myX, " ");
      if (myY >= 2) {
        myY--;
      }

      else {
        myY = rows - 3;
      }
    }

    if (dir == down) {
      mvprintw(0, 150, "Direction: Down       ");
      mvwprintw(menu_win, myY, myX, " ");
      if (myY >= rows - 3) {
        myY = 1;
      } else {
        myY++;
      }
    }

    if (dir == left) {
      mvprintw(0, 150, "Direction: Left        ");
      mvwprintw(menu_win, myY, myX, " ");
      if (myX >= 2) {
        myX--;
      } else {
        myX = (cols - 3);
      }
    }

    if (dir == right) {
      mvprintw(0, 150, "Direction: Right          ");
      mvwprintw(menu_win, myY, myX, " ");
      if (myX >= (cols - 3)) {
        myX = 1;
      } else {
        myX++;
      }
    }

    if (c != 0) {

      c = 0;
    }

    mvprintw(0, 0,
             "Snakes! Backspace to quit. Number of rows: %d, Cols: %d.  My x "
             "is: %d. My y is: %d             ",
             rows, cols, myX, myY);
    // mvprintw(myY, myX, " ");
    printHead(menu_win, myY, myX, dir);
    usleep(snakeSpeed(3, 100000, 1000000));
  }

  clrtoeol();
  refresh();
  endwin();
  return 0;
}

void printPit(WINDOW *menu_win, int y, int x, int dir) {}

/* Prints a single pip to act as the head */

void printHead(WINDOW *menu_win, int y, int x, int dir) {
  char head[] = "O";

  mvwprintw(menu_win, y, x, "%s", head);
  refresh();
  // wrefresh(menu_win);
}

/* Tosses random food (1~9) around the arena */

void randomFood(WINDOW *menu_win, int quant, int row, int col) {
  int x, y, z;
  time_t t;
  srand((unsigned)time(&t));
  for (int i = 0; i < quant; i++) {
    x = rand() % col;
    y = rand() % row;
    z = 1 + (rand() % 9);
    mvwprintw(menu_win, y, x, "%d", z);
  }
  move(x, y);
  addch(FOOD);
}

/* returns a sleep value. The higher the number the slower the speed. */

int snakeSpeed(int snakeLen, int maxSpeed, int minSpeed) {
  int speed = 500000;
  speed -= (snakeLen * 10000);

  if (speed <= maxSpeed)
    return maxSpeed;

  if (speed >= minSpeed)
    return minSpeed;

  return speed;
}

