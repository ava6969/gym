// Created by dewe on 3/25/21.
//

#pragma once

#include "space.h"
#include "iostream"
#include "sstream"
#include <random>

using std::ostream;
using std::vector;

namespace gym::space{
    template<typename T>
    class Box : public Space{

    protected:
        vector<Range<T >> m_Range{};

    public:

        Box(Range<T> range, Shape const& shape):
        Space(shape),
        m_Range({range})
        {}

        Box(vector<Range<T > > range, Shape const& shape);
        Box(Box&&)=default;
        Box(Box const&)=default;
        Box(Shape const& shape): Space(shape),
        m_Range({Range<T>{std::numeric_limits<T>::min(),
                        std::numeric_limits<T>::max()}})
        {}

        [[nodiscard]] inline vector<Range<T>> getRange() const {
            return this->m_Range;
        }

       inline void getRange(std::vector<T>& low, std::vector<T> & high) const {
           low.resize(m_Range.size()); high.resize(m_Range.size());
           std::transform(begin(m_Range), end(m_Range), begin(low), begin(high), [](auto const& r, auto& _low){
               _low = r.low;
               return r.high;
           });
        }

        [[nodiscard]] std::shared_ptr<Space> clone() override{
            return cloneHelper<Box<T>>();
        }

        template<class T1>
        friend ostream& operator<<(ostream& os, const Box<T1>& box);
    };
}

#include "box.tpp"