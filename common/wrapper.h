// Created by dewe on 8/31/21.
//

#ifndef GYMENV_WRAPPER_H
#define GYMENV_WRAPPER_H

#include "env.h"
#include "torch/torch.h"

#define DEFAULT_RESET_OVERRIDE inline  ObservationT reset( ObservationT && x) noexcept override  {  return x; }
#define DEFAULT_STEP_OVERRIDE  inline  StepT step( StepT && prevStep) noexcept override { return prevStep; }
#define DEFAULT_RESET_OVERRIDES inline  ObservationT reset() noexcept override  {  return m_Env->reset(); }
#define DEFAULT_STEP_OVERRIDES  inline  StepT step( ActionT const& a) noexcept override { return m_Env->step(a); }
#define TYPENAME_INFO \
        using ObservationT = ObsT<true>; \
        using ActionT = int; \
        using StepT = StepResponse < ObsT<true> >;

        namespace gym {

    template<class ... Args>
    constexpr auto nType(size_t n){
        return std::tuple_element(n, std::make_tuple<Args ...>() );
    }

    template<class ... Args>
    using lastType = std::tuple_element_t<sizeof ... (Args) - 1, std::tuple<Args ...>>;

    template<size_t T, class ... Args>
    using NthType = std::tuple_element_t<T, std::tuple<Args ...>>;

    template<class ... Args>
    struct WrapperUnroll : Env<
            typename lastType<Args ...>::ObservationT,
            typename lastType<Args ...>::ActionT,
            typename lastType<Args ...>::StepT>
            {

        std::tuple< Args ... > wrappers;

        using StepT = typename lastType<Args ...>::StepT;
        using ActionT = typename lastType<Args ...>::ActionT;
        using ObservationT = typename lastType<Args ...>::ObservationT;

        WrapperUnroll(Args ... wrappers):wrappers( std::forward< Args >(wrappers) ... ){

            this->m_ObservationSpace = std::get<sizeof...(Args) - 1>( this->wrappers ).observationSpace();

            this->m_ActionSpace = std::get<sizeof...(Args) - 1>( this->wrappers ).actionSpace();
        }

        template<size_t N> inline
        ObservationT wrap(ObservationT&& x){
            if constexpr ( N < sizeof...(Args)){
                return wrap<N + 1>( std::move( std::get<N>(wrappers).reset( std::move(x) ) ) );
            }
            return x;
        }

        template<size_t N> inline
        StepT wrap(StepT&& x){
            if constexpr ( N < sizeof...(Args)){
                return wrap<N + 1>( std::move(std::get<N>(wrappers).step( std::move(x) )) );
            }
            return x;
        }

        template<size_t RenderIdx>
        void render(RenderType x)  {
            std::get<RenderIdx>(wrappers).render(x);
        }

        inline ObservationT reset() noexcept override{
            auto x = std::get<0> (wrappers);
            return wrap<1>( std::move( std::get<0> (wrappers).reset() ) );
        }

        inline StepT step(ActionT const& action) noexcept override{
            return wrap<1>( std::move( std::get<0> (wrappers).step(action) ) );
        }
    };

    template<typename ObservationType,
            typename ActionType,
            typename  StepType=StepResponse<ObservationType>>
    class Wrapper : public Env<ObservationType, ActionType, StepType> {

    protected:
        std::shared_ptr<Env<ObservationType, ActionType, StepType>> m_Env;

    public:

        using StepT = StepType;
        using ActionT = ActionType;
        using ObservationT = ObservationType;

        explicit Wrapper(std::unique_ptr<Env<ObservationType, ActionType,StepType >> env):
        m_Env( std::move(env) ){
            this->m_ObservationSpace = this->m_Env->observationSpace();
            this->m_ActionSpace = this->m_Env->actionSpace();
        }

        explicit Wrapper(std::shared_ptr<Space> obsSpace, std::shared_ptr<Space> actSpace ):
        m_Env(nullptr ){
            this->m_ObservationSpace = obsSpace;
            this->m_ActionSpace = actSpace;
        }

        Wrapper( Wrapper&& ) noexcept = default;
        Wrapper( Wrapper const& ) = default;
        Wrapper<ObservationType, ActionType, StepType>& operator=(Wrapper<ObservationType, ActionType, StepType>&& x) noexcept= default;
        Wrapper<ObservationType, ActionType, StepType>& operator=(Wrapper<ObservationType, ActionType, StepType> const& x) noexcept= default;

        inline void render(RenderType type) override{
            return m_Env->render(type);
        }

        inline void seed(std::optional<uint64_t> const& seed) noexcept override {
            m_Env->seed(seed);
        }

        inline StepType step(const ActionType &action) noexcept override{
            return m_Env->step(action);
        }

        inline ObservationType reset() noexcept override {
            return m_Env->reset();
        }

        inline virtual StepType step(StepType&& prevStep) noexcept {
            return prevStep;
        }

        inline virtual ObservationType reset(ObservationType&& x) noexcept  {
            return x;
        }

        template <class T>
        T* try_cast() const noexcept{
            if( auto* casted_env = dynamic_cast<T*>(m_Env.get()) )
                return casted_env;

            auto wrapper =  dynamic_cast<Wrapper*>(m_Env.get());

            if(not wrapper)
                return nullptr;

            return wrapper->template try_cast<T>();
        }
    };

    template<typename ObservationType,
            typename ActionType,
            typename  StepType>
    class ActionWrapper : public Wrapper<ObservationType, ActionType, StepType>{

    public:
        explicit ActionWrapper(std::unique_ptr<Env<ObservationType, ActionType, StepType>> env):
        Wrapper<ObservationType, ActionType, StepType>(std::move(env))
        {}

        virtual torch::Tensor action(torch::Tensor const& action) const noexcept = 0;

        virtual torch::Tensor reverseAction(torch::Tensor const& action) const noexcept = 0;

        inline StepResponse<ObservationType> step(const torch::Tensor &_action) noexcept override{
            return this->m_Env->step(action(_action));
        }

        inline ObservationType reset() noexcept override { return this->m_Env->reset(); }
    };

    template<typename ObservationType,
            typename ActionType,
            typename  StepType=StepResponse<ObservationType>>
    class RewardWrapper : public Wrapper<ObservationType, ActionType, StepType>{

    public:
        explicit RewardWrapper(std::unique_ptr< Env<ObservationType, ActionType, StepType> > env):
        Wrapper<ObservationType, ActionType, StepType>(std::move(env)){}

        explicit RewardWrapper(std::shared_ptr<Space> obsSpace, std::shared_ptr<Space> actSpace ):
        Wrapper<ObservationType, ActionType, StepType>( obsSpace, actSpace ) {}

        [[nodiscard]] virtual float reward(float const& reward) const noexcept = 0;

        inline StepResponse<ObservationType> step(const int &action) noexcept override{
            auto resp = this->m_Env->step(action);
            resp.reward = reward(resp.reward);
            return resp;
        }

        StepType step(StepType && prevStep) noexcept override{
            auto resp = prevStep;
            resp.reward = reward(resp.reward);
            return resp;
        }
    };

    template<typename ObservationType, typename ActionType,
            typename  StepType=StepResponse<ObservationType>>
    class ObservationWrapper : public Wrapper<ObservationType, ActionType, StepType>{

    public:
        explicit
        ObservationWrapper(std::unique_ptr< Env<ObservationType, ActionType, StepType> > env):
        Wrapper<ObservationType, ActionType, StepType>( std::move(env)) {}

        explicit ObservationWrapper(std::shared_ptr<Space> obsSpace, std::shared_ptr<Space> actSpace ):
                Wrapper<ObservationType, ActionType, StepType>( std::move(obsSpace), actSpace ) {}

        virtual ObservationType observation(ObservationType&& ) const noexcept = 0;

        inline
        StepResponse<ObservationType> step(const ActionType &action) noexcept override {
            auto resp = this->m_Env->step(action);
            resp.observation = observation(std::move(resp.observation));
            return resp;
        }

        inline ObservationType reset() noexcept override { return observation(this->m_Env->reset()); }

        inline StepType step(StepType&& resp) noexcept override{
            return {observation(std::move(resp.observation)), resp.reward, resp.done, resp.info};
        }

        ObservationType reset(ObservationType&& x) noexcept override{
            return observation( std::move(x) );
        }
    };
}
#endif //GYMENV_WRAPPER_H
