
#ifndef RECWORKER_H
#define RECWORKER_H


#include <iostream>


#include "Modules/Recognition/recognition.h"
#include "Modules/Recognition/prediction.h"
#include "Modules/VectorGraphics/shapes.h"

#include <dlib/matrix.h>
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
                if (CheckLinePoints(*std::static_pointer_cast<Shape::Line>(m_prediction.shape),
                                    current,
                                    size))
                {
                    current = size - 1;
                    continue;
                }
                break;
            case RPrediction::Ellipse:
                if (CheckEllipsePoints(*std::static_pointer_cast<Shape::Ellipse>(m_prediction.shape),
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
            if (!PredictLine(0, size))
            {
                PredictEllipse(0, m_data->stroke.size());
            }

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
     * \brief CheckLinePoints checks if all points are within distance from line for indecies [first, last)
     * \param first index
     * \param last index
     * \return check result
     */
    bool CheckLinePoints(Shape::Line line, size_t first, size_t last)
    {
        float x1 = line.start.X;
        float y1 = line.start.Y;
        float x2 = line.end.X;
        float y2 = line.end.Y;

        auto len = sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
        for (size_t i = first; i < last; i++)
        {
            auto& p = m_data->stroke[i];
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
            if (CheckLinePoints(Shape::Line(Shape::Point(x1, y1), Shape::Point(x2, y2)), 1, m_data->stroke.size() - 1))
            {
                m_prediction.active = RPrediction::Line;
                m_prediction.shape = std::make_shared<Shape::Line>(Shape::Point(x1, y1), Shape::Point(x2, y2));
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

    //bool PredictBezier(size_t first, size_t last)
    //{

    //}

    /*!
     * \brief CheckEllipsePoints checks if all points are within algebraic distance(a*x^2 + b*x*y + c*y^2 *d*x + e*y + f) on indecies [first, last)
     * \param first index
     * \param last index
     * \return check result
     * https://github.com/0xfaded/ellipse_demo/issues/1
     */
    bool CheckEllipsePoints(Shape::Ellipse ellipse, size_t first, size_t last)
    {
        auto s = sin(-ellipse.phi);
        auto c = cos(-ellipse.phi);
        dlib::matrix<double, 2, 2> rot2d{ c, -s , s, c };

        for (size_t i = first; i < last; i++)
        {
            dlib::matrix<double, 2> p{ m_data->stroke[i].X - ellipse.center.X, 
                                       m_data->stroke[i].Y - ellipse.center.Y };
            dlib::matrix<double, 2> p_rot = rot2d * p;
            dlib::matrix<double, 2> p_abs = dlib::abs(p_rot);
            dlib::matrix<double, 2> t{ 0.707 , 0.707 };

            dlib::matrix<double, 2> ss{ ellipse.rl , ellipse.rs };
            auto ss_sub = (ss(0) * ss(0)) - (ss(1) * ss(1));
            dlib::matrix<double, 2> efac{ ss_sub, -ss_sub };

            dlib::matrix<double, 2> xy;
            for (int k = 0; k < 3; k++)
            {
                xy = dlib::pointwise_multiply(ss, t);
                dlib::matrix<double, 2> pow_t{ t(0) * t(0) * t(0), t(1) * t(1) * t(1) };
                auto e = dlib::pointwise_divide(dlib::pointwise_multiply(efac, pow_t), ss);
                auto q = p_abs - e;
                auto rq = dlib::length(xy - e) / dlib::length(q);
                t = dlib::normalize(dlib::pointwise_divide(q * rq + e, ss));
            }
            dlib::matrix<double, 2> p_edge{ xy(0) * (p_rot(0) >= 0 ? 1 : -1), xy(1) * (p_rot(1) >= 0 ? 1 : -1) };
            auto dist = dlib::length(p_edge - p_rot);

            if (dist > m_threshold_distance)
            {
                return false;
            }
        }

        return true;
    }

    /*!
     * \brief PredictEllipse predict ellipse on indecies [first, last)
     * \param first index
     * \param last index
     * \return true if got ellipse, else false
     * http://www.users.on.net/~zygmunt.szpak/ellipsefitting.html
     * src http://www.users.on.net/~zygmunt.szpak/sourcecode.html direct ellipse fit
     */
    bool PredictEllipse(size_t first, size_t last)
    {
        //std::vector<Shape::Point> points{ {0, 1}, {0, -1}, {-1, 0}, {1, 0} };
        dlib::matrix<double> D1;
        dlib::matrix<double> D2;
        D1.set_size(last - first, 3);
        D2.set_size(last - first, 3);

        for (size_t i = first; i < last; i++)
        {
            D1(i, 0) = m_data->stroke[i].X * m_data->stroke[i].X;
            D1(i, 1) = m_data->stroke[i].X * m_data->stroke[i].Y;
            D1(i, 2) = m_data->stroke[i].Y * m_data->stroke[i].Y;
            D2(i, 0) = m_data->stroke[i].X ;
            D2(i, 1) = m_data->stroke[i].Y ;
            D2(i, 2) = 1;
        }

        // Doesn't works well with auto for some reason
        dlib::matrix<double> S1 = dlib::trans(D1) * D1;
        dlib::matrix<double> S2 = dlib::trans(D1) * D2;
        dlib::matrix<double> S3 = dlib::trans(D2) * D2;

        dlib::matrix<double> invers3 = dlib::inv(S3);
        dlib::matrix<double> trans2 = dlib::trans(S2);

        dlib::matrix<double> T = - invers3 * trans2;
        dlib::matrix<double> M = S1 + S2 * T;

        dlib::matrix<double, 3, 3> M2;
        dlib::set_rowm(M2, 0) = dlib::rowm(M, 2) / 2;
        dlib::set_rowm(M2, 1) = - dlib::rowm(M, 1);
        dlib::set_rowm(M2, 2) = dlib::rowm(M, 0) / 2;
        
        dlib::eigenvalue_decomposition<dlib::matrix<double>> eigen(M2);
        auto evec = eigen.get_v();
        dlib::matrix<double, 3, 1> al;
        for (int i = 0; i < evec.nr(); i++)
        {
            auto cond = 4 * evec(0, i).real() * evec(2, i).real() - evec(1, i).real() * evec(1, i).real();
            if (cond > 0)
            {
                al(0, 0) = evec(0, i).real();
                al(1, 0) = evec(1, i).real();
                al(2, 0) = evec(2, i).real();
            }
        }
        dlib::matrix<double> tal = T * al;
        dlib::matrix<double, 6, 1> a;
        a(0) = al(0);
        a(1) = al(1);
        a(2) = al(2);
        a(3) = tal(0);
        a(4) = tal(1);
        a(5) = tal(2);


        a = dlib::normalize(a);

        CheckEllipsePoints(Shape::Ellipse{ a(0), a(1), a(2), a(3), a(4), a(5) }, first, last);
        

        return true;
    }
};

#endif
