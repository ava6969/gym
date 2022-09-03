//
// Created by dewe on 11/9/21.
//

#ifndef SAM_RL_STACKED_OBSERVATION_H
#define SAM_RL_STACKED_OBSERVATION_H

#include "utils.h"
#include "spaces/space.h"
#include "variant"
#include "torch/torch.h"


namespace gym{

    template<bool dict>
    class StackedObservation {
    public:

        template<class T=at::Tensor>
        using infer = std::conditional_t<dict, std::unordered_map<std::string, T>, T>;

        StackedObservation(int numEnvs,
                           int numStacks,
                           std::shared_ptr<gym::Space> const& observationSpace,
                           std::optional< std::string> const& channelsOrder = std::nullopt ):
                           nStack(numStacks){

            if constexpr(dict){
                for (auto const& space: observationSpace->namedSpaces())
                {
                    auto const& [key, subspace] = space;
                    std::tie(channelFirst[key], stackDimension[key], stackedObs[key], repeatAxis[key])
                    = computeStacking( numEnvs, numStacks, subspace->clone(), channelsOrder );
                }
            }else{
                std::tie(channelFirst, stackDimension, stackedObs, repeatAxis) = computeStacking( numEnvs,
                                                                                                  numStacks,
                                                                                                  observationSpace,
                                                                                                  channelsOrder);

            }
        }

        template<typename T> static
        std::vector<T> repeat(std::vector<T> const& x, int axis, int stacks){
            auto t = torch::tensor(x);
            auto result = torch::repeat_interleave(t, stacks, axis).view(-1);
            auto n = result.size(0);
            std::vector<T> vector_result(n);
            memcpy(vector_result.data(), result.template data_ptr<T>(), sizeof(T) * n);
            return vector_result;
        }

        template<typename T>
        std::shared_ptr<gym::Space> stackObservationSpace(const gym::Box<T> * space,
                                                          std::string const& key){
            auto range = space->getRange()[0];
            auto sz = space->size();
            if constexpr(dict)
                *(sz.end() + repeatAxis[key] ) = nStack;
            else
                *(sz.end() + repeatAxis) = nStack;
            return gym::makeBoxSpace<T>( range.low, range.high, sz);
        }

        std::shared_ptr<gym::Space> stackObservationSpace(std::shared_ptr<gym::Space> const& _space, std::string const& key){

            if(auto boxSpace = _space->as<gym::space::Box<float>>())
                return stackObservationSpace( boxSpace, key );
            else if( auto uIntSpace = _space->as<gym::space::Box<uint8_t>>() ){
                return stackObservationSpace( uIntSpace, key );
            }
            else{
                excludes.insert(key);
                std::cerr << "Not Stacking [" << key << "]\n";
                return _space;
            }

        }

        std::shared_ptr<gym::Space> stackObservationSpace(gym::ADict* space){
            gym::space::NamedSpaces spaces;
            for( auto const& [name, _space] : space->namedSpaces() ){
                spaces[name] = stackObservationSpace( _space->clone(), name );
            }
            return std::make_shared<gym::ADict>( spaces );
        }

        std::shared_ptr<gym::Space> stackObservationSpace(gym::space::Space* space){
            if constexpr(dict){
                return stackObservationSpace( space->template as<ADict>() );
            }else{
                return stackObservationSpace( space->clone(), "observation" );
            }
        }

        inline void stack(at::Tensor& y, at::Tensor const& x, int size, bool channel_first){
            using S = at::indexing::Slice;
            y = channel_first ? y.index_put_({S(), S(-size), "..."}, x) : y.index_put_({"...", S(-size)}, x);
        }

        void render(){
            if constexpr(not dict)
                plot("last stack", stackedObs[nStack-1], 84, 84, false);
        }

        inline void reset(at::Tensor & y, at::Tensor const& x, int stackDim, bool channel_first){
            y.index_put_({"..."}, 0);
            stack(y, x, x.size( stackDim ), channel_first);
        }

        infer<> reset(infer<> const& x){

            if constexpr(dict){
                for (auto const& [key, obs]: x) {
                    if(not excludes.contains(key))
                        reset(stackedObs[key], obs, stackDimension[key], channelFirst[key]);
                    else
                        stackedObs[key] = obs;
                }
            }else{
                reset(stackedObs, x, stackDimension, channelFirst);
            }
            return stackedObs;
        }

        void update(at::Tensor & y, at::Tensor const& x, int stackDim, bool channel_first){
            auto stackAxSize = x.size( stackDim );
            y = at::roll(y, -stackAxSize, stackDim);
            stack(y, x, stackDim, channel_first);
        }

        auto update(infer<> const& x){
            if constexpr( dict){
                for (auto const& [key, obs] : x) {
                    if(not excludes.contains(key))
                        update(stackedObs[key], obs, stackDimension[key], channelFirst[key]);
                    else
                        stackedObs[key] = obs;
                }
            }else{
                update(stackedObs, x, stackDimension, channelFirst);
            }

            return stackedObs;
        }

    private:
        int nStack{};
        infer<bool> channelFirst{};
        infer<int> stackDimension{};
        infer<at::Tensor> stackedObs{};
        infer<int> repeatAxis{};
        std::set<std::string> excludes;

        static bool isImageSpaceChannelsFirst(std::shared_ptr<Space> const&  space){
            auto sz = space->size();
            auto smallestDimIt = std::min_element(sz.begin(), sz.end());
            auto smallestDim = smallestDimIt - sz.begin();
            if (smallestDim == 1){
                throw std::runtime_error("Treating image space as channels-last, while second dimension "
                                         "was smallest of the three.");
            }
            return smallestDim == 0;
        }

        static bool isImageSpace( std::shared_ptr<Space> const& o_space, bool checkChannels=false ){
            if( auto _space = o_space->as<gym::space::Box<uint8_t>>() ){
                if( _space->size().size() == 3){
                    std::vector<uint8_t> low, high;
                    _space->getRange(low, high);
                    if( std::any_of(std::begin(low), std::end(low), [](auto x){ return x != 0; })
                        or std::any_of(std::begin(high), std::end(high), [](auto x){ return x != 255; }) )
                        return false;

                    if(not checkChannels)
                        return true;

                    auto nChannels = isImageSpaceChannelsFirst(o_space) ? _space->size()[0] : _space->size().back();
                    auto check = {1, 3, 4};
                    return std::find(check.begin(), check.end(), nChannels) != check.end();
                }
            }
            return false;
        }

        static auto computeStacking(int n_envs, int n_stacks, std::shared_ptr<gym::Space> const& oSpace,
                                    std::optional< std::string > const& channelsOrder){

            bool channels_first = false;
            if( not channelsOrder ){
                if( isImageSpace(oSpace) ){
                    channels_first = isImageSpaceChannelsFirst( oSpace );
                } else
                    channels_first = false;
            }else{
                std::vector lastFirst{"last", "first"};
                auto result = std::ranges::find(lastFirst, *channelsOrder);
                if(result != end(lastFirst))
                    throw std::runtime_error("`channels_order` must be one of following: 'last', 'first'");
                channels_first = (channelsOrder.value() == "first");
            }

            auto shape = oSpace->size();
            auto stack_dim = channels_first ? 1 : -1;
            auto repeat_axis = channels_first ? 0 : -1;
            *(shape.end() + repeat_axis) = n_stacks;
            shape.insert(shape.begin(), n_envs);
            auto stacked_obs = torch::zeros(shape, oSpace->type());
            return std::make_tuple( channels_first, stack_dim, stacked_obs, repeat_axis );
        }

    };
}
#endif //SAM_RL_STACKED_OBSERVATION_H
