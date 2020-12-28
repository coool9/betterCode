//! @file
//! This is the main file of the project, containing the main event loop and renderer

#define SDL_MAIN_HANDLED //reformat code and fix highlighting bugs!!!
//create constexpr for columnwidth and columnheight
#include <SDL2/SDL.h>
#include <immer-0.6.2/immer/flex_vector.hpp>
#include <vector>
#include <string>
#include <unordered_map>

#include "datatypes.hpp"
#include "fileUtils.hpp"
#include "renderchars.hpp"
#include "menubar.hpp"

//Progress bar for saving, etc

namespace better { //TODO: highlighting

//! better::verticalNav scrolls the text vertically based on arrow keys. This is done incrementing or decrementing the variable topLine.
//! The topLine variable tells the render function to start rendering at that line in the text buffer.

better::Text verticalNav(better::Text text, SDL_Keycode key, const int textHeight, const int textWidth);

//! better::horizontalNav essentially scrolls the text horizontally based on arrow keys. This is done incrementing or decrementing the variable topColumn.
//! The topColumn variable tells the render function to start rendering at that position in each line of the text buffer.

better::Text horizontalNav(better::Text text, SDL_Keycode key, const int textHeight, const int textWidth);

//! better::shift "shifts" every key on the keyboard that has an alternate key.
//! This function is currently configured for ascii Us Windows keyboards in ISO configuration.

char shift(char key);

//! better::shiftLetter "shifts" characters for when caps lock is pressed.
//! Non letters are unaffected as caps lock only capitalises letters.

char shiftLetter(char key);

//! better::unshiftLetter "unshifts" characters while both the shift key and caps lock are pressed.
//! This function only unshifts letters.

char unshiftLetter(char key);

//! better::edit1 initialises a text editor the size of the window passed by its pointer.
//! It also reads in the file into its own text buffer, and contains the event loop for that text edit.

better::Text mouseMotion(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor);

better::Text keyDown(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor);

better::Text mouseButton(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor);

better::Text mouseButtonUp(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor);

better::Text mouseWheel(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor);

better::Text autoBracket(better::Text text, char bracket);

better::Text pasteClipboard(better::Text text);

better::Text deleteHighlighted(better::Text text);

better::Text handleKey(better::Text text, char key);

void edit1(SDL_Window* window, std::string filename, const int textHeight, const int textWidth, Uint32 colorbg, Uint32 colorfg, Uint32 colorhighlight, Uint32 colorparens, Uint32 colorcomments);

void copyClipboard(better::Text text);

better::Text cutClipboard(better::Text text);

better::Text ctrlShortcuts(better::Text text);

void resetMenus(bool* menus);

int selectMenu(bool menus[]);

void quitApp(SDL_Cursor* cursor, SDL_Surface* surface, SDL_Window* window);

bool operator!=(better::Text lhs, better::Text rhs);

}

int main(int argc, char* argv[]) {
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);

    std::string filename {better::fileDialog().string()};

    SDL_Window* window = SDL_CreateWindow("Better Code",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1200,960,0);

    const int textHeight {60};
    const int textWidth {150};
    Uint32 colorbg {0x222222FF};
    Uint32 colorfg {0x5588AAFF};
    Uint32 colorhighlight {0x444444FF};
    Uint32 colorparens {0xAA6969FF};
    Uint32 colorcomments {0x666666FF};
    
    better::edit1(window, filename, textHeight, textWidth, colorbg, colorfg, colorhighlight, colorparens, colorcomments);
    return 0;
}

bool better::operator!=(better::Text lhs, better::Text rhs) {
    return lhs.textEdit != rhs.textEdit;
}

