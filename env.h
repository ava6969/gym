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
#include "pybind11/embed.h"
#include "pybind11/pybind11.h"
#include "mutex"
#ifdef WIN32
#define or ||
#define and &&
#define not !
#endif

namespace py = pybind11;
namespace gym{
    using namespace space;
    using AnyMap = std::unordered_map<std::string, std::any>;
    using ArgMap = std::unordered_map<std::string, std::any>;
    using OptionalArgMap = std::optional<std::unordered_map<std::string, std::any>>;

    template<typename ObservationType>
    struct StepResponse{
        ObservationType observation{};
        float reward{0.0};
        bool done{false};
        AnyMap info{};
    };

    enum class RenderType{
        HUMAN,
        IMG_RGB
    };

    template<typename ObservationType, typename ActionType, typename  StepType=StepResponse<ObservationType> >
    class Env{

    public:
        using StepT = StepResponse<ObservationType>;
        using ActionT = ActionType;
        using ObservationT = ObservationType;

        explicit Env(OptionalArgMap  args=std::nullopt): m_Args(std::move(args)){}

        Env(Env const&)=default;

        Env(Env&&) noexcept =default;

        virtual ~Env() = default;

        virtual ObservationT reset() noexcept = 0;

        virtual void render(RenderType ) {}

        virtual std::string info() { return "Gym Env"; }

        virtual void seed(std::optional<uint64_t> const& _seed) noexcept {}

        virtual StepT step(ActionT const& action) noexcept = 0;

        [[nodiscard]] inline auto actionSpace()  const noexcept { return m_ActionSpace; }

        [[nodiscard]] inline auto observationSpace() const noexcept { return m_ObservationSpace; }

        template<typename T>
        T getArg(std::string const& key, T default_) noexcept{
            return (m_Args->find(key) == m_Args->end()) ? default_ : std::any_cast<T>(m_Args->at(key));
        }

        inline auto np_random(std::optional<int> seed){
           if( not interpreter ){
               interpreter = std::make_unique<py::scoped_interpreter>();
               seeding = py::module_::import("gym.utils.seeding");
           }

            size_t seed2, seed1;
            if( seed.has_value() ){
                seed1 = seeding.attr("create_seed")( seed.value() ).template cast<size_t>();
            }
            else{
                seed1 = seeding.attr("create_seed")().template cast<size_t>();
            }
            seed2 = static_cast<int32_t>(seeding.attr("hash_seed")(seed1 + 1).template cast<size_t>()) %
                    static_cast<int32_t>(std::pow(2, 31));
            m_Device = std::mt19937 (seed2);
            return std::make_pair(seed1, seed2);
        }

    protected:
        std::shared_ptr<Space> m_ActionSpace, m_ObservationSpace;
        std::mt19937 m_Device;
        OptionalArgMap m_Args{};
        inline static std::unique_ptr<py::scoped_interpreter> interpreter{nullptr};
        inline static py::module_ seeding{};
    };

}
#endif //GYMENV_ENV_H
