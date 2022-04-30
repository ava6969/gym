//
// Created by dewe on 9/28/21.
//

#pragma once

#include "env.h"
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <boost/algorithm/string/predicate.hpp>
#include "opencv2/opencv.hpp"
#include "boost/algorithm/string.hpp"
#include "torch/torch.h"

namespace py = pybind11;

namespace gym{

    template<bool dict>
    using infer_obs_t = std::conditional_t<dict, std::unordered_map<std::string, py::array>, py::array>;

    template<bool cont>
    using infer_action_t = std::conditional_t<cont, std::vector<float>, int>;

    template<bool dict=false, bool cont=false, bool atari=false>
    class __attribute__ ((visibility("hidden"))) PythonEnv : public Env< infer_obs_t<dict>, infer_action_t<cont> > {
        py::object env_module;
        inline static std::unique_ptr<py::scoped_interpreter> m_Interpreter = {nullptr};
        std::vector<int64_t> obs_shape;
        c10::ScalarType obs_type;

        template<class T> inline static
        std::vector<T> PyArrayToVector(py::object const& _obj){
            auto np = _obj.cast<py::array_t<T>>();
            auto n = np.size();
            std::vector<T> v(n);
            std::memcpy(v.data(), np.request().ptr, sizeof(T)*n);
            return v;
        }

        std::unique_ptr<Space> fromPythonSpace(py::object const& space, py::module_ const& gym);

    public:

        explicit PythonEnv(std::string const& id);

        inline infer_obs_t<dict> observation( py::handle && x_in) noexcept{
            if constexpr(dict){
                infer_obs_t<dict> x_dict;
                auto x = x_in.template cast<py::dict>();
                for( auto const& entry: x)
                    x_dict[ entry.first.cast<std::string>()] = entry.second.cast<py::array>();
                return x_dict;
            }else{
                return x_in. template cast<py::array>();
            }
        }

        inline infer_obs_t<dict> reset() noexcept final{
            auto y = observation( env_module.attr("reset")() );
            return y;
        }

        StepResponse< infer_obs_t<dict> > step(infer_action_t<cont>  const& action) noexcept final;

        inline void seed(std::optional<uint64_t> const& seed) noexcept override{
            if(seed)
                env_module.attr("seed")(seed.value());
        }

        inline void render(RenderType ) override{
            env_module.attr("render")();
        }

        ~PythonEnv() override {
            env_module.attr("close")();
        }
    };


    extern template class PythonEnv<true, true, true>;
    extern template class PythonEnv<true, true, false>;
    extern template class PythonEnv<true, false, true>;
    extern template class PythonEnv<true, false, false>;
    extern template class PythonEnv<false, true, true>;
    extern template class PythonEnv<false, true, false>;
    extern template class PythonEnv<false, false, true>;
    extern template class PythonEnv<false, false, false>;
}
