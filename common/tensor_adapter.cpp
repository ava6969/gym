//
// Created by dewe on 10/30/21.
//

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

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<int>& out) {
        const auto N = v.size(0);
        assert(N == out.size());
        std::memcpy(out.data(), v.cpu().toType(torch::kInt).data_ptr<int>(), sizeof(int) * N);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<float>& out) {
        const auto N = v.size(0);
        assert(N == out.size());
        std::memcpy(out.data(), v.cpu().toType(torch::kFloat32).data_ptr<float>(), sizeof(float) * N);
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<int>> & out) {
        const auto H = v.size(0);
        auto split_tensor = v.cpu().unbind(0);
        assert(H == split_tensor.size());
        std::transform(split_tensor.begin(), split_tensor.end(), out.begin(), [](auto& x){
            x = x.dim() == 0 ? x.unsqueeze(0) : x;
            const auto W = x.size(0);
            std::vector<int> y(W);
            std::memcpy(y.data(), x.unsqueeze(1).toType(torch::kInt).template data_ptr<int>(), sizeof(int) * W);
            return y;
        });
    }

    template<>
    void TensorAdapter::decode(torch::Tensor const& v, std::vector<std::vector<float>> & out) {
        const auto H = v.size(0);
        auto split_tensor = v.cpu().unbind(0);
        assert(H == split_tensor.size());
        std::transform(split_tensor.begin(), split_tensor.end(), out.begin(), [](auto const& x){
            const auto W = x.dim() == 0 ?  1 : x.size(0);
            std::vector<float> y(W);
            std::memcpy(y.data(), x.unsqueeze(1).toType(torch::kFloat32).template data_ptr<float>(), sizeof(float) * W);
            return y;
        });
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
    torch::Tensor TensorAdapter::encode(std::vector<double> && x) {
        return torch::tensor( x );
    }

}