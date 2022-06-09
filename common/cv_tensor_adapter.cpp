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
        return torch::from_blob(x.data, {r, c, n}, {torch::kUInt8});
    }

}

#endif //RL_TRADING_BOT_CV_TENSOR_ADAPTER_H