better::Text better::pasteClipboard(better::Text text) {
    std::string clipboardText {};
    std::vector<better::Text> tempTexts {text};
    if(SDL_HasClipboardText()) {
        clipboardText = SDL_GetClipboardText();
        clipboardText.pop_back();
        clipboardText.pop_back();
        for(int i{}; i < clipboardText.size(); ++i) {
            if(clipboardText[i] == '\n') {
                tempTexts.push_back(better::newLine(tempTexts.back()));
            }
            else if(clipboardText[i] == '\r') {
                continue;
            }
            else {
                tempTexts.push_back(better::updateText(tempTexts.back(), clipboardText[i]));
            }
        }
        return tempTexts.back();
    }
    return text;
}

void better::copyClipboard(better::Text text) { //fix copying if dragging backwards to highlight
    std::string clipboardText {};
    for(int i{text.highlightStart.row}; i <= text.highlightEnd.row; ++i) {
        for(int j{}; j < text.textEdit[i].size(); ++j) {
            if(text.highlightStart.row == text.highlightEnd.row && j < text.highlightEnd.column && j >= text.highlightStart.column) {
                clipboardText.push_back(text.textEdit[i][j]);
            }
            else if(text.highlightStart.row == text.highlightEnd.row) {
                continue;
            }
            else if(i == text.highlightStart.row && j >= text.highlightStart.column) {
                clipboardText.push_back(text.textEdit[i][j]);
            }
            else if(i == text.highlightEnd.row && j < text.highlightEnd.column) {
                clipboardText.push_back(text.textEdit[i][j]);
            }
            else if(i > text.highlightStart.row && i < text.highlightEnd.row) {
                clipboardText.push_back(text.textEdit[i][j]);
            }
        }
        clipboardText.push_back('\n');
    }
    SDL_SetClipboardText(clipboardText.c_str());
}

better::Text better::deleteHighlighted(better::Text text) {
    std::vector<better::Text> tempTexts {text};
    tempTexts.back().cursor = text.highlightEnd;
    tempTexts.back().cursor.column += 1;
    int stopColumn {0};
    for(int i{tempTexts.back().cursor.row}; i >= tempTexts.back().highlightStart.row; --i) {
        if(i == tempTexts.back().highlightStart.row) {
            stopColumn = tempTexts.back().highlightStart.column + 1;
        }
        for(int j{tempTexts.back().cursor.column}; j >= stopColumn; --j) {
            tempTexts.push_back(better::backspace(tempTexts.back()));
        }
    }
    tempTexts.back().highlightEnd = tempTexts.back().highlightStart;
    return tempTexts.back();
}

better::Text better::cutClipboard(better::Text text) {
    better::copyClipboard(text);
    return better::deleteHighlighted(text);
} 

