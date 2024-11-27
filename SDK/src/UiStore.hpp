#ifndef SRC_UISTORE_HPP
#define SRC_UISTORE_HPP

#include <Windows.h>
#include <vector>

namespace Editor
{
    class UiStore
    {
    public:
        static void init(const RECT &rect, const int _cols, const int _rows)
        {
            
        }

        static void update(const RECT &rect)
        {
            parentRect = rect;
            segmentWidth = rect.right / cols;
            segmentHeight = rect.bottom / rows;

            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    SetWindowPos(store[(3 * i) + j], HWND_TOP, j * segmentWidth, i * segmentHeight, segmentWidth, segmentHeight, 0);
                }
            }
        }

        static void add(const HWND &handle, const int row, const int col)
        {
            store.push_back(handle);
        }

        static void move(const int index, const int width, const int height)
        {
           
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
