//! @file

#ifndef DATATYPES_HPP
#define DATATYPES_HPP

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <immer-0.6.2/immer/flex_vector.hpp>

namespace better {

constexpr int textWidth() {
    return 150;
}

constexpr int textHeight() {
    return 60;
}

struct Cursor { //store the line number and column number of the cursor
    int row;
    int column;
};

better::Cursor findCursorPos(int topLine, int topColumn, SDL_Event event);

struct editorData {
    bool menusToDraw[4] = {false, false, false, false};
    bool isShift {false};
    bool isCaps {false};
    bool isScroll {false};
    bool isCtrl {false};
    bool clearHistory {false};
    int index {-1};
    std::vector<std::string> menu;
    std::string filename;
    const int textHeight {60};
    const int textWidth {150};
};

struct Text {
    immer::flex_vector<immer::flex_vector<char>> textEdit; //keeps the text state
    better::Cursor cursor; //keeps the cursor state
    better::editorData data;
    int topLineNumber; //keeps top line number so can calculate new line being clicked on
    int topColumnNumber;
    better::Cursor highlightStart {0,0};
    better::Cursor highlightEnd {0,0};
};

struct charMapArr {
    ::Uint8 arr[16];
};

better::charMapArr makeCharMapArr(::Uint8 charArray[16]); 

better::Text updateText(better::Text textEdit, char newChar);
better::Text backspace(better::Text textEdit);
better::Text newLine(better::Text textEdit);
better::Text tab(better::Text text);

Uint8 getRed(Uint32 color);
Uint8 getGreen(Uint32 color);
Uint8 getBlue(Uint32 color);
Uint8 getAlpha(Uint32 color);

int getPreviousIndentLevel(better::Text text, int row);

}

#endif
