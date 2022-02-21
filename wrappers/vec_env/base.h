//
// Created by dewe on 12/17/21.
//

#ifndef GYM_BASE_H
#define GYM_BASE_H

#include "env.h"
#include "torch/torch.h"

namespace gym{

    /*
     * Food for thought?
     * Will I ever need to bundle env with different observation Type
     * */

    using TensorDict = std::unordered_map<std::string, torch::Tensor>;
    template<typename ObservationType>
    struct VecEnvStepReturn{
        ObservationType observation{};
        torch::Tensor reward{};
        torch::Tensor done{};
        std::vector<AnyMap> info;
    };

    template<bool dict>
    using infer_type = std::conditional_t<dict, TensorDict, torch::Tensor>;

    template<bool dict>
    class VecEnv{

    public:

        using ObservationT = infer_type<dict>;
        using ActionT = torch::Tensor;
        using StepT = VecEnvStepReturn< ObservationT >;

        VecEnv()=default;
        virtual ~VecEnv()=default;
        VecEnv(int numEnvs, std::shared_ptr<Space> const& o_space,
               std::shared_ptr<Space> const& a_space){
            this->m_ObservationSpace = o_space;
            this->m_ActionSpace = a_space;
            this->numEnvs = numEnvs;
        }

        auto actionSpace() const noexcept { return m_ActionSpace; }
        auto observationSpace() const noexcept { return m_ObservationSpace; }
        inline auto nEnvs() const { return numEnvs; }

        virtual ObservationT reset() noexcept = 0;

//        virtual std::vector<cv::Mat> GetImages() { return {}; };

        virtual void stepAsync( torch::Tensor const& actions) noexcept = 0;

        virtual StepT stepWait() noexcept = 0;

        inline StepT step(torch::Tensor const& actions) noexcept{
            stepAsync( actions );
            return stepWait();
        }

        virtual bool isWrapper() const { return false; }
//        virtual gym::Env<ObservationT, ActionT>* unwrapped(int i) = 0;

        virtual ObservationT reset(int index) = 0;

        virtual StepT step(int index, torch::Tensor const& action) noexcept = 0;

        virtual void render() const = 0;

        virtual inline std::vector<size_t> seed( size_t _seed) noexcept { return {}; }

    protected:
        int numEnvs{};
        std::shared_ptr<Space> m_ObservationSpace, m_ActionSpace;
    };
}

#endif //GYM_BASE_H
