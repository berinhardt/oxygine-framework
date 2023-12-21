#include "EventDispatcher.h"

#include "Event.h"

#ifdef __MINGW32__
#include <malloc.h>
#endif  // ifdef __MINGW32__

// #define USE_ALLOCA

namespace oxygine {
EventDispatcher::EventDispatcher() : _lastID(0), _listeners(0) {}

EventDispatcher::~EventDispatcher() {
   __doCheck();
}

int EventDispatcher::addEventListener(eventType et, const EventCallback& cb) {
   __doCheck();

   _lastID++;

   /*
    #ifdef OX_DEBUG
      for (listeners::iterator i = _listeners->begin(); i != _listeners->end(); ++i)
      {
       const listener& ls = *i;
       if (ls.type == et && cb == ls.cb)
       {
           OX_ASSERT(!"you are already added this event listener");
       }
      }
    #endif
    */

   listener ls;
   ls.type = et;
   ls.cb = cb;
   ls.id = _lastID;
   _listeners.push_back(ls);

   return ls.id;
}

void EventDispatcher::removeEventListener(int id) {
   __doCheck();

   for (size_t size = _listeners.size(), i = 0; i != size; ++i) {
      const listener& ls = _listeners.at(i);

      if (ls.id == id) {
         _listeners.erase(_listeners.begin() + i);
         break;
      }
   }
}

void EventDispatcher::removeEventListener(eventType et, const EventCallback& cb) {
   __doCheck();

   // OX_ASSERT(_listeners);

   for (size_t size = _listeners.size(), i = 0; i != size; ++i) {
      const listener& ls = _listeners.at(i);

      if ((ls.type == et) && (cb == ls.cb)) {
         _listeners.erase(_listeners.begin() + i);
         break;

         // OX_ASSERT(hasEventListeners(et, cb) == false);
         // --i;
      }
   }
}

bool EventDispatcher::hasEventListeners(void* CallbackThis) {
   __doCheck();

   for (size_t size = _listeners.size(), i = 0; i != size; ++i) {
      const listener& ls = _listeners.at(i);

      if (ls.cb.p_this == CallbackThis) return true;
   }
   return false;
}

bool EventDispatcher::hasEventListeners(eventType et, const EventCallback& cb) {
   __doCheck();

   for (size_t size = _listeners.size(), i = 0; i != size; ++i) {
      const listener& ls = _listeners.at(i);

      if ((ls.type == et) && (cb == ls.cb)) return true;
   }
   return false;
}

void EventDispatcher::removeEventListeners(void* CallbackThis) {
   __doCheck();

   for (size_t i = 0; i < _listeners.size(); ++i) {
      const listener& ls = _listeners.at(i);

      if (ls.cb.p_this == CallbackThis) {
         _listeners.erase(_listeners.begin() + i);

         // OX_ASSERT(hasEventListeners(CallbackThis) == false);
         --i;
      }
   }
}

void EventDispatcher::removeEventListenersByType(eventType et) {
   __doCheck();

   for (size_t i = 0; i < _listeners.size(); ++i) {
      const listener& ls = _listeners.at(i);

      if (ls.type == et) {
         _listeners.erase(_listeners.begin() + i);

         // OX_ASSERT(hasEventListeners(CallbackThis) == false);
         --i;
      }
   }
}

void EventDispatcher::removeAllEventListeners() {
   _listeners.clear();
}

void EventDispatcher::dispatchEvent(Event* event) {
   if (!event->target && (this->_ref_counter > 0)) event->target = this;

   __doCheck();

   size_t size = _listeners.size();
   size_t num = 0;

   listenerbase* copy = new listenerbase[size];

   for (size_t i = 0; i != size; ++i) {
      listener& ls = _listeners.at(i);

      if (ls.type != event->type) continue;
      copy[num] = ls;
      ++num;
   }

   for (size_t i = 0; i != num; ++i) {
      listenerbase& ls = copy[i];

      if (this->_ref_counter > 0) event->currentTarget = this;
      event->listenerID = ls.id;
      try {
         ls.cb(event);
      } catch (const oxygine::event_exception& ex) {
         Event e(Event::ERROR);
         e.userData = (void*)&ex;
         dispatchEvent(&e);
      }
      if (event->stopsImmediatePropagation) break;
   }

   delete[] copy;
}

int EventDispatcher::getListenersCount() const {
   return (int)_listeners.size();
}
}  // namespace oxygine
