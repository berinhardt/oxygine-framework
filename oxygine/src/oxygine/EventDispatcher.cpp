#include "EventDispatcher.h"

#include "Event.h"

#ifdef __MINGW32__
#include <malloc.h>
#endif  // ifdef __MINGW32__

// #define USE_ALLOCA

namespace oxygine {
EventDispatcher::EventDispatcher() : _lastID(0) {}

EventDispatcher::~EventDispatcher() {
   __doCheck();
}

int EventDispatcher::addEventListener(eventType et, const EventCallback& cb) {
   __doCheck();

   _lastID++;

   listener ls;
   ls.type = et;
   ls.cb = cb;
   ls.id = _lastID;
   _listeners.push_back(ls);

   return ls.id;
}

void EventDispatcher::removeEventListener(int id) {
   __doCheck();

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if (ls.id == id) {
         _listeners.erase(it);
         break;
      }
   }
}

void EventDispatcher::removeEventListener(eventType et, const EventCallback& cb) {
   __doCheck();

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if ((ls.type == et) && (cb == ls.cb)) {
         _listeners.erase(it);
         break;
      }
   }
}

bool EventDispatcher::hasEventListeners(void* CallbackThis) {
   __doCheck();
   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if (ls.cb.p_this == CallbackThis) return true;
   }
   return false;
}

bool EventDispatcher::hasEventListeners(eventType et, const EventCallback& cb) {
   __doCheck();

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if ((ls.type == et) && (cb == ls.cb)) return true;
   }
   return false;
}
const EventCallback* EventDispatcher::getListenerByID(int index) const {
   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;
      if (ls.id == index) return &ls.cb;
   }
   return NULL;
}
void EventDispatcher::removeEventListeners(void* CallbackThis) {
   __doCheck();

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if (ls.cb.p_this == CallbackThis) {
         _listeners.erase(it);
      }
   }
}

void EventDispatcher::removeEventListenersByType(eventType et) {
   __doCheck();

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      const listener& ls = *it;

      if (ls.type == et) {
         _listeners.erase(it);
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
   if (size == 0) return;

   size_t num = 0;

   listenerbase* copy = new listenerbase[size];

   for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
      listener& ls = *it;

      if (ls.type != event->type) continue;
      copy[num] = ls;
      ++num;
   }

   for (size_t i = 0; i != num; ++i) {
      listenerbase& ls = copy[i];

      if (this->_ref_counter > 0) event->currentTarget = this;
      event->listenerID = ls.id;
      ls.cb(event);

      if (event->stopsImmediatePropagation) break;
   }

   delete[] copy;
}

int EventDispatcher::getListenersCount() const {
   return (int)_listeners.size();
}
}  // namespace oxygine
uint32_t detail::Closure0::NEXT_FINGERPRINT = 0;
uint32_t detail::Closure1::NEXT_FINGERPRINT = 0;
uint32_t detail::Closure2::NEXT_FINGERPRINT = 0;
uint32_t detail::Closure3::NEXT_FINGERPRINT = 0;
uint32_t detail::Closure4::NEXT_FINGERPRINT = 0;