void better::edit1(SDL_Window* window, std::string filename, const int textHeight, const int textWidth, Uint32 colorbg, Uint32 colorfg, Uint32 colorhighlight, Uint32 colorparens, Uint32 colorcomments) {
    std::vector<better::Text> texts {};

    SDL_Cursor* guiCursor {SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM)};
    SDL_SetCursor(guiCursor);

    SDL_Rect screen {.x = 0, .y = 16, .w = 1200, .h = 960 - 16};

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    std::string menus {" File  Edit  View  Settings "};
    std::vector<std::vector<std::string>> menuText {{"Open","Save","Exit"},{"Cut","Copy","Paste"},{},{"Edit Settings"}};
    
    better::Text firstText {better::readFile(filename), {0,0}, {{false,false,false,false}, false, false, false, false, false, -1, {}, filename}, 0, 0, {0,0}, {0,0}};
    texts.push_back(firstText);
    
    SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, 0x22, 0x22, 0x22, 0xFF));
    better::renderText(surface, texts.back().textEdit, texts.back().topLineNumber, texts.back().topColumnNumber, textHeight, textWidth, {0,0}, {0,0}, colorbg, colorfg, colorhighlight, colorparens, colorcomments);
    better::renderCursor(surface, texts.back().cursor.column, texts.back().cursor.row, texts.back().topLineNumber, texts.back().topColumnNumber);
    better::drawMenuBar(surface, menus, 0xDDDDDDFF, 0x666666FF, 1200);

    SDL_UpdateWindowSurface(window);

    SDL_Event event;

    std::unordered_map<Uint32, better::Text(*)(better::Text, SDL_Event, SDL_Surface*, SDL_Cursor*)> handlers {
        {SDL_MOUSEMOTION, better::mouseMotion},
        {SDL_KEYDOWN, better::keyDown},
        {SDL_MOUSEBUTTONDOWN, better::mouseButton},
        {SDL_MOUSEWHEEL, better::mouseWheel},
        {SDL_MOUSEBUTTONUP, better::mouseButtonUp}
    };

    while(1) {
        if(SDL_WaitEvent(&event)) {
            if(texts.back().cursor.row == -1) {
                texts.back().data.isScroll = true;
            }
            const Uint8* state {SDL_GetKeyboardState(NULL)};
            if(state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
                texts.back().data.isShift = true;
            }
            if(!state[SDL_SCANCODE_LSHIFT] && !state[SDL_SCANCODE_RSHIFT]) {
                texts.back().data.isShift = false;
            }
            if(state[SDL_SCANCODE_RCTRL] || state[SDL_SCANCODE_LCTRL]) {
                texts.back().data.isCtrl = true;
            }
            if(!state[SDL_SCANCODE_RCTRL] && !state[SDL_SCANCODE_LCTRL]) {
                texts.back().data.isCtrl = false;
            }

            if(event.type == SDL_QUIT) {
                better::quitApp(guiCursor, surface, window);
                return;
            }

            if(event.type == SDL_KEYDOWN && texts.back().data.isCtrl && event.key.keysym.sym == 'z') {
                if(texts.size() > 1) {
                    texts.pop_back();
                    texts.shrink_to_fit();
                }
            }

            else if(event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEWHEEL || event.type == SDL_MOUSEBUTTONUP) {
                better::Text textTemp {texts.back().textEdit, {texts.back().cursor.row,texts.back().cursor.column}, {{texts.back().data.menusToDraw[0],texts.back().data.menusToDraw[1],texts.back().data.menusToDraw[2],texts.back().data.menusToDraw[3]}, texts.back().data.isShift, texts.back().data.isCaps, texts.back().data.isScroll, texts.back().data.isCtrl, texts.back().data.clearHistory, texts.back().data.index, texts.back().data.menu, texts.back().data.filename}, texts.back().topLineNumber, texts.back().topColumnNumber, texts.back().highlightStart, texts.back().highlightEnd};
                better::Text tempText {(handlers[event.type])(textTemp, event, surface, guiCursor)};
                if(tempText.data.clearHistory) {
                    texts.clear();
                    texts.push_back(tempText);
                    texts.back().data.clearHistory = false;
                }
                if(tempText != texts.back()) {
                    texts.push_back(tempText);
                }
                else {
                    texts.pop_back();
                    texts.push_back(tempText);
                }
            }
            else {
                continue;
            }

            SDL_FillRect(surface, &screen, SDL_MapRGBA(surface->format, 0x22, 0x22, 0x22, 0xFF));
            better::renderText(surface, texts.back().textEdit, texts.back().topLineNumber, texts.back().topColumnNumber, textHeight, textWidth, texts.back().highlightStart, texts.back().highlightEnd, colorbg, colorfg, colorhighlight, colorparens, colorcomments);
            if(!texts.back().data.isScroll) {
                better::renderCursor(surface, texts.back().cursor.column, texts.back().cursor.row, texts.back().topLineNumber, texts.back().topColumnNumber);
            }
            texts.back().data.index = better::selectMenu(texts.back().data.menusToDraw);
            if(texts.back().data.index < 0 || texts.back().data.index > 3) {
                texts.back().data.menu.clear();
            }
            else {
                texts.back().data.menu = menuText[texts.back().data.index];
            }

            better::drawMenus(surface, texts.back().data.menu, 0xDDDDDDFF, colorhighlight);

            SDL_UpdateWindowSurface(window);
        }
    }
}

