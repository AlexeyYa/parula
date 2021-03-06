
#ifndef IMODULE_H
#define IMODULE_H

#include "Input/input_event.h"

#include <memory>

class ImageEngine;

struct IModule
{
public:
    IModule(ImageEngine* image_engine) :
        m_iengine(image_engine)
    {}
    virtual ~IModule() = default;

    virtual void UpdateTextures() = 0;

protected:
    ImageEngine* m_iengine;
    std::shared_ptr<void*> m_data;
};

#endif
