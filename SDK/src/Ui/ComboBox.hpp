#ifndef SRC_UI_COMBOBOX_HPP
#define SRC_UI_COMBOBOX_HPP

#include <windows.h>
#include <vector>
#include <string>
#include <commctrl.h>

namespace Ui
{
    class ComboBox
    {
    public:
        ComboBox(HWND parent, HINSTANCE instance)
            : hwndComboBox(NULL), hwndParent(parent), hInstance(instance) {}

        void create(const int x, const int y, const int width, const int height)
        {
            hwndComboBox = CreateWindowEx(
                0, WC_COMBOBOX, NULL,
                WS_VISIBLE | WS_CHILD | CBS_DROPDOWN | WS_VSCROLL,
                x, y, width, height,
                hwndParent, NULL, hInstance, NULL);
        }

        void addItem(LPCWSTR item)
        {
            if (hwndComboBox)
            {
                SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)item);
            }
        }

        int getSelectedIndex() const
        {
            if (hwndComboBox)
            {
                return SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0);
            }
            return -1; 
        }

        LPCWSTR getSelectedItem() const
        {
            if (hwndComboBox)
            {
                int index = getSelectedIndex();
                if (index != -1)
                {
                    wchar_t buffer[256];
                    SendMessage(hwndComboBox, CB_GETLBTEXT, index, (LPARAM)buffer);
                    return buffer;
                }
            }
            return L"";
        }

        HWND getHandle() const
        {
            return hwndComboBox;
        }
    private:
        HWND hwndComboBox;   
        HWND hwndParent;     
        HINSTANCE hInstance; 

    };
};
#endif