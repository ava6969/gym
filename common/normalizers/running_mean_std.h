//
// Created by dewe on 10/27/21.
//

#ifndef SAMFRAMEWORK_RUNNING_MEAN_STD_H
#define SAMFRAMEWORK_RUNNING_MEAN_STD_H

#include "torch/torch.h"

class RunningMeanStdImpl : public torch::nn::Module{

    mutable std::mutex read_mtx;
public:
    RunningMeanStdImpl()=default;

    explicit RunningMeanStdImpl(std::vector<int64_t> const& input_shape, float eps=1e-4){
        m_Mean = register_buffer("mean", torch::zeros(input_shape, torch::kFloat32));
        m_Var = register_buffer("m_Var", torch::ones(input_shape, torch::kFloat32));
        m_Count = register_buffer("m_Count", torch::tensor(eps, torch::kFloat32));
    }

    RunningMeanStdImpl( RunningMeanStdImpl const& _clone){
        m_Mean =  _clone.m_Mean;
        m_Var =  _clone.m_Var;
        m_Count =  _clone.m_Count;
    }

    void save(torch::serialize::OutputArchive &archive) const override
    {
        archive.write("mean", m_Mean);
        archive.write("m_Var", m_Var);
        archive.write("m_Count", m_Count);
    }

    void load(torch::serialize::InputArchive &archive) override
    {
        archive.read("mean", m_Mean);
        archive.read("m_Var", m_Var);
        archive.read("m_Count", m_Count);
    }

    inline void update(torch::Tensor const& x) noexcept{
        updateFromMoment(x.mean(0), x.var(0, false), x.size(0));
    }

    void updateFromMoment(torch::Tensor const& batch_mean,
                          torch::Tensor const& batch_var,
                          int64_t const& batch_count) noexcept{

        torch::Tensor const& delta = batch_mean - m_Mean;
        auto total_count = m_Count + torch::tensor(batch_count);

        torch::Tensor  newMean = m_Mean + delta * batch_count / total_count;
        auto ma = m_Var * m_Count;
        auto mb = batch_var * batch_count;
        auto m2 = ma + mb + delta.square() * m_Count * batch_count / total_count;
        auto newVar= m2 / total_count;

        {
            std::lock_guard lk(read_mtx);
            m_Mean = newMean;
            m_Var = newVar;
        }

        m_Count = total_count;
    }

    inline auto count() const noexcept {  return m_Count; }

    template<bool clone>
    inline auto mean() const noexcept {
        if constexpr(clone)
            return mean_clone();
        return m_Mean;
    }

    template<bool clone>
    inline auto var() const noexcept {
        if constexpr(clone)
            return var_clone();
        return m_Var;
    }

    inline torch::Tensor mean_clone() const noexcept {
        std::lock_guard lk(read_mtx);
        return m_Mean;
    }
    inline torch::Tensor var_clone() const noexcept {
        std::lock_guard lk(read_mtx);
        return m_Var;
    }

private:
    torch::Tensor m_Mean{}, m_Var{}, m_Count{};
};

TORCH_MODULE(RunningMeanStd);

#endif //SAMFRAMEWORK_RUNNING_MEAN_STD_H
