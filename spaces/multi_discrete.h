//
// Created by dewe on 8/22/21.
//

#ifndef GYMENV_MULTI_DISCRETE_H
#define GYMENV_MULTI_DISCRETE_H

#include "space.h"
#include "box.h"

namespace gym::space{
    class MultiDiscrete : public Box<size_t>{

    public:
        const int64_t m_NVec;
        const std::vector<size_t> m_Dims;

        explicit MultiDiscrete(std::vector<size_t> const& dims):
        Box<size_t>(Shape{std::vector<int64_t>{(long)dims.size()}, c10::kInt}),
        m_NVec(static_cast<int64_t>(dims.size())),
        m_Dims(dims)
        {
            std::transform(m_Dims.begin(), m_Dims.end(), std::back_inserter(m_Range), [](auto size){
                return Range<size_t>{0UL, size};
            });
        }

        [[nodiscard]] std::shared_ptr<Space> clone() override{
            return cloneHelper<MultiDiscrete>();
        }

        friend ostream& operator << (ostream& os, const MultiDiscrete& dt);
    };

    ostream& operator << (ostream& os, const MultiDiscrete& dt);
}

#endif //GYMENV_MULTI_DISCRETE_H
