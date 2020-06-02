
#ifndef RECWORKER_H
#define RECWORKER_H

#include "Input/iinput_state.h"
#include "Input/input_event.h"
#include "Modules/imodule.h"
#include "Graphics/image_engine.h"

class Recognition : public IModule
{
public:
    Recognition(ImageEngine* image_engine, float threshold_angle, float threshold_distance);

    void HandleEvent(std::shared_ptr<void*> data);
    void UpdateTextures() final;
private:
    void RecognizeStroke(std::shared_ptr<IStroke> data);
    float m_threshold_angle;
    float m_threshold_distance;
};

#endif
