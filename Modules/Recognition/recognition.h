
#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"
#include "Modules/imodule.h"
#include "Graphics/image_engine.h"
#include "Modules/VectorGraphics/shapes.h"

#include <tbb/concurrent_queue.h>

class Recognition : public IModule
{
public:
    Recognition(ImageEngine* image_engine, float threshold_angle, float threshold_distance);

    void HandleEvent(std::shared_ptr<void*> data);
    void UpdateTextures() override final;
private:
    float m_threshold_angle;
    float m_threshold_distance;
    tbb::concurrent_queue < std::shared_ptr<Shape::Shape> > updateQueue;
};

#endif
