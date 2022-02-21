//
// Created by dewe on 9/1/21.
//

#ifndef GYMENV_SPECS_H
#define GYMENV_SPECS_H

#include "string"
#include "vector"

using namespace std::string_literals;

namespace gym{

    enum class DType{
        Int,
        Float,
        String,
        Byte
    };

    struct Array{
        const std::vector<int> shape;
        const DType dType;
        const std::string name;
    };

    template<typename T>
    struct BoundedArray : Array{
        const T minimum,
        maximum;
    };

    template<typename T>
    struct DiscreteArray : BoundedArray<T>{
        const int numValues;
    };

    template<typename StringType=std::string>
    struct StringArray : Array{
        const StringType type;
    };

}



#endif //GYMENV_SPECS_H