better::Text better::mouseButtonUp(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor) {
    better::Cursor tempCursor = better::findCursorPos(text.topLineNumber, text.topColumnNumber, event);
    if(tempCursor.row == -1) {
        return text;
    }
    text.highlightEnd = tempCursor;
    if(text.highlightEnd.row < text.highlightStart.row || (text.highlightEnd.row == text.highlightStart.row && text.highlightEnd.column < text.highlightStart.column)) {
        better::Cursor tempCursor = text.highlightEnd;
        text.highlightEnd = text.highlightStart;
        text.highlightStart = tempCursor;
        if(text.textEdit[text.cursor.row].size() > text.highlightEnd.column) {
            text.cursor.column += 1;
        }
        else {
            text.cursor.row += 1;
            text.cursor.column = 0;
        }
    }
    return text;
}

better::Text better::autoBracket(better::Text text, char letter) {
    int num {letter == '(' ? 1 : 2};
    return better::horizontalNav(better::updateText(better::updateText(text,letter),letter + num),SDL_SCANCODE_LEFT, text.data.textHeight, text.data.textWidth);
}

better::Text better::mouseWheel(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor) {
    text.data.isScroll = true;
    return better::scroll(text, event, text.data.textHeight, text.data.textWidth);
}

better::Text better::keyDown(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor) {
    if(event.key.keysym.scancode == SDL_SCANCODE_DOWN || event.key.keysym.scancode == SDL_SCANCODE_UP) {
        text.highlightStart = text.highlightEnd;
        return better::verticalNav(text, event.key.keysym.scancode, text.data.textHeight, text.data.textWidth);
    }

    else if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT || event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
        text.highlightStart = text.highlightEnd;
        return better::horizontalNav(text, event.key.keysym.scancode, text.data.textHeight, text.data.textWidth);
    }
    else if(event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
        text.highlightStart = text.highlightEnd;
        std::vector<better::Text> texts {text}; //fix scrolling here for newline near bottom of text
        
        if((text.cursor.column - 1 > -1) && ((text.textEdit[text.cursor.row][text.cursor.column - 1] == '{' && text.textEdit[text.cursor.row][text.cursor.column] == '}') || (text.textEdit[text.cursor.row][text.cursor.column - 1] == '(' && text.textEdit[text.cursor.row][text.cursor.column] == ')'))) {
            if(texts.back().cursor.row != texts.back().textEdit.size() - 1 && texts.back().cursor.row >= texts.back().topLineNumber + better::textHeight() - 2) {
                texts.back().topLineNumber += 3;
            }
            texts.push_back(better::newLine(better::newLine(texts.back())));
            texts.push_back(better::verticalNav(texts.back(),SDL_SCANCODE_UP, better::textHeight(), better::textWidth()));
            texts.push_back(better::tab(texts.back()));
        }
        else {
            if(texts.back().cursor.row != texts.back().textEdit.size() - 1 && texts.back().cursor.row >= texts.back().topLineNumber + better::textHeight() - 2) {
                texts.back().topLineNumber += 1;
            }
            texts.push_back(better::newLine(texts.back()));
        }
        return texts.back();
    }

    else if(event.key.keysym.scancode == SDL_SCANCODE_TAB) {
        return better::tab(text);
    }

    else {
        SDL_Keycode keycode {event.key.keysym.scancode};
        if(keycode == SDL_SCANCODE_CAPSLOCK) {
            text.data.isCaps ? text.data.isCaps = false : text.data.isCaps = true;
            return text;
        }
        else if(keycode == SDL_SCANCODE_LSHIFT || keycode == SDL_SCANCODE_RSHIFT) {
            return text;
        }
        else if(keycode == SDL_SCANCODE_RCTRL || keycode == SDL_SCANCODE_LCTRL) {
            return text;
        }
        char key = {static_cast<char>(event.key.keysym.sym)};
        if(text.data.isShift) {
            key = better::shift(key);
            if(text.data.isCaps) {
                key = better::unshiftLetter(key);
            }
            text.data.isShift = false;
        }
        if(text.data.isCaps) {
            key = better::shiftLetter(key);
        }
        if(text.data.isScroll) { //change this
            text.topLineNumber = text.cursor.row;
            text.topColumnNumber = 0;
        }
        if(text.data.isCtrl) {
            std::string tempFilename {};
            switch(key) {
                case 's':
                    better::saveFile(text.textEdit, text.data.filename);
                    return text;
                case 'o':
                    better::saveFile(text.textEdit, text.data.filename);
                    tempFilename = better::fileDialog().string();
                    text.data.clearHistory = true;
                    text.textEdit = better::readFile(tempFilename);
                    text.cursor = {0,0};
                    text.topLineNumber = 0;
                    text.topColumnNumber = 0;
                    text.highlightStart = {0,0};
                    text.highlightEnd = {0,0};
                    text.data.index = -1;
                    text.data.isScroll = false;
                    text.data.isCaps = false;
                    text.data.isShift = false;
                    text.data.isCtrl = false;
                    text.data.menu.clear();
                    text.data.filename = tempFilename;
                    return text;
                case 'c':
                    better::copyClipboard(text);
                    return text;
                case 'v':
                    return better::pasteClipboard(text);
                case 'x':
                    return better::cutClipboard(text);
            }
            text.highlightEnd = text.highlightStart;
        }
        if(text.cursor.row == -1) {
            text.cursor.row = 0;
            better::resetMenus(text.data.menusToDraw);
        }
        if((text.highlightStart.row != text.highlightEnd.row) || (text.highlightEnd.column > text.highlightStart.column)) {
            return better::handleKey(better::deleteHighlighted(text),key);
        }
    text.highlightEnd = text.cursor;
    text.highlightStart = text.cursor;
    return better::handleKey(text, key);
        
    }
    return text;
}

