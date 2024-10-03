#include "Stage.h"

#include <sstream>

#include "../Clock.h"
#include "../RenderState.h"
#include "../STDRenderDelegate.h"
#include "../core/oxygine.h"
#include "../math/Rect.h"

#ifdef OXYGINE_SDL
#include "SDL.h"
#endif

namespace oxygine {
spStage Stage::instance;

Stage::Stage(bool autoReset) : _statUpdate(0), _clipOuter(false), _viewport(0, 0, 0, 0)  //, _active(true)
{
   spClock clock = new Clock();
   setClock(clock);
   setName("Stage");

   // each mobile application should handle focus lost
   // and free/restore GPU resources
   if (autoReset) {
      addEventListener(Stage::DEACTIVATE, CLOSURE(this, &Stage::onDeactivate));
      addEventListener(Stage::ACTIVATE, CLOSURE(this, &Stage::onActivate));
   }
   _stage = this;

#ifdef OXYGINE_SDL
   _window = 0;
#endif
}

#if OXYGINE_SDL
void Stage::associateWithWindow(SDL_Window* wnd) {
   _window = wnd;
   SDL_SetWindowData(wnd, "_", this);
   addRef();
}

SDL_Window* Stage::getAssociatedWindow() const {
   if (_window)
      return _window;
   return core::getWindow();
}

Stage* Stage::getStageFromWindow(SDL_Window* wnd) {
   void* data = SDL_GetWindowData(wnd, "_");
   return (Stage*)data;
}

#endif

Stage::~Stage() {
}

void Stage::onDeactivate(Event*) {
   //_active = false;
   core::reset();
}

void Stage::onActivate(Event*) {
   core::restore();
   //_active = true;
}

std::string Stage::dump(const dumpOptions& opt) const {
   std::stringstream st;

   st << "{Stage}\n";
   // st << " displaySize=(" << _realDisplaySize.x << "," << _realDisplaySize.y << ")";
   st << Actor::dump(opt);

   return st.str();
}

Rect Stage::calcCenteredViewport(const Point& displaySize, const Point& gameSize) {
   float width = (float)displaySize.x;
   float height = (float)displaySize.y;

   float scaleFactorX = width / gameSize.x;
   float scaleFactorY = height / gameSize.y;

   float scaleFactor = scaleFactorX < scaleFactorY ? scaleFactorX : scaleFactorY;
   Vector2 size = Vector2(gameSize) * scaleFactor;

   Vector2 free = Vector2(displaySize) - size;

   Rect r((free / 2.0f), size);
   logs::messageln("VP %f, %f [%f x %f]", r.pos.x, r.pos.y, r.size.x, r.size.y);

   return r;
}

void Stage::init(const Point& displaySize, const Point& gameSize) {
   setSize(gameSize);

   _viewport = calcCenteredViewport(displaySize, gameSize);
   float scaleFactor = _viewport.size.x / (float)gameSize.x;

   setScale(scaleFactor);
   setPosition(_viewport.pos);
}

bool Stage::isOn(const Vector2& localPosition, float localScale) {
   return true;
}

RectF Stage::getDestRect() const {
   //        OX_ASSERT(0);
   Vector2 s = getSize() + getPosition();
   return RectF(-getPosition(), s);
}

/*
bool Stage::handleEvent(const EventState &es)
{
    bool handled = Actor::handleEvent(es);
    return handled;
}
*/
void Stage::render(const Color* clearColor, const Rect& viewport, const Matrix4& view, const Matrix4& proj) {
   if (!STDRenderer::instance || !IVideoDriver::instance || !STDRenderer::getCurrent()) return;
   IVideoDriver* driver = IVideoDriver::instance;
   driver->setViewport(viewport);

   if (clearColor)
      driver->clear(*clearColor);

   Matrix4 vp = view * proj;
   STDRenderer::instance->setViewProj(vp);

   RenderState rs;
   Point ds = core::getDisplaySize();

   RectF clip(0.0f, 0.0f, (float)ds.x, (float)ds.y);
   rs.clip = &clip;

   if (_clipOuter) {
      driver->setScissorRect(&_viewport);
      clip = _viewport.cast<RectF>();
   }
   RenderState nrs;
   Actor::render(rs, nrs);
   Actor::postRender(nrs);
   STDRenderer::getCurrent()->flush();

   if (_clipOuter) {
      driver->setScissorRect(0);
   }

   Material::null->apply();
}

void Stage::render(const Color& clearColor, const Rect& viewport) {
   // initialize projection and view matrix
   Matrix4 proj = glm::orthoLH(0.0f, (float)viewport.getWidth(), (float)viewport.getHeight(), 0.0f, 0.0f, 100.0f);

   render(&clearColor, viewport, Matrix4(1.0f), proj);
}

void Stage::cleanup() {
}

void Stage::update() {
   timeMS t = getTimeMS();
   UpdateState us;
   Actor::update(us);

   _statUpdate = getTimeMS() - t;
}
}  // namespace oxygine
