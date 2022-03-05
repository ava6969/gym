//
// Created by dewe on 3/4/22.
//

#ifndef RL_TRADING_BOT_CV_TENSOR_ADAPTER_H
#define RL_TRADING_BOT_CV_TENSOR_ADAPTER_H

#include "opencv2/opencv.hpp"
#include "tensor_adapter.h"

namespace gym {
    TensorDict TensorAdapter::encode(const std::unordered_map<std::string, DMObservation> &x,
                      std::unordered_map<char, int> tokenizer) {
        TensorDict _dict;

        for (auto const&[k, obs]: x) {
            _dict[k] = std::visit(overloaded{
                    [&](std::vector<double> const &x) -> torch::Tensor {
                        return torch::tensor(x);
                    },
                    [&](cv::Mat const &x) -> torch::Tensor {
                        auto[r, c, n] = std::make_tuple(x.rows, x.cols, x.channels());
                        return torch::from_blob(x.data, r * c * n, torch::kUInt8);
                    },
                    [&tokenizer](std::string const &x) -> torch::Tensor {
                        std::vector<int> result(x.size());
                        std::transform(x.begin(), x.end(), result.begin(), [&tokenizer](char _x) {
                            return tokenizer.at(_x);
                        });
                        return torch::tensor(result);
                    }}, obs);
        }
        return _dict;
    }

    template<>
    torch::Tensor TensorAdapter::encode(cv::Mat &&x) {
        auto[r, c, n] = std::make_tuple(x.rows, x.cols, x.channels());
        return torch::from_blob(x.data, {r, c, n}, {torch::kUInt8});
    }

}

#endif //RL_TRADING_BOT_CV_TENSOR_ADAPTER_H
