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

#define ENV_MACRO(EnvT) Env<typename EnvT::ObservationT, typename EnvT::ActionT, typename EnvT::StepT>

    template<typename EnvT>
    class Wrapper : public ENV_MACRO(EnvT) {

    protected:
        std::shared_ptr<EnvT> m_Env;

    public:

        explicit Wrapper(std::unique_ptr<EnvT> env):m_Env( std::move(env) ){
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

        Wrapper<EnvT>& operator=(Wrapper<EnvT>&& x) noexcept= default;
        Wrapper<EnvT>& operator=(Wrapper<EnvT> const& x) noexcept= default;

        inline void render(RenderType type) override{
            return m_Env->render(type);
        }

        inline std::string info() override {
            return this->m_Env->info();
        }

        inline void seed(std::optional<uint64_t> const& seed) noexcept override {
            m_Env->seed(seed);
        }

        inline typename Wrapper<EnvT>::StepT step(const typename Wrapper<EnvT>::ActionT &action) noexcept override{
            return m_Env->step(action);
        }

        inline typename Wrapper<EnvT>::ObservationT reset() noexcept override {
            return m_Env->reset();
        }

        inline virtual typename Wrapper<EnvT>::StepT  step(typename Wrapper<EnvT>::StepT && prevStep) noexcept {
            return prevStep;
        }

        inline virtual typename Wrapper<EnvT>::ObservationT
        reset(typename Wrapper<EnvT>::ObservationT && x) noexcept  {
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

    template<typename EnvT>
    class ActionWrapper : public Wrapper<EnvT>{

    public:
        explicit ActionWrapper(std::unique_ptr<EnvT> env):Wrapper<EnvT>(std::move(env))
        {}

        virtual torch::Tensor action(torch::Tensor const& action) const noexcept = 0;

        virtual torch::Tensor reverseAction(torch::Tensor const& action) const noexcept = 0;

        inline typename ActionWrapper<EnvT>::StepT step(
                const typename ActionWrapper<EnvT>::ActionT &_action) noexcept override{
            return this->m_Env->step(action(_action));
        }

        inline typename ActionWrapper<EnvT>::ObservationT  reset() noexcept override { return this->m_Env->reset(); }
    };

    template<typename EnvT>
    class RewardWrapper : public Wrapper<EnvT>{

    public:
        explicit RewardWrapper(std::unique_ptr< EnvT > env):Wrapper<EnvT>(std::move(env)){}

        explicit RewardWrapper(std::shared_ptr<Space> obsSpace, std::shared_ptr<Space> actSpace ):
        Wrapper<EnvT>( obsSpace, actSpace ) {}

        [[nodiscard]] virtual float reward(float const& reward) const noexcept = 0;

        inline typename ActionWrapper<EnvT>::StepT step(const int &action) noexcept override{
            auto resp = this->m_Env->step(action);
            resp.reward = reward(resp.reward);
            return resp;
        }

        typename ActionWrapper<EnvT>::StepT step(typename ActionWrapper<EnvT>::StepT && prevStep) noexcept override{
            auto resp = prevStep;
            resp.reward = reward(resp.reward);
            return resp;
        }
    };

    template<typename EnvT>
    class ObservationWrapper : public Wrapper<EnvT>{

    public:
        explicit
        ObservationWrapper(std::unique_ptr< EnvT > env):Wrapper<EnvT>( std::move(env)) {}

        explicit ObservationWrapper(std::shared_ptr<Space> obsSpace,
                                    std::shared_ptr<Space> actSpace ):
                Wrapper<EnvT>( std::move(obsSpace), actSpace ) {}

        virtual typename Wrapper<EnvT>::ObservationT observation(
                typename Wrapper<EnvT>::ObservationT&& ) const noexcept = 0;

        inline
        typename Wrapper<EnvT>::StepT step(const typename Wrapper<EnvT>::ActionT &action) noexcept override {
            auto resp = this->m_Env->step(action);
            resp.observation = observation(std::move(resp.observation));
            return resp;
        }

        inline typename Wrapper<EnvT>::ObservationT reset() noexcept override {
            return observation(this->m_Env->reset());
        }

        inline typename Wrapper<EnvT>::StepT step(
                typename Wrapper<EnvT>::StepT && resp) noexcept override{
            return {observation(std::move(resp.observation)), resp.reward, resp.done, resp.info};
        }

        typename Wrapper<EnvT>::ObservationT reset(typename Wrapper<EnvT>::ObservationT&& x) noexcept override{
            return observation( std::move(x) );
        }
    };

    template<typename EnvT, typename OutputT>
    class ObservationWrapper2 : public Env< OutputT, typename EnvT::ActionT>{

    protected:
        std::shared_ptr< EnvT > m_Env;
    public:

        ObservationWrapper2(const std::shared_ptr<Space>& obsSpace,
                            std::shared_ptr< EnvT > env):m_Env( std::move(env)) {
            this->m_ObservationSpace = obsSpace;
            this->m_ActionSpace = this->m_Env->actionSpace();
        }

        virtual OutputT observation( typename EnvT::ObservationT && ) const noexcept = 0;

        inline StepResponse<OutputT> step(const typename EnvT::ActionT &action) noexcept override {
            return step( this->m_Env->step(action) );
        }

        inline OutputT reset() noexcept override {
            return observation(this->m_Env->reset());
        }

        inline StepResponse<OutputT> step( typename EnvT::StepT && resp) noexcept {
            return {observation(std::move(resp.observation)), resp.reward, resp.done, resp.info};
        }

        OutputT reset(typename EnvT::ObservationT && x) noexcept {
            return observation( std::move(x) );
        }

        inline void render(RenderType type) override{
            return m_Env->render(type);
        }

        inline std::string info() override {
            return this->m_Env->info();
        }

        inline void seed(std::optional<uint64_t> const& seed) noexcept override {
            m_Env->seed(seed);
        }

        inline EnvT* wrapped() { return m_Env.get(); }
    };
}
#endif //GYMENV_WRAPPER_H
