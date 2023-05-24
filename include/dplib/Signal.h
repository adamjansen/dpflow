#pragma once
#include <limits>
#include <string>

namespace datapanel
{
class Signal
{
  public:
    double minimum() const
    {
        return m_minValue;
    }
    void setMinimum(double minimum)
    {
        m_minValue = minimum;
    }
    double maximum() const
    {
        return m_maxValue;
    }
    void setMaximum(double maximum)
    {
        m_maxValue = maximum;
    }
    double scale() const
    {
        return m_scale;
    }
    void setScale(double scale)
    {
        m_scale = scale;
    }
    double offset() const
    {
        return m_offset;
    }
    void setOffset(double offset)
    {
        m_offset = offset;
    }

    std::string name() const
    {
        return m_name;
    }

    void setName(std::string name)
    {
        m_name = name;
    }

    std::string comment() const
    {
        return m_comment;
    }

    void setComment(std::string name)
    {
        m_comment = name;
    }

  private:
    double m_minValue = std::numeric_limits<double>::min();
    double m_maxValue = std::numeric_limits<double>::max();

    double m_scale = 1.0;
    double m_offset = 0;

    std::string m_name;
    std::string m_comment;
};
}  // namespace datapanel
