
#ifndef RECWORKER_H
#define RECWORKER_H


#include <iostream>


#include "Modules/Recognition/recognition.h"
#include "Modules/Recognition/prediction.h"
#include "Modules/VectorGraphics/shapes.h"

#include <dlib/matrix.h>
#include <memory>
#include <thread>
#include <vector>

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
    double m_eps = 0.1;
    float min_dist_sqr = 26;

    std::vector<RPrediction> m_predictions{ 1 };
    size_t pred_idx = 0;

    double angle;
    double d_angle;
    double d_d_angle;

    double prev_angle;
    double prev_d_angle;
    double prev_d_d_angle;

    double avg_angle;

    bool new_shape = true;

    size_t current = 0;
    size_t m_idx = 0;
    size_t size = 0;

    void WorkerThread(std::shared_ptr<RecWorker>)
    {
        auto self(shared_from_this());

        std::mutex mtx;
        while (!m_data->completed)
        {
            std::unique_lock<std::mutex> lck(mtx);

            // Wait for data
            if (m_data->stroke.size() <= current + 1)
                m_data->cv.wait(lck, [&] { return true; });

            // Save size to avoid race conditions
            size = m_data->stroke.size();

            // Skip if only few points
            if (size - m_idx < 10)
                continue;

            //std::cout << "c: " << current << " i: " << m_idx << " s: " << size << std::endl;
            if (!CheckAngle())
                continue;

            // Check if fits current prediction and update
            if (CheckPrediction(m_idx, size))
            {
                continue;
            }

            if (!PredictShape(m_idx, size))
            {
                // Next shape if prediction fails
                m_idx = m_predictions[pred_idx].end;
                std::cout << "m_pred.end " << m_predictions[pred_idx].end << std::endl;
                m_predictions.emplace_back();
                pred_idx++;
            }
            
            current = size - 1;

            //prev_angle = angle;
            //prev_d_angle = d_angle;
            //prev_d_d_angle = d_d_angle;
        }

        std::cout << "End\nPredSize" << m_predictions.size() << "\n" << std::endl;

        for (auto& sh : m_predictions)
        {
            std::shared_ptr<Shs::Line> ptr;
            std::shared_ptr<Shs::Ellipse> ptrE;
            switch (sh.active) {
            case RPrediction::Line:
                std::cout << "Line through ";
                ptr = std::static_pointer_cast<Shs::Line>(sh.shape);

                m_rec_module->AddShape(ptr);

                std::cout << ptr->start.X << "," << ptr->start.Y << " " << ptr->end.X << "," << ptr->end.Y << std::endl;
                break;
            case RPrediction::Ellipse:
                std::cout << "Ellipse ";
                ptrE = std::static_pointer_cast<Shs::Ellipse>(sh.shape);

                m_rec_module->AddShape(ptrE);

                std::cout << "C:" << ptrE->center.X << "," << ptrE->center.Y
                    << " RL:" << ptrE->rl << " RS:" << ptrE->rs << " phi:" << ptrE->phi << std::endl;
                break;
            default:
                //std::cout << "Empty" << std::endl;
                break;
            }
        }
        m_predictions.clear();
        std::cout << "Clear\n" << std::endl;

        // Ellipse  Check ends(closed/open)

        // Polyline Check ends(closed/open), angles and sides

        // Polycurve Check ends(closed/open)

        // Save for next texture update

        
    }

    bool CheckAngle()
    {
        // Init angles for new shape
        auto px = m_data->stroke[m_idx].X;
        auto py = m_data->stroke[m_idx].Y;
        
        auto dx = m_data->stroke[size - 1].X - px;
        auto dy = m_data->stroke[size - 1].Y - py;
        // Skip if too close
        if (dx * dx + dy * dy < min_dist_sqr)
            return false;


        if (new_shape)
        {
            std::cout << "CheckA1" << std::endl;
            bool a = false;
            bool d_a = false;
            bool d_d_a = false;

            for (size_t i = current + 1; i < size - 1; i++)
            {
                auto dx = m_data->stroke[i].X - px;
                auto dy = m_data->stroke[i].Y - py;
                // Skip if too close
                if (dx * dx + dy * dy < min_dist_sqr)
                    continue;
                if (!a)
                {
                    avg_angle = prev_angle = atan2(dy, dx);
                    a = true;
                }
                else if (!d_a)
                {
                    angle = atan2(dy, dx);
                    prev_d_angle = angle - prev_angle;
                    prev_angle = angle;
                    d_a = true;
                }
                else if (!d_d_a)
                {
                    angle = atan2(dy, dx);
                    d_angle = angle - prev_angle;
                    prev_d_d_angle = d_angle - prev_d_angle;
                    prev_d_angle = d_angle;
                    prev_angle = angle;

                    // Loop exit
                    px = m_data->stroke[i].X;
                    py = m_data->stroke[i].Y;

                    current = i;
                    new_shape = false;

                    break;
                }

                // Update point
                px = m_data->stroke[i].X;
                py = m_data->stroke[i].Y;
            }

            //std::cout << "pa: " << prev_angle << " pb: " << prev_d_angle << " pc: " << prev_d_d_angle << std::endl;
            // Not enough points
            return false;
        }

        std::cout << "CheckA2" << std::endl;

        // Check angle
        for (size_t i = current + 1; i < size; i++)
        {
            auto dx = m_data->stroke[i].X - px;
            auto dy = m_data->stroke[i].Y - py;

            // Skip if too close
            if (dx * dx + dy * dy < min_dist_sqr)
                continue;

            angle = atan2(dy, dx);
            d_angle = angle - prev_angle;
            d_d_angle = d_angle - prev_d_angle;

            std::cout << "pa: " << prev_angle << " p_d_a: " << prev_d_angle << " p_d_d_a: " << prev_d_d_angle << std::endl;
            std::cout << "a: " << angle << " d_a: " << d_angle << " d_d_a: " << d_d_angle << std::endl;

            if (abs(d_d_angle - prev_d_d_angle) > m_threshold_angle)
            {
                size = i;
                new_shape = true;
                std::cout << "d_d_angle break" << std::endl;
                break;
            }

            prev_angle = angle;
            prev_d_angle = d_angle;
            prev_d_d_angle = d_d_angle;

            //std::cout << "pa: " << prev_angle << " pb: " << prev_d_angle <<  " pc: " << prev_d_d_angle << std::endl;
        }
        return true;
    }

    bool CheckPrediction(size_t start, size_t end)
    {
        switch (m_predictions[pred_idx].active) {
        case RPrediction::Line:
            if (CheckLinePoints(*std::static_pointer_cast<Shs::Line>(m_predictions[pred_idx].shape),
                start,
                end))
            {
                std::static_pointer_cast<Shs::Line>(m_predictions[pred_idx].shape)->end =
                    Shs::Point(m_data->stroke[end - 1].X, m_data->stroke[end - 1].Y);
                current = m_predictions[pred_idx].end = end - 1;
                return true;
            }
            break;
        case RPrediction::Ellipse:
            if (CheckEllipsePoints(*std::static_pointer_cast<Shs::Ellipse>(m_predictions[pred_idx].shape),
                start,
                end))
            {
                current = m_predictions[pred_idx].end = end - 1;
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }

    bool PredictShape(size_t start, size_t end)
    {
        if (m_predictions[pred_idx].active <= RPrediction::Line)
        {
            if (PredictLine(start, end))
                return true;
            else
                m_predictions[pred_idx].active = RPrediction::Ellipse;
        }

        if (m_predictions[pred_idx].active <= RPrediction::Ellipse)
        {
            if (PredictEllipse(start, end))
                return true;
            else
            {

            }
        }

        std::cout << "Prediction failed" << std::endl;
                
        return false;
    }

    /*!
     * \brief CheckLinePoints checks if all points are within distance from line for indecies [first, last)
     * \param first index
     * \param last index
     * \return check result
     */
    bool CheckLinePoints(const Shs::Line& line, size_t first, size_t last)
    {
        float x1 = line.start.X;
        float y1 = line.start.Y;
        float x2 = line.end.X;
        float y2 = line.end.Y;

        // Shortline check
        if (abs(m_data->stroke[last - 1].X - x1) < 2 * m_threshold_distance &&
            abs(m_data->stroke[last - 1].Y - y1) < 2 * m_threshold_distance)
        {
            return true;
        }

        auto len = sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));

        for (size_t i = first + 1; i < last; i++)
        {
            auto& p = m_data->stroke[i];
            float dist = abs((y2 - y1) * p.X - (x2 - x1) * p.Y + x2 * y1 - y2 * x1) / len;
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

        // Shortline check
        if (abs(x2 - x1) < 2*m_threshold_distance && abs(y2 - y1) < 2*m_threshold_distance)
        {
            m_predictions[pred_idx].active = RPrediction::Line;
            m_predictions[pred_idx].end = last - 1;
            m_predictions[pred_idx].shape = std::make_shared<Shs::Line>(Shs::Point(x1, y1), Shs::Point(x2, y2));
            return true;
        }

        if (CheckLinePoints(Shs::Line(Shs::Point(x1, y1), Shs::Point(x2, y2)), first, last))
        {
            m_predictions[pred_idx].active = RPrediction::Line;
            m_predictions[pred_idx].end = last - 1;
            m_predictions[pred_idx].shape = std::make_shared<Shs::Line>(Shs::Point(x1, y1), Shs::Point(x2, y2));
            return true;
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
    bool CheckEllipsePoints(const Shs::Ellipse& ellipse, size_t first, size_t last)
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
        dlib::matrix<double> D1;
        dlib::matrix<double> D2;
        D1.set_size(last - first, 3);
        D2.set_size(last - first, 3);

        for (size_t i = 0; i < last - first; i++)
        {
            D1(i, 0) = m_data->stroke[i + first].X * m_data->stroke[i + first].X;
            D1(i, 1) = m_data->stroke[i + first].X * m_data->stroke[i + first].Y;
            D1(i, 2) = m_data->stroke[i + first].Y * m_data->stroke[i + first].Y;
            D2(i, 0) = m_data->stroke[i + first].X;
            D2(i, 1) = m_data->stroke[i + first].Y;
            D2(i, 2) = 1;
        }

        // Doesn't works well with auto for some reason
        dlib::matrix<double> S1 = dlib::trans(D1) * D1;
        dlib::matrix<double> S2 = dlib::trans(D1) * D2;
        dlib::matrix<double> S3 = dlib::trans(D2) * D2;

        dlib::matrix<double> invers3 = dlib::inv(S3);
        dlib::matrix<double> trans2 = dlib::trans(S2);

        dlib::matrix<double> T = -invers3 * trans2;
        dlib::matrix<double> M = S1 + S2 * T;

        dlib::matrix<double, 3, 3> M2;
        dlib::set_rowm(M2, 0) = dlib::rowm(M, 2) / 2;
        dlib::set_rowm(M2, 1) = -dlib::rowm(M, 1);
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

        if (CheckEllipsePoints(Shs::Ellipse{ a(0), a(1), a(2), a(3), a(4), a(5) }, first, last))
        {
            m_predictions[pred_idx].active = RPrediction::Ellipse;
            m_predictions[pred_idx].end = last - 1;

            m_predictions[pred_idx].shape = std::make_shared< Shs::Ellipse>(a(0), a(1), a(2), a(3), a(4), a(5));
            return true;
        }

        return false;
    }
};

#endif
