//
// Created by dewe on 10/30/21.
//


#include "pybind11/numpy.h"
#include "tensor_adapter.h"

namespace gym {

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, int& y) {
        y = v.item<int>();
    }
    template<>
    void TensorAdapter::decode(torch::Tensor const& v, float& y) {
        y = v.item<float>();
    }

    template<typename T>
    void _mem_cpy(torch::Tensor const& v, std::vector<T>& out, size_t N){
        if constexpr( std::is_same_v<T, int>)
            std::memcpy(out.data(), v.cpu().toType(torch::kInt32).data_ptr<T>(), sizeof(int) * N);
        else if constexpr( std::is_same_v<T, int64_t>)
            std::memcpy(out.data(), v.cpu().toType(torch::kInt64).data_ptr<T>(), sizeof(int64_t) * N);
        else if constexpr( std::is_same_v<T, double>)
            std::memcpy(out.data(), v.cpu().toType(torch::kFloat64).data_ptr<T>(), sizeof(double) * N);
        else if constexpr( std::is_same_v<T, float>)
            std::memcpy(out.data(), v.cpu().toType(torch::kFloat32).data_ptr<T>(), sizeof(float) * N);
        else{
            static_assert(true, "action type currently supported [int32, int64, double float]");
        }
    }

    template<typename T>
    void _decode(torch::Tensor const& v, std::vector<T>& out) {
        const auto N = v.size(0);
        assert(N == out.size());
        _mem_cpy(v, out, N);
    }

    template<typename T>
    void _decode(torch::Tensor const& v, std::vector<std::vector<T>> & out) {
        const auto H = v.size(0);
        auto split_tensor = v.cpu().unbind(0);
        assert(H == split_tensor.size());
        std::transform(split_tensor.begin(), split_tensor.end(), out.begin(), [](auto& x){
            x = x.dim() == 0 ? x.unsqueeze(0) : x;
            const size_t W = x.size(0);
            std::vector<T> y(W);
            _mem_cpy(x.unsqueeze(1), y, W);
            return y;
        });
    }

    template<class T>
    static inline auto complete(py::array && x, std::vector<ssize_t> const& sshape, ssize_t n){
        std::vector<T> x_vec(n);
        std::memmove( x_vec.data(), x.request().ptr, sizeof(T) * n);
        return torch::tensor(x_vec).view(sshape);
    }

    template<>
    torch::Tensor TensorAdapter::encode(py::array && x) {
        auto n = x.size();
        auto type = x.dtype().kind();
        auto shape = x.shape();
        std::vector<ssize_t> sshape(x.ndim());
        std::memmove(sshape.data(), shape, sizeof(ssize_t) * x.ndim());

        switch (type) {
            case 'u':
                return complete<uint8_t>( std::move(x), sshape, n);
            case 'f':
                return complete<float>( std::move(x), sshape, n);
            case 'i':
                return complete<int>( std::move(x), sshape, n);
            default:
                throw std::runtime_error("numpy array conversion failed - only supports[u|f|i]");
        }
    }


    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<int>& out) {
        _decode<int>(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<int64_t>& out) {
        _decode<int64_t>(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<float>& out) {
        _decode<float>(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<double>& out) {
        _decode<double>(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<int>> & out) {
        _decode(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<float>> & out) {
        _decode(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<int64_t>> & out) {
        _decode(v, out);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<double>> & out) {
        _decode(v, out);
    }

    template<>
    torch::Tensor TensorAdapter::encode(std::vector<double> && x) {
        return torch::tensor( x );
    }

    template<>
    torch::Tensor TensorAdapter::encode(std::vector<int> && x) {
        return torch::tensor( x );
    }

    template<>
    torch::Tensor TensorAdapter::encode(std::vector<float> && x) {
        return torch::tensor( x );
    }

}