#pragma once
#include <map>

#include "test.h"

DECLARE_SMART(Camera, spCamera);
class Camera : public Actor {
  public:
   spActor _content;

   Camera() : _transform(1.0f) {
      addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &Camera::onEvent));
      addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &Camera::onEvent));
      addEventListener(TouchEvent::MOVE, CLOSURE(this, &Camera::onEvent));
      addEventListener(TouchEvent::WHEEL_DIR, CLOSURE(this, &Camera::onEvent));
   }

   void setContent(spActor content) {
      _content = content;
      addChild(content);
   }

   struct touch {
      Vector2 current(0, 0);
      Vector2 previous(0, 0);
   };

   std::map<int, touch> _touches;

   void onEvent(Event* ev) {
      TouchEvent* te = safeCast<TouchEvent*>(ev);
      const Vector2& pos = te->localPosition;

      if (te->type == TouchEvent::TOUCH_DOWN) {
         touch& t = _touches[te->index];
         t.previous = t.current = pos;
      }

      if (te->type == TouchEvent::TOUCH_UP) {
         auto it = _touches.find(te->index);
         if (it != _touches.end())
            _touches.erase(it);
      }

      if (te->type == TouchEvent::WHEEL_DIR) {
         if (te->wheelDirection.y != 0.0f) {
            float scale = te->wheelDirection.y < 0 ? 0.95f : 1.05f;

            _transform = glm::translate(_transform, -Vector3(pos, .0f));
            _transform = glm::scale(_transform, Vector3(scale, scale, .0f));
            _transform = glm::translate(_transform, +Vector3(pos, .0f));
         }
      }

      if (te->type == TouchEvent::MOVE && !_touches.empty()) {
         touch& t = _touches[te->index];
         t.previous = t.current;

         t.current = pos;

         if (_touches.size() == 1) {
            Vector2 offset = t.current - t.previous;
            _transform = glm::translate(_transform, Vector3(offset, .0f));
         } else {
            touch *p1, *p2;
            for (std::map<int, touch>::iterator i = _touches.begin(); i != _touches.end(); ++i) {
               if (i->first == te->index)
                  p1 = &i->second;
               else
                  p2 = &i->second;
            }

            Vector2 center = (p1->current + p2->current) / 2.0f;
            Vector2 prevCenter = (p1->previous + p2->previous) / 2.0f;

            float dist = glm::distance(p1->current, p2->current);
            float prevDist = glm::distance(p1->previous, p2->previous);
            float scale = dist / prevDist;

            Vector2 offset = center - prevCenter;

            p1->previous = p1->current;
            p2->previous = p2->current;

            _transform = glm::translate(_transform, Vector3(offset, .0f));

            _transform = glm::translate(_transform, -Vector3(center, .0f));
            _transform = glm::scale(_transform, Vector3(scale, scale, 1.0f));
            _transform = glm::translate(_transform, Vector3(center, .0f));
         }
      }

      update();
   }

   void doUpdate(const UpdateState& us) {
   }

   void update() {
      /*Transform tr(_transform);
      _content->setTransform(tr);*/
   }

   Matrix4 _transform;
};

class TestCamera : public Test {
  public:
   TestCamera() {
      spCamera cam = new Camera;
      cam->attachTo(_content);
      cam->setSize(_content->getSize());

      spSprite map = new Sprite();
      map->setResAnim(resources.getResAnim("map"));
      cam->setContent(map);

      spButton button = new Button;

      button->setPosition(map->getSize() / 2.0f);
      button->setResAnim(Test::_resources.getResAnim("button"));
      button->attachTo(map);
      button->setAnchor(0.5f, 0.5f);
      button->addEventListener(TouchEvent::CLICK, CLOSURE(this, &TestCamera::testClick));
   }

   void testClick(Event* event) {
      notify("clicked");
   }
};
