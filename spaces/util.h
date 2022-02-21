//
// Created by dewe on 8/22/21.
//

#ifndef GYMENV_SPACE_UTILITY_H
#define GYMENV_SPACE_UTILITY_H

#include <sstream>
#include "vector"
#include "ostream"
#include <algorithm>
#include "c10/core/ScalarType.h"

using std::ostream;

namespace gym::space{

    struct Shape{
        std::vector<int64_t> m_Axis{};
        c10::ScalarType m_Type{c10::kInt};
    };

    template<typename T>
    struct Range{
        T low{}, high{};
    };

    template<typename T> inline
    Shape make_shape(int64_t vector_length);

    template<typename T> inline
    Shape make_shape(std::vector<int64_t> const& axis);

    static inline
    size_t dim(Shape const& shape){
        return shape.m_Axis.size();
    }

    template<typename T> inline
    T difference(Range<T> const& range){
        return range.high - range.low;
    }

    size_t flatten(std::vector<int64_t> const& shape);

    template<typename T>
    std::string stringifyVector(const std::vector<T> & vect);

    ostream& operator<<(ostream& os, const Range<float>& range);

    ostream& operator<<(ostream& os, const std::vector<Range<float >> & range);

    ostream& operator<<(ostream& os, const std::vector<int64_t> & vect);

    template<>  inline
    Shape make_shape<int>(int64_t vector_length){
        return {{vector_length}, c10::kInt};
    }

    template<>  inline
    Shape make_shape<uint8_t>(int64_t vector_length) {
        return {{vector_length}, c10::kChar};
    }

    template<>  inline
    Shape make_shape<float>(int64_t vector_length){
        return {{vector_length}, c10::kFloat};
    }

    template<>  inline
    Shape make_shape<double>(int64_t vector_length){
        return {{vector_length}, c10::kDouble};
    }

    template<>  inline
    Shape make_shape<bool>(int64_t vector_length){
        return {{vector_length}, c10::kBool};
    }

    template<>  inline
    Shape make_shape<uint8_t>(std::vector<int64_t> const& axis){
        return {axis, c10::kChar};
    }

    template<>  inline
    Shape make_shape<int>(std::vector<int64_t> const& axis){
        return {axis, c10::kInt};
    }

    template<>  inline
    Shape make_shape<float>(std::vector<int64_t> const& axis){
        return {axis, c10::kFloat};
    }

    template<>  inline
    Shape make_shape<double>(std::vector<int64_t> const& axis){
        return {axis, c10::kDouble};
    }
}

#include "../spaces/util.tpp"

#endif //GYMENV_SPACE_UTILITY_H
