
#include "Modules/Recognition/rec_worker.h"
#include <boost/signals2.hpp>

#include <iostream>

Recognition::Recognition(ImageEngine* image_engine, float threshold_angle, float threshold_distance) :
    IModule(image_engine),
    m_threshold_angle(threshold_angle),
    m_threshold_distance(threshold_distance)
{
    std::cout << "Recognition()" << std::endl;
        m_iengine->SubscribeEvent(INPUTEVENT::STROKE_START, boost::bind(&Recognition::HandleEvent, this, _1));
}

void Recognition::HandleEvent(std::shared_ptr<void *> data)
{
    std::cout << "HandleEvent" << std::endl;
    std::thread t(&Recognition::RecognizeStroke, this, std::reinterpret_pointer_cast<IStroke>(data));
    t.detach();
}

void Recognition::UpdateTextures()
{

}

void Recognition::RecognizeStroke(std::shared_ptr<IStroke> data)
{
    size_t current = 0;
    std::mutex mtx;
    while(!data->completed)
    {
//        std::cout << data->completed << std::endl;
        //std::unique_lock<std::mutex> lck(mtx);
        //while (data->stroke.size() > current)
        //    data->cv.wait(lck);
        //std::cout << "coord:" << std::endl;
        if (data->stroke.size() > current)
        {
            std::cout << "X: " << data->stroke[current].X << "Y: " << data->stroke[current].Y << std::endl;
            current++;
            //std::cout << "coord:" << std::endl;
        }
    }
}
