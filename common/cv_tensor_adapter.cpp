//
// Created by dewe on 3/4/22.
//

#ifndef RL_TRADING_BOT_CV_TENSOR_ADAPTER_H
#define RL_TRADING_BOT_CV_TENSOR_ADAPTER_H

#include "opencv2/opencv.hpp"
#include "tensor_adapter.h"

namespace gym {

    template<>
    torch::Tensor TensorAdapter::encode(cv::Mat &&x) {
        auto[r, c, n] = std::make_tuple(x.rows, x.cols, x.channels());
        c10::ScalarType t;
        switch (x.type()) {
            case CV_32F:
                t = torch::kF32;
                break;
            default:
                t = torch::kUInt8;
        }
        return (r == 1 or c == n ? torch::from_blob(x.data, {r*c*n}, {t}) : torch::from_blob(x.data, {r, c, n}, {t})).clone();
    }

}

#endif //RL_TRADING_BOT_CV_TENSOR_ADAPTER_H
