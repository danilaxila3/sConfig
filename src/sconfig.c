#include <locale.h>
#include <ncurses/ncurses.h>
#include <string.h>

int main(int argc, char *argv[]) {
  setlocale(LC_ALL, "");

  if (argc < 2) {
    printf("\e[34m\U0000f449 \e[0mUsage: sconf <file>\n");
    return 1;
  }

  int file_new = 0;
  FILE *file;
  file = fopen(argv[1], "r");

  if (file == NULL) {
    file = fopen(argv[1], "w+");
    file_new = 1;

    if (file == NULL) {
      printf("\e[31m\U000f0752 \e[0mCould not open file %s\n", argv[1]);
      return 1;
    }
  }

  char entries[128][3][128];
  int entries_num = 0;

  char line[128];
  char ch;

  while ((ch = fgetc(file)) != EOF) {
    line[strlen(line)] = ch;

    if (ch == '\n') {
      line[strlen(line) - 1] = '\0';

      int equals_pos = -1;

      for (int i = 0; i < (int)strlen(line); i++) {
        if (line[i] == '=') {
          equals_pos = i;
        }
      }

      if (equals_pos != -1) {
        char key[128], value[128];

        memcpy(key, line, equals_pos);
        memcpy(value, &line[0] + equals_pos + 1, strlen(line) - equals_pos - 1);

        if (key[strlen(key) - 1] == ' ') {
          key[strlen(key) - 1] = '\0';
        }

        if (value[0] == ' ') {
          memmove(value, value + 1, strlen(value));
        }

        memcpy(entries[entries_num][0], "value", 6);
        memcpy(entries[entries_num][1], key, 128);
        memcpy(entries[entries_num][2], value, 128);
        entries_num += 1;
      }

      if (line[0] == '[' && line[strlen(line) - 1] == ']') {
        char section[128];

        memcpy(section, &line[0] + 1, strlen(line) - 2);

        memcpy(entries[entries_num][0], "section", 8);
        memcpy(entries[entries_num][1], section, 128);
        entries_num += 1;
      }

      memset(line, 0, 128);
    }
  }

  fclose(file);

  initscr();
  noecho();
  keypad(stdscr, 1);
  curs_set(0);
  start_color();

  init_pair(1, COLOR_BLACK, COLOR_BLACK);
  init_pair(2, COLOR_YELLOW, COLOR_BLACK);

  int running = 1;
  int selection = 0;
  int mode = 0;
  int edit_mode = 0;
  char input_buf[128] = "";
  int input_buf_cursor = 0;

  while (running) {
    clear();

    for (int i = 0; i < entries_num; i++) {
      for (int j = 0; j < COLS; j++) {
        mvprintw(i, j, " ");
      }

      mvprintw(i, 0, "%3d", i + 1);

      if (strcmp(entries[i][0], "value") == 0) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(i, COLS / 2, "│");
        attroff(COLOR_PAIR(1) | A_BOLD);

        if (i != selection) {
          attron(COLOR_PAIR(1) | A_BOLD);
        }

        mvprintw(i, 4, "%s", entries[i][1]);
        mvprintw(i, COLS / 2 + 1, "%s", entries[i][2]);

        attroff(COLOR_PAIR(1) | A_BOLD);
      }

      if (strcmp(entries[i][0], "section") == 0) {
        attron(COLOR_PAIR(1) | A_BOLD);

        for (int j = 4; j < COLS; j++) {
          mvprintw(i, j, "─");
        }

        if (i != selection) {
          attron(COLOR_PAIR(1) | A_BOLD);
        } else {
          attroff(COLOR_PAIR(1) | A_BOLD);
        }

        mvprintw(i, 4, "%s ", entries[i][1]);

        attroff(COLOR_PAIR(1) | A_BOLD);
      }
    }

    if (mode == 1) {
      for (int i = 0; i < COLS; i++) {
        mvprintw(LINES - 2, i, " ");
      }

      mvprintw(LINES - 2, 0, "%s", input_buf);

      attron(A_REVERSE);

      if (input_buf[input_buf_cursor] == '\0') {
        mvprintw(LINES - 2, input_buf_cursor, " ");
      } else {
        mvprintw(LINES - 2, input_buf_cursor, "%c",
                 input_buf[input_buf_cursor]);
      }
      attroff(A_REVERSE);
    }

    if (file_new) {
      mvprintw(LINES - 1, 0, "sConfig: %s [New]", argv[1]);
    } else {
      mvprintw(LINES - 1, 0, "sConfig: %s", argv[1]);
    }

    int input = getch();

    if (mode == 0) {
      if (input == 'q') {
        file = fopen(argv[1], "w");

        for (int i = 0; i < entries_num; i++) {
          if (strcmp(entries[i][0], "value") == 0) {
            fprintf(file, "%s = %s\n", entries[i][1], entries[i][2]);
          }

          if (strcmp(entries[i][0], "section") == 0) {
            fprintf(file, "[%s]\n", entries[i][1]);
          }
        }

        fclose(file);
        running = 0;
      }

      if (input == KEY_DOWN || input == 'j') {
        selection = (selection == entries_num - 1) ? 0 : selection + 1;
      }

      if (input == KEY_UP || input == 'k') {
        selection = (selection == 0) ? entries_num - 1 : selection - 1;
      }

      if (input == KEY_LEFT || input == 'h') {
        mode = 1;
        edit_mode = 0;
        memset(input_buf, 0, 128);
        strcpy(input_buf, entries[selection][1]);
        input_buf_cursor = strlen(entries[selection][1]);
      }

      if ((input == KEY_RIGHT || input == 'l') &&
          strcmp(entries[selection][0], "value") == 0) {
        mode = 1;
        edit_mode = 1;
        memset(input_buf, 0, 128);
        strcpy(input_buf, entries[selection][2]);
        input_buf_cursor = strlen(entries[selection][2]);
      }

      if (input == 'a') {
        mode = 1;
        edit_mode = 2;
        memset(input_buf, 0, 128);
        input_buf_cursor = 0;
      }

      if (input == 'A') {
        mode = 1;
        edit_mode = 3;
        memset(input_buf, 0, 128);
        input_buf_cursor = 0;
      }

      if (input == 'd') {
        for (int i = selection; i < 127; i++) {
          strcpy(entries[i][0], entries[i + 1][0]);
          strcpy(entries[i][1], entries[i + 1][1]);
          strcpy(entries[i][2], entries[i + 1][2]);
        }
        entries_num--;
      }
    } else if (mode == 1) {
      if (input >= 32 && input <= 128) {
        memmove(&input_buf[input_buf_cursor + 1], &input_buf[input_buf_cursor],
                127);
        input_buf[input_buf_cursor] = input;
        input_buf_cursor++;
      }

      if (input == KEY_BACKSPACE) {
        input_buf_cursor--;
        memmove(&input_buf[input_buf_cursor], &input_buf[input_buf_cursor + 1],
                127);
        input_buf_cursor = (input_buf_cursor == -1) ? 0 : input_buf_cursor;
      }

      if (input == KEY_LEFT) {
        input_buf_cursor--;
        input_buf_cursor = (input_buf_cursor == -1) ? 0 : input_buf_cursor;
      }

      if (input == KEY_RIGHT) {
        input_buf_cursor++;
        input_buf_cursor = (input_buf_cursor == (int)strlen(input_buf) + 1)
                               ? (int)strlen(input_buf)
                               : input_buf_cursor;
      }

      if (input == '\n') {
        mode = 0;

        if (edit_mode >= 0 && edit_mode <= 1) {
          strcpy(entries[selection][1 + edit_mode], input_buf);
        }

        if (edit_mode == 2) {
          strcpy(entries[entries_num][0], "value");
          strcpy(entries[entries_num][1], input_buf);
          strcpy(entries[entries_num][2], "0");
          entries_num++;
        }

        if (edit_mode == 3) {
          strcpy(entries[entries_num][0], "section");
          strcpy(entries[entries_num][1], input_buf);
          entries_num++;
        }
      }
    }
  }

  endwin();

  return 0;
}
