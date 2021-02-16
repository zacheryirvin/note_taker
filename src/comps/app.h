#ifndef APP_H
#define APP_H

#include <ncurses.h>
#include <menu.h>
#include <panel.h>
#include <array>
#include <string>
#include <vector>

class App {
  using l_buffer = std::vector<char>;
  using w_buffer = std::vector<l_buffer>;

  struct Data {
    bool hidden;
    bool start;
    bool end;
    PANEL* next;
    PANEL* prev;
    PANEL* selection_box;
    PANEL* menu_switch;
    int w_current_line;
    int w_current_col;
    bool open;
    w_buffer buffer{};
    std::string file_name{""};
  };

  private:
    WINDOW* m_padding_win;
    WINDOW** m_wins;
    PANEL** m_pans;
    PANEL* m_top_panel;
    PANEL* m_end_panel;
    PANEL* m_start_panel;
    MENU* m_menu;
    // MENU* m_open_menu;
    // ITEM** m_open_items;
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
    int m_current_line{2};
    int m_current_column{0};

    void init_windows();
    void init_menu();
    void init_data();
    void menu_loop(const int ch, int& index, bool& loop);
    void notes_window_loop(int ch);
    void main_loop();
    void print_title(PANEL* pan);
    int find_open_window();
    void add_char(char ch, Data& data);
    void del_char(Data& data);
    void show_text(const w_buffer buffer, int start, int end);
    std::string open_file_box(std::string box_title, PANEL* pan);
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
      main_loop();
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
