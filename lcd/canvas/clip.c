#include "../canvas.h"


void canvas_clip(Canvas* c, const CanvasRectangle *clip)
{
  if (!clip) c->clip.active = false;
  else {
    const int ax = clip->a.x;
    const int bx = clip->b.x;
    const int ay = clip->a.y;
    const int by = clip->b.y;
    if (ax > bx) {
      c->clip.rect.a.x = bx;
      c->clip.rect.b.x = ax;
    }
    else {
      c->clip.rect.a.x = ax;
      c->clip.rect.b.x = bx;
    }
    if (ay > by) {
      c->clip.rect.a.y = by;
      c->clip.rect.b.y = ay;
    }
    else {
      c->clip.rect.a.y = ay;
      c->clip.rect.b.y = by;
    }
    c->clip.active = true;
  }
}
