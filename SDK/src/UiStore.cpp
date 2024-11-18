#include "UiStore.hpp"

std::vector<HWND> Editor::UiStore::store(6);
RECT Editor::UiStore::parentRect = {};
int Editor::UiStore::segmentHeight = 0;
int Editor::UiStore::segmentWidth = 0;
int Editor::UiStore::cols = 3;
int Editor::UiStore::rows = 2;
