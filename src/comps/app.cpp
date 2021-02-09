#include "ncurses.h"
#include "panel.h"
#include "menu.h"
#include <string>
#include <fstream>
#include <algorithm>

#include "app.h"

void App::init_windows() {
  WINDOW* padding_win{newwin(m_padding_window_height, m_padding_window_width, m_padding_window_start_y, m_padding_window_start_x)};
  box(padding_win, 0, 0);
  wrefresh(padding_win);
  for(int i{0}; i < m_num_wins; ++i) {
    if(i == (m_num_wins - 1)) {
      m_wins[i] = newwin(10, 36, (LINES - 19)/2, (COLS - 16)/2);
      keypad(m_wins[i], true);
      m_pans[i] = new_panel(m_wins[i]);
      hide_panel(m_pans[i]);
      ++m_open_windows;
    } else {
      m_wins[i] = newwin(m_height, m_main_window_width, m_start_y, m_main_window_start_x);
      m_pans[i] = new_panel(m_wins[i]);
      if(i == 0) {
        m_top_panel = m_pans[i];
        ++m_open_windows;
      }
      else {
        hide_panel(m_pans[i]);
      }
    }
  }
}

void App::init_data() {
  for(int i{0}; i < m_num_wins; ++i) {
    if(i == 0) {
      m_data[i].hidden = false;
      m_data[i].selection_box = m_pans[m_num_wins - 1];
      m_data[i].open = false;
      m_data[i].file_name = "Main Menu";
    } else {
      m_data[i].hidden = true;
      m_data[i].menu_switch = m_pans[0];
      m_data[i].open = true;
      m_data[i].file_name = "";
    } 
    m_data[i].w_array_pos = i;
    set_panel_userptr(m_pans[i], &m_data[i]);
  }
}

void App::init_menu() {
  m_items[0] = new_item("Open", "->");
  m_items[1] = new_item("New", "->");
  m_items[2] = new_item("Close", "->");
  m_items[3] = new_item("Delete", "->");
  m_items[4] = new_item("Exit", "->");
  m_items[5] = new_item("Save", "->");
  m_items[6] = nullptr;

  m_wins[0] = newwin(m_padding_window_height, m_main_menu_width, m_padding_window_start_y, m_main_menu_start_x);
  box(m_wins[0], 0, 0);
  replace_panel(m_pans[0], m_wins[0]);
  keypad(m_wins[0], true);

  m_menu = new_menu(m_items);
  set_menu_win(m_menu, m_wins[0]);
  set_menu_sub(m_menu, derwin(m_wins[0], 15, 15, 1,1));

  post_menu(m_menu);
}

void App::print_title(PANEL* pan) {
  WINDOW* temp_win{panel_window(pan)}; 
  std::string temp_text{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(pan)))->file_name};
  int string_length{static_cast<int>(temp_text.length())};
  int w_cols{getmaxx(temp_win)};
  wattron(temp_win, A_BOLD);
  mvwprintw(temp_win, 1, (w_cols - string_length)/2, temp_text.c_str());
  wattroff(temp_win, A_BOLD);
  wrefresh(temp_win);
}

std::string App::create_option_box(std::string box_title, PANEL* pan) {
  int input{0};
  std::string input_file_name{""};
  WINDOW* border_win = newwin(12, 40, (LINES - 20)/2, (COLS - 20)/2);
  box(border_win, 0 , 0);
  wrefresh(border_win);
  m_top_panel = pan;
  top_panel(m_top_panel);
  mvwprintw(panel_window(m_top_panel), 1, (getmaxx(panel_window(m_top_panel)) - box_title.length())/2, box_title.c_str());
  wmove(panel_window(m_top_panel), 2,2);
  bool loop{true};
  while(loop) {
    input = wgetch(panel_window(m_top_panel));
    if(input == 10) {
      loop = false;
      break;
    } else if(input == KEY_BACKSPACE) {
      int y, x;
      getyx(panel_window(m_top_panel), y, x);
      mvwdelch(panel_window(m_top_panel), y, x - 1);
      input_file_name.pop_back();
    } else {
      waddch(panel_window(m_top_panel), input);
      input_file_name += input;
    }
  }
    delwin(border_win);
    wmove(panel_window(m_top_panel), 2,2);
    wclrtoeol(panel_window(m_top_panel));
    hide_panel(m_top_panel);

  return input_file_name;
}

int App::find_open_window() {
  for(int i{0}; i < m_num_wins - 1; ++i) {
    if(const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[i])))->open)
      return i;
  }
  return -1;
} 

