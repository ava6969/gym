//
// Created by dewe on 8/22/21.
//E

#ifndef GYMENV_ENV_H
#define GYMENV_ENV_H
//
#include <utility>
#include <random>
#include "spaces/space.h"
#include "optional"
#include "any"
#include "mutex"
#ifdef WIN32
#define or ||
#define and &&
#define not !
#endif

namespace gym{
    using namespace space;
    using AnyMap = std::unordered_map<std::string, std::any>;
    using ArgMap = std::unordered_map<std::string, std::any>;
    using Kwargs = std::optional< ArgMap >;

    template<typename ObservationType>
    struct StepResponse{
        ObservationType observation{};
        float reward{0.0};
        bool done{false};
        AnyMap info{};
    };

    template<typename ObservationType,
            typename ActionType,
            typename StepType=StepResponse<ObservationType> >
    class Env{

    public:

        using StepT = StepType;
        using ActionT = ActionType;
        using ObservationT = ObservationType;
        using BaseEnvT = Env<ObservationType, ActionType, StepType>;

        explicit Env(Kwargs  kwargs=std::nullopt): m_Args(std::move(kwargs)){}

        Env(Env const&)=default;

        Env(Env&&) noexcept =default;

        virtual ~Env() = default;

        virtual ObservationT reset() noexcept = 0;

        virtual void render() {}

        virtual std::string info() { return "Gym Env"; }

        virtual void seed(std::optional<uint64_t> const& _seed) noexcept {
            if (_seed)
                m_Device.seed(*_seed);
            else{
                std::random_device r;                                       // 1
                std::seed_seq seed{r(), r()}; // 2
                m_Device = std::mt19937(seed);
            }
        }

        virtual StepT step(ActionT const& action) noexcept = 0;

        [[nodiscard]] inline auto actionSpace()  const noexcept { return m_ActionSpace; }

        [[nodiscard]] inline auto observationSpace() const noexcept { return m_ObservationSpace; }

        template<typename T>
        inline T getArg(std::string const& key, T default_) noexcept{
            return (m_Args->find(key) == m_Args->end()) ? default_ : std::any_cast<T>(m_Args->at(key));
        }

    protected:
        std::shared_ptr<Space> m_ActionSpace, m_ObservationSpace;
        std::mt19937 m_Device;
        Kwargs m_Args{};
    };

}
#endif //GYMENV_ENV_H
