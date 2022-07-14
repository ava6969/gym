//
// Created by dewe on 8/30/21.
//

#include "ale/ale_interface.hpp"
#include "atari_env.h"

namespace gym{

    template<bool image>
    ObsT<image> AtariEnv<image>::getObs() noexcept {
        if constexpr(not image)
            getRAM(m_Ale.get(), data.data());
        else
            getScreenRGB2(m_Ale.get(), data.data);
        return data;
    }

    template<bool image>
    int AtariEnv<image>::lives() const noexcept { return this->m_Ale->lives(); }

    template<bool image> ObsT<image> AtariEnv<image>::reset()  noexcept {
        this->m_Ale->reset_game();
        return getObs();
    }

    template<bool image>
    AtariEnv<image>::~AtariEnv() {
        if (isOpened)
            cv::destroyWindow(m_Game);
    }

    template<bool image>
    AtariEnv<image>::AtariEnv(std::string game, const std::optional<uint8_t> &mode,
                            const std::optional<uint8_t> &difficulty,
                            std::variant<int, std::array<int, 2>> frameSkip, float repeatActionProbability,
                            bool fullActionSpace,  bool render, bool sound) :
                            m_Game(std::move(game)),
                            m_FrameSkip(frameSkip),
                            m_GameMode(mode),
                            m_GameDifficulty(difficulty),
                            m_Ale(std::make_unique< ale::ALEInterface >(false)){
        setLoggerMode(2);

        if  (render or sound) {
            m_Ale->setBool("display_screen", render);
            m_Ale->setBool("sound", sound);
        }

        m_Ale->setFloat("repeat_action_probability", repeatActionProbability);

        seed(std::nullopt);

        m_ActionSet = fullActionSpace ? copy<ale::Action, int>(m_Ale->getLegalActionSet()) :
                copy<ale::Action, int>(m_Ale->getMinimalActionSet());

        this->m_ActionSpace = makeDiscreteSpace(int(m_ActionSet.size()));

        auto width = int(m_Ale->getScreen().width());
        auto height = int(m_Ale->getScreen().height());

        if constexpr(not image){
            data = std::vector<uint8_t>(128);
            this->m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {128});
            this->m_ObservationSpace->setName("observation");
        }else {
            data = cv::Mat(height, width, CV_8UC3);
            this->m_ObservationSpace = makeBoxSpace<uint8_t>(0, 255, {height, width, 3});
            this->m_ObservationSpace->setName("observation");
        }
    }

    template<bool image>
    void AtariEnv<image>::seed(std::optional<uint64_t> const& _seed) noexcept{

        long seed1;
        std::tie(this->_np_random, seed1) = np_random(_seed);
        auto seed2 = static_cast<int32_t >( hash_seed( seed1 + 1 ) % ( long(std::pow(2, 31) ) ) );

        m_Ale->setInt("random_seed", seed2 );

        std::filesystem::path home = std::getenv("HOME");
        auto fs = (home / "atari_roms" / m_Game).concat(".bin");
        auto s = fs.string();

        m_Ale->loadROM(s);

        if(m_GameMode){
            auto modes = copy<ale::game_mode_t, ale::game_mode_t>(m_Ale->getAvailableModes());
            assert(std::find(modes.begin(), modes.end(), m_GameMode.value()) != modes.end());
            m_Ale->setMode(m_GameMode.value());
        }

        if(m_GameDifficulty){
            auto difficulties = copy<ale::game_mode_t, ale::game_mode_t>(m_Ale->getAvailableModes());
            assert(std::find(difficulties.begin(), difficulties.end(), m_GameDifficulty.value()) != difficulties.end());
            m_Ale->setDifficulty(m_GameDifficulty.value());
        }
    }

    template<bool image>
    StepResponse< ObsT<image> > AtariEnv<image>::step(const int &action_idx) noexcept {
        ale::reward_t reward = 0;
        uint64_t game_action = m_ActionSet[action_idx];
        auto numSteps = std::visit(FrameStackVisitor{}, m_FrameSkip);

        while (numSteps--){
            reward += m_Ale->act(static_cast<ale::Action>(game_action));
        }

        return {getObs(), static_cast<float>(reward), m_Ale->game_over(),
                AnyMap{{"ale.lives", m_Ale->lives()}}
        };
    }

    template<bool image>
    void AtariEnv<image>::render() {

        if constexpr(image){
            auto scr = m_Ale->getScreen();
            auto [h, w] = std::pair{scr.height(), scr.width()};
            cv::Mat _image = cv::Mat(h, w, CV_8UC3, data.data);
            cv::imshow(m_Game, _image);
            cv::waitKey(1);
            isOpened = true;
        }else{
            std::cout << "Atari RAM: [";
            std::copy(data.begin(), data.end(), std::ostream_iterator<uint8_t>(std::cout, ", "));
            std::cout << " ]\n";
        }

    }

    template<bool image>
    std::vector<std::string> AtariEnv<image>::getActionMeaning(){
        std::vector<std::string> actionSet(m_ActionSet.size());
        std::transform(m_ActionSet.begin(), m_ActionSet.end(), actionSet.begin(), [](auto a){
            return ACTION_MEANING[a];
        });
        return actionSet;
    }

    template<bool image>
    ale::ALEInterface* AtariEnv<image>::ale() const noexcept { return m_Ale.get();}

    template<bool image>
    template<class SRC, class DEST>
    std::vector<DEST>  AtariEnv<image>::copy(std::vector<SRC> const& src) const noexcept {
        auto n = src.size();
        std::vector<DEST> dest(n);
        memcpy(dest.data(), src.data(), n*sizeof(SRC) );
        return dest;
    }

    template class AtariEnv<true>;
    template class AtariEnv<false>;
}

