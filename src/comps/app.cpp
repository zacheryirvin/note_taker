#include "ncurses.h"
#include "panel.h"
#include "menu.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <regex>
#include <filesystem>
namespace fs = std::filesystem;

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
    m_data[i].w_current_col = 0;
    m_data[i].w_current_line = 2;
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

std::string App::open_file_box(std::string box_title, PANEL* pan) {
  std::string path{"/home/zac/projects/ncurses/note_taker/build/notes/"};
  MENU* open_menu;
  ITEM** open_items;
  std::vector<std::string> file_read_in{};
  std::regex parse_file("[^/]+$");
  std::smatch o_match;
  std::string open_file{};

  WINDOW* border_win = newwin(13, 40, (LINES - 20)/2, (COLS - 20)/2);
  box(border_win, 0, 0);
  wrefresh(border_win);
  m_top_panel = pan;
  top_panel(m_top_panel);
  mvwprintw(panel_window(m_top_panel), 1, (getmaxx(panel_window(m_top_panel)) - box_title.length())/2, box_title.c_str());
  for(const auto& f : fs::directory_iterator(path)) {
    std::string temp{f.path()};
    if(std::regex_search(temp, o_match, parse_file))
      file_read_in.push_back(o_match.str());
  }

  open_items = new ITEM*[file_read_in.size()];
  open_items[file_read_in.size()] = nullptr;
  for(int i{0}; i < static_cast<int>(file_read_in.size()); ++i) {
    open_items[i] = new_item(file_read_in[i].c_str(), "->");
  }
  open_menu = new_menu(open_items);
  set_menu_win(open_menu, panel_window(m_top_panel));
  set_menu_sub(open_menu, derwin(panel_window(m_top_panel), 5, 15, 2, 1));
  post_menu(open_menu);
  wrefresh(panel_window(m_top_panel));
  int index{0};
  bool loop{true};
  while(loop) {
    int ch{wgetch(panel_window(m_top_panel))};
    switch(ch) {
      case KEY_UP: {
        if(index == 0) {
          menu_driver(open_menu, REQ_LAST_ITEM);
          index = file_read_in.size() -1;
        } else {
          menu_driver(open_menu, REQ_UP_ITEM);
          --index;
        }
        break;
      }
      case KEY_DOWN: {
        if(index == static_cast<int>(file_read_in.size()) -1) {
          menu_driver(open_menu, REQ_FIRST_ITEM);
          index = 0;
        } else {
          menu_driver(open_menu, REQ_DOWN_ITEM);
          ++index;
        }
        break;
      }
      case 10: {
        open_file = file_read_in[index];
        loop = false;
        break;
      }
    }
  }

  unpost_menu(open_menu);
  for(int i{0}; i < static_cast<int>(file_read_in.size()); ++i) {
    delete[] open_items[i];
    open_items[i] = nullptr;
  }
  delete[] open_menu;
  open_menu = nullptr;
  delwin(border_win);
  wclear(panel_window(m_top_panel));
  hide_panel(m_top_panel);

  return open_file;
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

void App::add_line(Data& data) {
  if(data.buffer.empty()) {
    l_buffer temp{static_cast<char>('\n')};
    l_buffer ttemp{};
    data.buffer.push_back(temp);
    data.buffer.push_back(ttemp);
  }
  if(data.buffer[m_current_line - 2].empty()) {
    l_buffer ttemp{};
    if(m_current_line - 2 == static_cast<int>(data.buffer.size() -1)) {
      data.buffer[m_current_line - 2].push_back(static_cast<char>('\n'));
      data.buffer.push_back(ttemp);
    } else {
      data.buffer.insert(data.buffer.begin() + m_current_line - 2, ttemp);
    }
  } else if(!data.buffer[m_current_line - 2].empty()) {
    l_buffer p_line{data.buffer[m_current_line - 2].begin(), data.buffer[m_current_line -2].begin() + m_current_column};
    l_buffer n_line{data.buffer[m_current_line - 2].begin() + m_current_column, data.buffer[m_current_line - 2].end()};
    p_line.push_back(static_cast<char>('\n'));
    data.buffer[m_current_line - 2].clear();
    for(auto c : p_line) {
      data.buffer[m_current_line - 2].push_back(c);
    }
    if(m_current_line - 2 == static_cast<int>(data.buffer.size() -1)) {
      data.buffer.push_back(n_line);
    } else {
      data.buffer.insert(data.buffer.begin() + m_current_line, n_line);
    }
  }
}

void App::add_char(char c,Data& data) {
  if(!data.buffer.empty()) {
    if(!data.buffer[m_current_line-2].empty())
      data.buffer[m_current_line-2].insert(data.buffer[m_current_line-2].begin() + m_current_column, c);
    else
      data.buffer[m_current_line - 2].push_back(c);
  } else {
    l_buffer temp{c};
    data.buffer.push_back(temp);
  }
}

void App::del_char(Data& data) {
  if(static_cast<int>(data.buffer[m_current_line -2].size()) >= m_current_column && m_current_column >= 0) {
    data.buffer[m_current_line - 2].erase(data.buffer[m_current_line - 2].begin() + m_current_column - 1);
  }
}

void App::show_text(const w_buffer buffer, int start, int end) {
  for(int i{start}; i < end; ++i) {
    for(int j{0}; j < static_cast<int>(buffer[i].size()); ++j) {
      wprintw(panel_window(m_top_panel), "%c", buffer[i][j]);
    }
  }
}
