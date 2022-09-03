//
// Created by dewe on 12/28/21.
//

#pragma once

#include "wrappers/vec_env/vec_env_wrapper.h"
#include "torch/torch.h"

namespace gym {

    template<bool dict>
    class VecNormAndPermute : public VecEnvWrapper<dict> {

    private:
        std::vector<std::string> key;
        bool norm{true};

    public:
        explicit VecNormAndPermute(std::unique_ptr <VecEnv<dict>> env);

        explicit VecNormAndPermute(std::unique_ptr <VecEnv<dict>> env,
                                   std::shared_ptr <Space> obsSpace, std::shared_ptr <Space> actSpace);

        [[nodiscard]] inline torch::Tensor observation(torch::Tensor const &x) const noexcept;

        [[nodiscard]] inline auto observation(TensorDict x) const noexcept;

        inline typename VecEnv<dict>::ObservationT reset() noexcept override;

        inline typename VecEnv<dict>::ObservationT reset(int index) override;

        typename VecEnv<dict>::StepT stepWait() noexcept override;

        typename VecEnv<dict> ::StepT step(int index, torch::Tensor const& action) noexcept override;

        static std::unique_ptr< VecEnv<dict> > make(std::unique_ptr< VecEnv<dict> > x);
    };

    extern template class VecNormAndPermute<true>;
    extern template class VecNormAndPermute<false>;
}
