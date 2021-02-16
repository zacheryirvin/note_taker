#include "ncurses.h"
#include "panel.h"
#include "menu.h"

#include "app.h"

void App::notes_window_loop(const int ch) {
  PANEL* temp{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch};
  Data* panel_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))};
  w_buffer current_buffer{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer};
  int y, x;
  int begy, begx;
  int maxy, maxx;
  int start{0};
  int end{m_height};
  int doc_size{static_cast<int>(current_buffer.size())};
  getyx(panel_window(m_top_panel), y, x);
  getbegyx(panel_window(m_top_panel), begy, begx);
  getmaxyx(panel_window(m_top_panel), maxy, maxx);
  keypad(panel_window(m_top_panel), true);
  if(panel_data->w_current_line != m_current_line || panel_data->w_current_col != m_current_column) {
    m_current_line = panel_data->w_current_line;
    m_current_column = panel_data->w_current_col;
    wmove(panel_window(m_top_panel), m_current_line, m_current_column);
  }
  switch(ch) {
    case KEY_UP: {
      if(y > begy + 1 && y < end) {
        wmove(panel_window(m_top_panel), y - 1, x);
        --m_current_line;
      } else if(y == begy + 1 && m_current_line > 2) {
        show_text(current_buffer, start, end);
        --start;
        --end;
        --m_current_line;
      }
      panel_data->w_current_line = m_current_line;
      break;
    }
    case KEY_DOWN: {
      if(y < maxy - 1 && y <= doc_size) {
        wmove(panel_window(m_top_panel), y + 1, x);
        ++m_current_line;
        mvwprintw(panel_window(temp), LINES - 1, 2, "%d, %d", m_current_line, current_buffer.size());
      } else if(y == maxy - 1) {
        show_text(current_buffer, start, end);
        ++start;
        ++end;
        ++m_current_line;
      }
      panel_data->w_current_line = m_current_line;
      break;
    }
    case KEY_LEFT: {
      if(x > ((begx - 20) + 1)) {
        wmove(panel_window(m_top_panel), y, x - 1);
        --m_current_column;
      }
      panel_data->w_current_col = m_current_column;
      break;
    }
    case KEY_RIGHT: {
      if(x < maxx - 1 && x < static_cast<int>(current_buffer[m_current_line - 2].size())) {
        wmove(panel_window(m_top_panel), y, x + 1);
        ++m_current_column;
      }
      panel_data->w_current_col = m_current_column;
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
      temp->buffer.clear();
      temp->file_name = "";
      temp->open = true;
      // temp->next = nullptr;
      // temp->prev = nullptr;
      if(m_open_windows == 3) {
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = nullptr;
        m_top_panel = m_pans[0];
        top_panel(m_top_panel);
      } else {
        PANEL* previous{temp->prev};
        PANEL* next{temp->next};
        if(m_top_panel == m_start_panel) {
          const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(next)))->start = true;
          temp->start = false;
          m_start_panel = next;
        }
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(previous)))->next = temp->next;
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(next)))->prev = temp->prev;
        if(temp->prev != m_top_panel) {
          m_top_panel = temp->prev;
          // wrefresh(panel_window(m_top_panel));
          top_panel(m_top_panel);
        } else if(temp->next != m_top_panel) {
          m_top_panel = temp->next;
          // wrefresh(panel_window(m_top_panel));
          top_panel(m_top_panel);
        }
      }
      --m_open_windows;
      break;
    }
    case 10: {
      if(m_current_line-2 == static_cast<int>(current_buffer.size() - 1) || (m_current_line - 2 == 0 && static_cast<int>(current_buffer.size()) == 0)) {
        waddch(panel_window(m_top_panel), ch);
        l_buffer temp{};
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer.push_back(temp);
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->buffer.back().push_back('\n');
        ++m_current_line;
        m_current_column = 0;
      }
      panel_data->w_current_line = m_current_line;
      panel_data->w_current_col = m_current_column;
      break;
    }
   case KEY_BACKSPACE: {
      if(m_current_column > 0) {
        Data* data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))};
        mvwdelch(panel_window(m_top_panel), y, x - 1);
        del_char(*data);
        --m_current_column;
      }
      panel_data->w_current_col = m_current_column;
      break;
    } 
    default: {
      // waddch(panel_window(m_top_panel), ch);
      Data* data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))};
      add_char(static_cast<char>(ch), *data);
      if(m_current_column != static_cast<int>(data->buffer[m_current_line-2].size())-1) {
        std::string temp_buff{""};
        for(int i{m_current_column}; i < static_cast<int>(data->buffer[m_current_line -2].size()); ++i) {
          temp_buff += data->buffer[m_current_line-2][i];
        }
        wclrtoeol(panel_window(m_top_panel));
        mvwprintw(panel_window(m_top_panel), m_current_line, m_current_column, temp_buff.c_str());
        wmove(panel_window(m_top_panel), m_current_line, m_current_column + 1);
      } else
        // mvwprintw(panel_window(m_top_panel), m_current_line, m_current_column, "%c", ch);
        waddch(panel_window(m_top_panel), ch);
      ++m_current_column;
      panel_data->w_current_col = m_current_column;
    }
    mvwprintw(panel_window(temp), LINES - 1, 2, "%d", ch);
  }
}
