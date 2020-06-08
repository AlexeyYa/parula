
#ifndef RECWORKER_H
#define RECWORKER_H


#include <iostream>


#include "Modules/Recognition/recognition.h"
#include "Modules/Recognition/prediction.h"
#include "Modules/VectorGraphics/shapes.h"

#include <memory>
#include <thread>

class RecWorker : public std::enable_shared_from_this<RecWorker>
{
public:
    RecWorker(Recognition* rec_module, std::shared_ptr<IStroke> data, float threshold_distance, float threshold_angle) :
        m_rec_module(rec_module),
        m_data(data),
        m_threshold_distance(threshold_distance),
        m_threshold_angle(threshold_angle)
    {}

    void RecognizeStroke()
    {
        auto self(shared_from_this());

        std::thread t(&RecWorker::WorkerThread, this, self);
        t.detach();
    }

private:
    Recognition* m_rec_module;
    std::shared_ptr<IStroke> m_data;
    float m_threshold_distance;
    float m_threshold_angle;
    RPrediction m_prediction;


    void WorkerThread(std::shared_ptr<RecWorker>)
    {
        size_t current = 0;
        std::mutex mtx;
        while(!m_data->completed)
        {
            std::unique_lock<std::mutex> lck(mtx);

            if (m_data->stroke.size() == current + 1)
                m_data->cv.wait(lck, [&]{ return true; });

            // Save size to avoid race conditions
            size_t size = m_data->stroke.size();

            // Check if fits current prediction
            switch (m_prediction.active) {
            case RPrediction::Line:
                if (CheckLinePoints(*std::static_pointer_cast<Line>(m_prediction.shape),
                                    current,
                                    size))
                {
                    current = size - 1;
                    continue;
                }
                break;
            default:

                break;
            }

            // Line   A*x + B*y = C
            // data->stroke.front()  +  End
            // Line at leats 2*threshold px
            PredictLine(0, size);

            // Curve
            // data->stroke.front() + P1 + P2 + data->stroke.back()
            // find P1, P2 (4 unknowns)
            // (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3

            // If not fits try Ellipse
            // 5 unknowns

            // Polyline/polycurve Check angles(~15 deg)
            current = size - 1;

        }
        if (m_prediction.active == RPrediction::Line)
        {
            std::cout << "Line through ";
            for (size_t i = 0; i < m_data->stroke.size(); i += m_data->stroke.size() / 5)
            {
                std::cout << "P" << i << "(" << m_data->stroke[i].X << ";" << m_data->stroke[i].Y << ")" << std::endl;
            }
        }
        std::cout << "End of stroke rec " << current << " " << m_data->stroke.size() << std::endl;
        // Ellipse  Check ends(closed/open)

        // Polyline Check ends(closed/open), angles and sides

        // Polycurve Check ends(closed/open)

        // Save for next texture update
    }

    /*!
     * \brief CheckLinePoints checks if all points are within line distance on indecies [first, last)
     * \param first index
     * \param last index
     * \return check result
     */
    bool CheckLinePoints(Line line, size_t first, size_t last)
    {
        float x1 = line.start.X;
        float y1 = line.start.Y;
        float x2 = line.end.X;
        float y2 = line.end.Y;


        auto len = sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
        for (size_t i = first; i < last; i++)
        {
            auto& p = m_data->stroke[i];
            // |(y2-y1)*x0 - (x2-x1)*y0 + x2*y1 - y2*x1|
            // -----------------------------------------
            //       sqrt((y2-y1)^2 + (x2-x1)^2)
            float dist = abs((y2 - y1) * p.X - (x2 - x1) * p.Y +x2*y1 - y2*x1)/len;
            if (dist > m_threshold_distance)
            {
                return false;
            }
        }
        return true;
    }

    /*!
     * \brief PredictLine predict line on indecies [first, last)
     * \param first index
     * \param last index
     * \return true if got line, else false
     */
    bool PredictLine(size_t first, size_t last)
    {
        float x1 = m_data->stroke[first].X;
        float y1 = m_data->stroke[first].Y;
        float x2 = m_data->stroke[last - 1].X;
        float y2 = m_data->stroke[last - 1].Y;
        if (abs(x1 - x2) > 2*m_threshold_distance ||
                abs(y1 - y2) > 2*m_threshold_distance) // Short line check
        {
            if (CheckLinePoints(Line(Point(x1, y1), Point(x2, y2)), 1, m_data->stroke.size() - 1))
            {
                m_prediction.active = RPrediction::Line;
                m_prediction.shape = std::make_shared<Line>(Point(x1, y1), Point(x2, y2));
                return true;
            }
            else
            {
                m_prediction.active = RPrediction::Unknown;
                m_prediction.shape = nullptr;
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    bool PredictBezier(size_t first, size_t last)
    {

    }

    bool CheckEllipse(Point start, Point end, size_t first, size_t last)
    {

    }

    bool PredictEllipse(size_t first, size_t last)
    {

    }
};

#endif
