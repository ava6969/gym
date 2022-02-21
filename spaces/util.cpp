//
// Created by dewe on 10/29/21.
//

#include "util.h"


namespace gym::space{

    size_t flatten(std::vector<int64_t> const& shape) {

        if (shape.empty())
            return 0;

        size_t product=1;

        auto&& cummulativeProduct = [&product](int64_t dim){ product *= dim;};

        std::for_each(std::begin(shape), std::end(shape), cummulativeProduct);

        return product;
    }

    ostream& operator<<(ostream& os, const std::vector<Range<float >> & range){
        std::stringstream l("["), h("[");

        for(auto const& single_range: range){
            l << single_range.low << " ,";
            h << single_range.high << " ,";
        }
        std::string l_str, h_Str;
        l_str = l.str();
        h_Str = h.str();

        l_str.back() = ']';
        h_Str.back() = ']';

        os << "{low: " << l_str << ", high: " << h_Str << "}";
        return os;
    }

    ostream& operator<<(ostream& os, const std::vector<int64_t> & vect){
        os << stringifyVector(vect);
        return os;
    }

    ostream& operator<<(ostream& os, const Range<float>& range){
        os << "Range<Float32>[" << range.low << ", " << range.high << "]";
        return os;
    }

}