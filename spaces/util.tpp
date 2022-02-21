
#pragma once

namespace gym::space{
    template<typename T>
    std::string stringifyVector(const std::vector<T> & vect){
        std::stringstream l("[");

        for(auto const& element: vect){
            l << element << " ,";
        }
        std::string l_str;
        l_str = l.str();

        l_str.back() = ']';

        return l_str;
    }
}