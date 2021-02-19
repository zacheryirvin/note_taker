#include "ncurses.h"
#include "panel.h"
#include "menu.h"
#include <fstream>

#include "app.h"

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
      if( const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch ) {
        m_top_panel = const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch;
        const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch = temp_pan;
      }
      top_panel(m_top_panel);
      break;
    }
    case 10: {
      switch(index) {
        case 0: {
          Data* menu_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))};
          std::string new_buffer_title{open_file_box("Open File", menu_data->selection_box)};
          if(m_open_windows < m_num_wins) {
            ++m_open_windows;
            int available_slot{find_open_window()};
            mvwprintw(panel_window(m_pans[0]), LINES - 2, 2, "%d", available_slot);
            Data* new_pan_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[available_slot])))};
            Data* prev_pan_data;
            new_pan_data->next = m_start_panel;
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
              prev_pan_data->next = m_end_panel;
            }
            print_title(m_end_panel);
            m_top_panel = m_end_panel;
            const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = m_top_panel; 
            Data* buffer_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))};
            std::string current_line;
            std::ifstream my_file{"/home/zac/projects/ncurses/note_taker/build/notes/" + new_buffer_title};
            if(my_file.is_open()) {
              int line_count {0};
              while(std::getline(my_file, current_line)) {
                std::vector<char> new_vec{};
                buffer_data->buffer.push_back(new_vec);
                for(const auto c : current_line) {
                  buffer_data->buffer[line_count].push_back(c) ;
                }
                buffer_data->buffer[line_count].push_back(static_cast<char>('\n'));
                ++line_count;
              }
              my_file.close();
            } else {
              mvwprintw(panel_window(m_top_panel), LINES-1, 3, "ERROR");
            }
            wmove(panel_window(m_top_panel), 2,0);
            for(const auto v : buffer_data->buffer) {
              for(const auto c : v) {
                waddch(panel_window(m_top_panel), c);
              }
            }
          }
          wmove(panel_window(m_top_panel), 2,0);
          top_panel(m_top_panel);
          keypad(panel_window(m_top_panel), true);
          break;
        }
        case 1: {
          Data* menu_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))};
          std::string new_buffer_title{create_option_box("New File", menu_data->selection_box)};
          if(m_open_windows < m_num_wins) {
            ++m_open_windows;
            int available_slot{find_open_window()};
            Data* new_pan_data{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[available_slot])))};
            Data* prev_pan_data;
            new_pan_data->next = m_start_panel;
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
              prev_pan_data->next = m_end_panel;
            }
            print_title(m_end_panel);
            m_top_panel = m_end_panel;
            const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = m_top_panel;
            top_panel(m_top_panel);
            keypad(panel_window(m_top_panel), true);
          }
          wmove(panel_window(m_top_panel), 2,0);
          break;
        }
        case 2: {
          PANEL* switch_panel{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_top_panel)))->menu_switch};
          Data* temp{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(switch_panel)))};
          wclear(panel_window(switch_panel));
          temp->buffer.clear();
          temp->file_name = "";
          temp->open = true;
          temp->w_current_col = 0;
          temp->w_current_line = 2;
          temp->end = false;
          if(m_open_windows == 3) {
            m_top_panel = m_pans[0];
            m_start_panel = m_pans[1];
            m_end_panel = m_pans[1];
            const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))->menu_switch = nullptr;
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
            Data* menu_panel{const_cast<Data*>(reinterpret_cast<const Data*>(panel_userptr(m_pans[0])))};
            if(temp->prev != menu_panel->menu_switch) {
              menu_panel->menu_switch = temp->prev;
              // m_top_panel = temp->prev;
              top_panel(temp->prev);
              update_panels();
              doupdate();
              top_panel(m_top_panel);
            } else if(temp->next != menu_panel->menu_switch) {
              menu_panel->menu_switch = temp->next;
              // m_top_panel = temp->next;
              top_panel(temp->next);
              update_panels();
              doupdate();
              top_panel(m_top_panel);
            }
          }
          temp->next = nullptr;
          temp->prev = nullptr;
          --m_open_windows;
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
          myfile.open("/home/zac/projects/ncurses/note_taker/build/notes/" + temp_data->file_name);
          for(auto v : temp_data->buffer) {
            for(auto c : v) {
              myfile << c;
            }
            // myfile << '\n';
          }
          // myfile << temp_data->buffer << '\n';
          myfile.close();
          break;
        }
        break;
      }
    }
  }
}
