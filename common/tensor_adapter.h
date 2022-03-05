//
// Created by adede on 9/7/2021.
//

#ifndef SAMFRAMEWORK_TENSOR_ADAPTER_H
#define SAMFRAMEWORK_TENSOR_ADAPTER_H

#include <unordered_map>
#include <string>
#include <torch/torch.h>
#include "vector"
#include "../custom/dm_lab/base_lab.h"
#include "array"
#include "pybind11/numpy.h"


namespace py = pybind11;

using std::vector;
using std::string;

using TensorDict = std::unordered_map<string, torch::Tensor>;
namespace cv{
    class Mat;
}

// helper type for the visitor
template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

namespace gym{
    struct TensorAdapter {

        template<class T> static
        void decode(torch::Tensor const& v, T& y);

        template<class T> static
        torch::Tensor encode(T && x);

        static torch::Tensor encode(torch::Tensor && x) { return x; }

        static TensorDict
        encode(std::unordered_map<std::string, DMObservation> const &x, std::unordered_map<char, int> tokenizer = {});

        template<class T>
        static inline auto complete(py::array && x, std::vector<ssize_t> const& sshape, ssize_t n){
            std::vector<T> x_vec(n);
            std::memmove( x_vec.data(), x.request().ptr, sizeof(T) * n);
            return torch::tensor(x_vec).view(sshape);
        }

        template<typename T>
        static inline
        auto encode(std::unordered_map<std::string, T> const &x) {
            TensorDict result;
            for (auto const&[k, v]: x)
                result[k] = encode(v);
            return result;
        }

    };

    static std::ostream &operator<<(std::ostream &os, TensorDict const &t_map) {
        for (auto &t: t_map)
            os << t << "\n";
        return os;
    }
}


#endif //SAMFRAMEWORK_TENSOR_ADAPTER_H
