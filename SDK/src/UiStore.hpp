#ifndef SRC_UISTORE_HPP
#define SRC_UISTORE_HPP

#include <Windows.h>
#include <vector>

namespace Editor
{
  class UiStore
{
public:


    static void init(const RECT& rect, const int _cols, const int _rows)
    {
        parentRect = rect;
        rows = _rows;
        cols = _cols;

        segmentWidth = rect.right / cols;
        segmentHeight = rect.bottom / rows;
    }

    static void update(const RECT& rect)
    {
        parentRect = rect;
        segmentWidth = rect.right / cols;
        segmentHeight = rect.bottom / rows;

        for (int i = 0; i < rows; i++)
        {
            for(int j = 0; j < cols; j++)
            {
                SetWindowPos(store[(3 * i ) + j], HWND_TOP, j * segmentWidth, i * segmentHeight, segmentWidth, segmentHeight, 0);
            }
        }
    }

    static void add(const HWND &handle, const int row, const int col)
    {
        store[(3 * row ) + col] = handle;
        SetWindowPos(handle, HWND_TOP, col * segmentWidth, row * segmentHeight, segmentWidth, segmentHeight, 0);
        ShowWindow(handle, 0);
        UpdateWindow(handle);        
    }

    static void move(const int index, const int width, const int height)
    {
        // if (index < 0 || index >= store.size()) 
        //     return;

        // RECT rect;
        // GetClientRect(store[index], &rect);
    
        // if(rect.right >)

    }
    

private:
    static std::vector<HWND> store;
    static RECT parentRect;

    static int segmentWidth;
    static int segmentHeight;

    static int cols;
    static int rows;
};



};

#endif