better::Text better::handleKey(better::Text text, char key) {
    text.highlightStart = text.cursor;
    text.highlightEnd = text.cursor;
    switch(key) {
        case '\b':
            return better::backspace(text);
        case '\'':
            return better::horizontalNav(better::updateText(better::updateText(text,'\''),'\''),SDL_SCANCODE_LEFT, text.data.textHeight, text.data.textWidth);
        case '\"':
            return better::horizontalNav(better::updateText(better::updateText(text,key),'\"'),SDL_SCANCODE_LEFT, text.data.textHeight, text.data.textWidth);
        default:
            if(key == '(' || key == '{' || key == '[') {
                return better::autoBracket(text,key);
            }
            else {
                return better::updateText(text,key);
            }
    }
}

better::Text better::mouseButton(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor) {
    better::Cursor tempCursor {better::findCursorPos(text.topLineNumber, text.topColumnNumber, event)};
    if(tempCursor.row > text.textEdit.size() - 1) {
        tempCursor.row = text.textEdit.size() - 1;
        tempCursor.column = text.textEdit[text.textEdit.size() - 1].size();
    }
    if(tempCursor.column > text.textEdit[tempCursor.row].size()) {
        tempCursor.column = text.textEdit[tempCursor.row].size();
    }
    if((event.button.y / 16) - 1 == -1) {
        tempCursor.column = event.button.x / 8;
        tempCursor.row = -1;
        text.data.isScroll = true;
        better::resetMenus(text.data.menusToDraw);
        if(tempCursor.column < 6) {
            text.data.menusToDraw[0] = true;
        }
        else if(tempCursor.column > 5 && tempCursor.column < 12) {
            text.data.menusToDraw[1] = true;
        }
        else if(tempCursor.column > 11 && tempCursor.column < 18) {
            text.data.menusToDraw[2] = true;
        }
        else if(tempCursor.column > 17 && tempCursor.column < 28) {
            text.data.menusToDraw[3] = true;
        }
        tempCursor.column = 0;
    }

    else if((better::selectMenu(text.data.menusToDraw) > -1) && (((event.button.x / 8) > 20) || ((event.button.y / 16) > text.data.menu.size()))){
        better::resetMenus(text.data.menusToDraw);
    }

    int selectedMenu {better::selectMenu(text.data.menusToDraw)};
    int clickRow {event.button.y / 16};
    int clickColumn {event.button.x / 8};
    if(selectedMenu == 0) {
        if(clickRow == 2) {
            better::saveFile(text.textEdit, text.data.filename);
            better::resetMenus(text.data.menusToDraw);
            return text;
        }
        else if(clickRow == 1) {
            better::saveFile(text.textEdit, text.data.filename);
            std::string tempFilename = better::fileDialog().string();
            text.data.clearHistory = true;
            text.textEdit = better::readFile(tempFilename);
            text.cursor = {0,0};
            text.topLineNumber = 0;
            text.topColumnNumber = 0;
            text.highlightStart = {0,0};
            text.highlightEnd = {0,0};
            text.data.index = -1;
            text.data.isScroll = false;
            text.data.isCaps = false;
            text.data.isShift = false;
            text.data.isCtrl = false;
            text.data.menu.clear();
            text.data.filename = tempFilename;
            better::resetMenus(text.data.menusToDraw);
            return text;
        }
        else if(clickRow == 3) {
            better::resetMenus(text.data.menusToDraw);
            return text;
        }
    }
    else if(selectedMenu == 1) {
        if(clickRow == 1) {
            better::resetMenus(text.data.menusToDraw);
            return better::cutClipboard(text);
        }
        else if(clickRow == 2) {
            better::copyClipboard(text);
            better::resetMenus(text.data.menusToDraw);
            return text;
        }
        else if(clickRow == 3) {
            better::resetMenus(text.data.menusToDraw);
            return better::pasteClipboard(text);
        }
    }
    
    if(tempCursor.row != -1 ) {
        text.data.isScroll = false;
        text.cursor = tempCursor;
        text.highlightStart = tempCursor;
        text.highlightEnd = tempCursor;
    }
    return text;
}

