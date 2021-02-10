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
          wmove(panel_window(m_top_panel), 2,0);
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
          for(auto v : temp_data->buffer) {
            for(auto c : v) {
              myfile << c;
            }
            myfile << '\n';
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
