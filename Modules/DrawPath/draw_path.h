
#ifndef DRAWPATH_H
#define DRAWPATH_H

#include "Modules/Common/point.h"
#include "Input/iinput_state.h"
#include "Input/input_event.h"
#include "Modules/imodule.h"
#include "Graphics/image_engine.h"

#include <tbb/concurrent_queue.h>

class Drawing : public IModule
{
public:
    Drawing(ImageEngine* image_engine);

    void HandleEvent(std::shared_ptr<void*> data);
    virtual void UpdateTextures() override final;
private:
    void DrawLine(Point start, Point end) const;
    // ToDo: Point and Bezier Point struct
    std::shared_ptr<IStroke> m_stroke;
    size_t cur_idx;
    int* m_pixels;
    int m_pitch;
};

#endif