better::Text better::mouseMotion(better::Text text, SDL_Event event, SDL_Surface* surface, SDL_Cursor* guiCursor) {
    if((event.motion.y / 16) == 0) {
        SDL_FreeCursor(guiCursor);
        guiCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    }
    else {
        guiCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    }
    SDL_SetCursor(guiCursor);
    return text;
}

void better::quitApp(SDL_Cursor* cursor, SDL_Surface* surface, SDL_Window* window) {
    SDL_FreeCursor(cursor);
    SDL_FreeSurface(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int better::selectMenu(bool menus[]) {
    for(int i{}; i < 4; ++i) {
        if(menus[i]) {
            return i;
        }
    }
    return -1;
}

void better::resetMenus(bool* menus) {
    for(int i{}; i < 4; ++i) {
        menus[i] = false;
    }
}

char better::shiftLetter(char key) {
    if(key >= 'a' && key <= 'z') {
        key -= 32;
    }
    return key;
}

char better::unshiftLetter(char key) {
    if(key >= 'A' && key <= 'Z') {
        key += 32;
    }
    return key;
}

char better::shift(char key) {
    std::unordered_map<char, char> shifted = {
        {32,32},{33,33},{34,34},{35,126},{36,36},{37,37},{38,38},{39,64},
        {40,40},{41,41},{42,42},{43,43},{44,60},{45,95},{46,62},{47,63},
        {48,41},{49,33},{50,34},{51,35},{52,36},{53,37},{54,94},{55,38},
        {56,42},{57,40},{58,58},{59,58},{60,60},{61,43},{62,62},{63,63},
        {64,64},{65,65},{66,66},{67,67},{68,68},{69,69},{70,70},{71,71},
        {72,72},{73,73},{74,74},{75,75},{76,76},{77,77},{78,78},{79,79},
        {80,80},{81,81},{82,82},{83,83},{84,84},{85,85},{86,86},{87,87},
        {88,88},{89,89},{90,90},{91,123},{92,124},{93,125},{94,94},{95,95},
        {96,96},{97,65},{98,66},{99,67},{100,68},{101,69},{102,70},{103,71},
        {104,72},{105,73},{106,74},{107,75},{108,76},{109,77},{110,78},{111,79},
        {112,80},{113,81},{114,82},{115,83},{116,84},{117,85},{118,86},{119,87},
        {120,88},{121,89},{122,90},{123,123},{124,124},{125,125},{126,126}
    };
    return shifted[key];
}

better::Text better::verticalNav(better::Text text, SDL_Keycode key, const int textHeight, const int textWidth) {
    better::Cursor cursor {text.cursor};
    switch(key) {
        case SDL_SCANCODE_DOWN:
            if(cursor.row < text.textEdit.size()) {
                if((cursor.row == text.textEdit.size() - 1) || (cursor.row == text.topLineNumber + textHeight - 1 && text.topLineNumber + textHeight - 1 >= text.textEdit.size() - 1)) {
                    return text;
                }
                if(text.textEdit[cursor.row + 1].size() < cursor.column) { //check the next row has less elements than current column of cursor position
                    cursor.column -= 1;
                    cursor.column = text.textEdit[cursor.row + 1].size();
                    cursor.row += 1;
                }
                else {
                    cursor.row += 1;
                }
                if(cursor.row == text.topLineNumber + textHeight - 1 && text.topLineNumber + textHeight - 1 < text.textEdit.size()) {
                    text.topLineNumber += 1; //scroll down
                }
            }
            break;

        case SDL_SCANCODE_UP:
            if(text.cursor.row) {
                if((cursor.row == text.topLineNumber) && cursor.row) {
                    text.topLineNumber -= 1;
                }
                if(text.textEdit[text.cursor.row - 1].size() < cursor.column) { //check the next row has less elements than current column of cursor position
                    cursor.column = text.textEdit[cursor.row - 1].size();
                    cursor.row -= 1;
                }
                else {
                    cursor.row -= 1;
                }
            }
            break;
    }
    text.cursor = cursor;
    return text;
}

better::Text better::horizontalNav(better::Text text, SDL_Keycode key, const int textHeight, const int textWidth) { //cases for if cursor is at top right/bottom left of screen
    better::Cursor cursor{text.cursor};
    switch(key) {
        case SDL_SCANCODE_RIGHT:
            if((cursor.row != text.textEdit.size() - 1) || (cursor.row == text.textEdit.size() - 1 && cursor.column < text.textEdit[cursor.row].size())) {
                if(cursor.column == text.textEdit[cursor.row].size()) {
                    cursor.column -= 1;
                    cursor.column = 0;
                    cursor.row += 1;
                    text.topColumnNumber = 0;
                }
                else {
                    cursor.column += 1;
                }
                if(cursor.column == text.topColumnNumber + textWidth) {
                    text.topColumnNumber += 1;
                }
            }
            break;

        case SDL_SCANCODE_LEFT:
            if((cursor.row) || (!cursor.row && cursor.column)) {
                if(!cursor.column) {
                    cursor.row -= 1;
                    cursor.column = text.textEdit[cursor.row].size();
                    if(cursor.column > textWidth - 2) {
                        text.topColumnNumber = (cursor.column - textWidth) + 1;
                    }
                }
                else {
                    cursor.column -= 1;
                }
                if((cursor.column == text.topColumnNumber) && (cursor.column)) {
                    text.topColumnNumber -= 1;
                }
            }
            break;
    }
    text.cursor = cursor;
    return text;
}
