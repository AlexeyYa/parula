
#ifndef DRAWPATH_H
#define DRAWPATH_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"
#include "Modules/imodule.h"
#include "Graphics/image_engine.h"

#include <tbb/concurrent_queue.h>

class Point
{

};

class Drawing : public IModule
{
public:
    Drawing(ImageEngine* image_engine);

    void HandleEvent(std::shared_ptr<void*> data);
    void UpdateTextures() final;
private:
    void DrawLine() const;
    // ToDo: Point and Bezier Point struct
    std::shared_ptr<IStroke> m_stroke;
    size_t cur_idx;
    void* m_pixels;
};

#endif