void App::menu_loop(const int ch, int& index, bool& loop) {
  switch(ch) {
    case KEY_UP: {
      if(index == 0) {
        menu_driver(m_menu, REQ_LAST_ITEM);
        index = m_num_items - 1;
        break;
      } else {
        menu_driver(m_menu, REQ_UP_ITEM);
        --index;
        break;
      }
    }
    case KEY_DOWN: {
      if(m_top_panel == m_pans[0]) {
        if(index == m_num_items - 1) {
          menu_driver(m_menu, REQ_FIRST_ITEM);
          index = 0;
          break;
        } else {
          menu_driver(m_menu, REQ_DOWN_ITEM);
          ++index;
          break;
        }
      } 
    }
    case KEY_F(1): {
      PANEL* temp_pan{m_top_panel};
      m_top_panel = const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch;
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch = temp_pan;
      top_panel(m_top_panel);
      break;
    }
    case 10: {
      switch(index) {
        case 1: {
          Data* menu_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))};
          std::string new_buffer_title{create_option_box("OPEN", menu_data->selection_box)};
          if(m_open_windows < m_num_wins) {
            ++m_open_windows;
            int available_slot{find_open_window()};
            Data* new_pan_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[available_slot])))};
            Data* prev_pan_data;
            new_pan_data->next = m_pans[1];
              new_pan_data->end = true;
              new_pan_data->open = false;
            new_pan_data->file_name = new_buffer_title.c_str();
            if(m_open_windows == 3) {
              new_pan_data-> prev = m_pans[1];
              new_pan_data->start = true;
              m_end_panel = m_pans[1];
              m_start_panel = m_pans[1];
            } else {
              prev_pan_data = const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_end_panel)));
              new_pan_data->prev = m_end_panel;
              new_pan_data->end = true;
              new_pan_data->next = m_start_panel;
              prev_pan_data->end = false;
              m_end_panel = m_pans[available_slot];
              Data* first_pan_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_start_panel)))};
              first_pan_data->prev = m_end_panel;
            // prev_pan_data->prev = m_pans[m_open_windows - 4];
              prev_pan_data->next = m_end_panel;
            }
            print_title(m_end_panel);
            m_top_panel = m_end_panel;
            const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = m_top_panel;
            top_panel(m_top_panel);
            keypad(panel_window(m_top_panel), true);
          }
          break;
        }
        case 4: {
          loop = false;
          break;
        }
        case 5: {
          std::ofstream myfile;
          PANEL* prev_panel{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch};
          Data* temp_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(prev_panel)))};
          myfile.open("./" + temp_data->file_name);
          myfile << temp_data->buffer << '\n';
          myfile.close();
          break;
        }
        break;
      }
    }
  }
}

void App::notes_window_loop(const int ch) {
  int y, x;
  int begy, begx;
  int maxy, maxx;
  getyx(panel_window(m_top_panel), y, x);
  getbegyx(panel_window(m_top_panel), begy, begx);
  getmaxyx(panel_window(m_top_panel), maxy, maxx);
  keypad(panel_window(m_top_panel), true);
  switch(ch) {
    case KEY_UP: {
      if(y > begy + 1) {
        wmove(panel_window(m_top_panel), y - 1, x);
      }
      break;
    }
    case KEY_DOWN: {
      if(y < maxy - 1) {
        wmove(panel_window(m_top_panel), y + 1, x);
      }
      break;
    }
    case KEY_LEFT: {
      if(x > begx + 1) {
        wmove(panel_window(m_top_panel), y, x - 1);
      }
      break;
    }
    case KEY_RIGHT: {
      if(x < maxx - 1) {
        wmove(panel_window(m_top_panel), y, x + 1);
      }
      break;
    }
    case KEY_F(1): {
      PANEL* temp_pan{m_top_panel};
      m_top_panel = const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch;
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch = temp_pan;
      top_panel(m_top_panel);
      break;
    }
    case KEY_F(2): {
      PANEL* t_panel{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->prev};
      m_top_panel = t_panel ? t_panel : m_top_panel;
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = t_panel;
      top_panel(m_top_panel);
      break;
    }
    case KEY_F(3): {
      PANEL* t_panel{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->next};
      m_top_panel = t_panel ? t_panel : m_top_panel;
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = t_panel;
      top_panel(m_top_panel);
      break;
    }
    case KEY_F(5): {
      Data* temp{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))};
      wclear(panel_window(m_top_panel));
      PANEL* previous{temp->prev};
      PANEL* next{temp->next};
      if(m_top_panel == m_start_panel) {
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(next)))->start = true;
        temp->start = false;
        m_start_panel = next;
      }
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(previous)))->next = temp->next;
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(next)))->prev = temp->prev;
      temp->buffer = "";
      temp->file_name = "";
      if(temp->prev != m_top_panel) {
        m_top_panel = temp->prev;
        // wrefresh(panel_window(m_top_panel));
        top_panel(m_top_panel);
      } else if(temp->next != m_top_panel) {
        m_top_panel = temp->next;
        // wrefresh(panel_window(m_top_panel));
        top_panel(m_top_panel);
      }
      break;
    }
    case 10: {
      waddch(panel_window(m_top_panel), ch);
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer += "\n";
      break;
    }
   case KEY_BACKSPACE: {
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer.pop_back();
      mvwdelch(panel_window(m_top_panel), y, x - 1);
      break;
    } 
    default: {
      waddch(panel_window(m_top_panel), ch);
      const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer += ch;
    }
  }
}

void App::main_loop() {
  bool loop{true};
  int index{0};
  while(loop) {
    int ch(wgetch(panel_window(m_top_panel)));
    if(m_top_panel == m_pans[0]) {
    menu_loop(ch, index, loop);
    } else {
      notes_window_loop(ch);
    }
    update_panels();
    doupdate();
  }
}
