#pragma once
#include "oxygine-include.h"
#include "Event.h"
#include "EventDispatcher.h"
#include "SDL.h"
#include <string>

namespace oxygine
{
    DECLARE_SMART(InputText, spInputText);

    class InputText: public EventDispatcher
    {
    public:
        enum {EVENT_TEXT_CHANGED = sysEventID('I', 'T', 'C') };
        enum {EVENT_COMPLETE = Event::COMPLETE};

        InputText(int cursorSpeed = 400);
        ~InputText();

        /**Shows virtual keyboard(if supported on platform) and sends pressed chars to TextField*/
        void start(spTextField textActor);
        void stop();

        void setAllowedSymbols(const std::string& utf8str);
        void setAllowedSymbols(const std::wstring& str);
        void setDisallowedSymbols(const std::string& utf8str);
        void setDisallowedSymbols(const std::wstring& str);
        void setMaxTextLength(int);
        void setNumeric(bool isNumeric);
        void showCursor(bool show);

        static void stopAnyInput();

    protected:
        spTextField _textActor;
        spColorRectSprite _cursor;
        virtual void updateCursor();

    private:
        std::string _allowed;
        std::string _disallowed;
        std::string _txt;

        int _maxLength;
        int _isNumeric;
        void updateText();
        static InputText* _active;

        void _onSysEvent(Event* event);
        int _onSDLEvent(SDL_Event* event);
    };
}
