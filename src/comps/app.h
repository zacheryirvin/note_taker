#ifndef APP_H
#define APP_H

#include <ncurses.h>
#include <menu.h>
#include <panel.h>

#include <string>

class App {
  struct Data {
    bool hidden;
    PANEL* next;
    PANEL* prev;
    PANEL* selection_box;
    PANEL* menu_switch;
    std::string buffer{""};
    std::string file_name{""};
  };
  private:
    WINDOW* m_padding_win;
    WINDOW** m_wins;
    PANEL** m_pans;
    PANEL* m_top_panel;
    MENU* m_menu;
    ITEM** m_items;
    Data* m_data;

    int m_open_windows{0};
    int m_num_wins{12};
    int m_num_items{5};

    int m_height;
    int m_start_y;
    int m_main_window_width;
    int m_main_window_start_x;
    int m_main_menu_width;
    int m_main_menu_start_x;
    int m_padding_window_height;
    int m_padding_window_width;
    int m_padding_window_start_x;
    int m_padding_window_start_y;

    void init_windows();
    void init_menu();
    void init_data();
    void menu_loop();
    void print_title(PANEL* pan);
    std::string create_option_box(std::string box_title, PANEL* pan);
  public:
    App(const int wins=12,const int items=6) 
    : m_wins{new WINDOW*[wins]}, m_pans{new PANEL*[wins]}, m_items{new ITEM*[items]}, m_data{new Data[wins]}, m_num_wins{wins}, m_num_items{items}
  {
      initscr();
      start_color();
      keypad(stdscr, true);
      noecho();
      cbreak();
      m_main_menu_width = 16;
      m_main_menu_start_x = 0;

      m_padding_window_height = LINES;
      m_height = m_padding_window_height - 2;

      m_padding_window_width = COLS - (m_main_menu_width + 1);
      m_main_window_width = m_padding_window_width - 4;

      m_padding_window_start_x = m_main_menu_width + 1;
      m_main_window_start_x = m_padding_window_start_x + 2;

      m_padding_window_start_y = 0;
      m_start_y = m_padding_window_start_y + 1;

      refresh();
      init_windows();
      init_menu();
      init_data();
      update_panels();
      doupdate();
      menu_loop();
    }

  ~App() {
    unpost_menu(m_menu);
    for(int i{0}; i < m_num_items; ++i) {
      delete[] m_items[i];
      m_items[i] = nullptr;
    }
    for(int i{0}; i < m_num_wins; ++i) {
      delete[] m_pans[i];
      m_pans[i] = nullptr;
    }
    delete[] m_menu;
    delete[] m_wins;
    delete[] m_pans;
    m_menu = nullptr;
    m_wins = nullptr;
    m_pans = nullptr;
    m_data = nullptr;
    endwin();
  }
};
#endif
