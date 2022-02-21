//
// Created by dewe on 10/9/21.
//

#include "common/utils.h"
#include "torch/torch.h"
namespace gym{

    template<bool dict, bool cont, bool atari>
    StepResponse< infer_obs_t<dict> > PythonEnv<dict, cont, atari>::step(const ActionT & action) noexcept {

        py::tuple tuple_type;

        if constexpr( cont and !atari) {
            auto n = action.size();
            if( n == 1)
                tuple_type = env_module.attr("step")(action).template cast<py::tuple>();
            else {
                py::array_t<float> x(n);
                memcpy(x.request().ptr, action.data(), sizeof(float) * n);
                tuple_type = env_module.attr("step")(x).cast<py::tuple>();
            }

        }else {
            tuple_type = env_module.attr("step")(action).template cast<py::tuple>();
        }

        auto rew_v = tuple_type[1].cast<float>();
        auto done_v = tuple_type[2].cast<bool>();

        auto _info = tuple_type[3].cast<py::dict>();

        AnyMap info;
        for(auto&[k, v] : _info)
        {
            auto k_str = k.template cast<std::string>();
            if(k_str == "episode")
            {
                info["episode"] = Result{v["r"].template cast<float>(), v["l"].template cast<float>(), 0};
            }else{
                try{
                    info[k_str] = v.template cast<float>();
                } catch (std::exception const& ) {}
            }
        }
        return {tuple_type[0], rew_v, done_v, info};
    }

    template<bool dict, bool cont, bool atari>
    PythonEnv<dict, cont, atari>::PythonEnv(std::string const& id):
    Env<infer_obs_t < dict>, infer_action_t<cont>>(){
        using namespace py::literals;

        if(m_Interpreter == nullptr){
            m_Interpreter = std::make_unique<py::scoped_interpreter>();
            auto sys = py::module_::import("sys");
            sys.attr("path").attr("append")("/home/dewe/anaconda3/lib/python3.8");
            sys.attr("path").attr("append")("/home/dewe/anaconda3/lib/python3.8/site-packages");
            py::print( sys.attr("path"));
            py::print( sys.attr("executable"));
        }

        auto gym = py::module_::import("gym");

        if constexpr(atari){
            auto m = py::module_::import("atari_wrappers");
            env_module = m.attr("make")(id);
            this->m_ActionSpace = makeDiscreteSpace(6);
            this->m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {84, 84, 1});
        }else{
            env_module = gym.attr("make")(id);
            auto observation_space_py = env_module.attr("observation_space");
            auto action_space_py = env_module.attr("action_space");

            this->m_ActionSpace = fromPythonSpace(action_space_py, gym);
            this->m_ObservationSpace = fromPythonSpace(observation_space_py, gym);
        }
        obs_shape = this->m_ObservationSpace->size();
    }

    template<bool dict, bool cont, bool atari>
    std::unique_ptr<Space> PythonEnv<dict, cont, atari>::fromPythonSpace(const py::object &space, const py::module_ &gym) {

        if(py::isinstance(space, gym.attr("spaces").attr("box").attr("Box"))){
            auto low = space.attr("low");
            auto high = space.attr("high");
            auto shape = space.attr("shape").cast<py::tuple>();
            auto dtype = space.attr("dtype");
            auto type_name = dtype.attr("name").cast<std::string>();

            if(boost::starts_with(type_name, "float")){
                auto sz = shape[0].cast<int64_t>();
                obs_type = c10::kFloat;
                return makeBoxSpace(PyArrayToVector<float>(low),
                                    PyArrayToVector<float>(high),
                                    sz);
            }
            else if(boost::starts_with(type_name, "uint8")){
                auto sz = std::vector<int64_t>();
                obs_type = torch::kUInt8;
                for(auto  const& s: shape)
                    sz.emplace_back(s.cast<int64_t>());
                return makeBoxSpace<uint8_t>(0, 255, sz);
            }

            else if(boost::starts_with(type_name, "uint")){
                auto sz = shape[0].cast<int64_t>();
                obs_type = torch::kInt64;
                return makeBoxSpace(PyArrayToVector<float>(low),
                                    PyArrayToVector<float>(high),
                                    sz);
            }
            else if(boost::starts_with(type_name, "int")){
                auto sz = shape[0].cast<int64_t>();
                obs_type = torch::kInt64;
                return makeBoxSpace(PyArrayToVector<float>(low),
                                    PyArrayToVector<float>(high),
                                    sz);
            }else{
                throw std::runtime_error("only support float, int, uint dtype ");
            }
        }else if(py::isinstance(space, gym.attr("spaces").attr("discrete").attr("Discrete"))){
            return makeDiscreteSpace(space.attr("n").cast<uint64_t>());
        }else if(py::isinstance(space, gym.attr("spaces").attr("dict").attr("Dict"))){

            NamedSpaces spaces;
            for(auto const& entry: space.attr("spaces").cast<py::dict>()){
                spaces.emplace(entry.first.cast<std::string>(),
                               fromPythonSpace(entry.second.cast<py::object>(), gym));
            }
            return makeDictionarySpace(std::move(spaces));
        }else{
            throw std::runtime_error("Invalid Space");
        }
    }
}