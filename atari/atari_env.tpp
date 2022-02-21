//
// Created by dewe on 8/30/21.
//


namespace gym{

    template<bool image>
    AtariEnv<image>::AtariEnv(std::string game, const std::optional<uint8_t> &mode,
                            const std::optional<uint8_t> &difficulty,
                            std::variant<int, std::array<int, 2>> frameSkip, float repeatActionProbability,
                            bool fullActionSpace,  bool render, bool sound) :
                            m_Game(std::move(game)),
                            m_FrameSkip(frameSkip),
                            m_GameMode(mode),
                            m_GameDifficulty(difficulty),
                            m_Ale(new ale::ALEInterface(false)){
        setLoggerMode(2);

    #ifdef ALE_SDL_SUPPORT
        m_Ale->setBool("display_screen", render);
        m_Ale->setBool("sound", sound);
    #endif

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
        auto[seed1, seed2] = this->np_random(_seed);

        m_Ale->setInt("random_seed", seed2);

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
    void AtariEnv<image>::render(RenderType) {
        auto scr = m_Ale->getScreen();
        cv::Mat _image = cv::Mat(scr.height(), scr.width(), CV_8UC3, 0);
        getScreenRGB2(m_Ale.get(), _image.data);
        cv::imshow(m_Game, _image);
        cv::waitKey(1);
        isOpened = true;
    }

    template<bool image>
    ale::ALEInterface* AtariEnv<image>::ale() const noexcept { return m_Ale.get();}

}