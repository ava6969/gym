//
// Created by dewe on 5/21/22.
//

#include "src/vecgame.h"
#include "src/game.h"
#include "common/utils.h"
#include "procgen.h"


namespace gym {
    BaseProcgenEnv::BaseProcgenEnv(const Option &opt):
            handle(libenv_make(1, makeOption(opt))),
            game( ((VecGame *)(handle))->games[0] ){

        obs_data = cv::Mat(64, 64, CV_8UC3);
    }

    libenv_options BaseProcgenEnv::makeOption(const BaseProcgenEnv::Option &opt) {

        base_opt = opt;
        base_opt.num_threads = 0;
        combos = getCombos();
        base_opt.num_actions = combos.size();

        if( base_opt.rand_seed == -1){
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
            std::uniform_int_distribution<> distrib(0, int( pow(2, 31) - 1 ));
            base_opt.rand_seed = distrib( gen );
        }

        if(!DISTRIBUTION_MODE_DICT.contains(base_opt.distribution_mode)){
            throw std::runtime_error(base_opt.distribution_mode + " is not a valid distribution mode.");
        }

        if( base_opt.distribution_mode == "exploration"){
            if(!EXPLORATION_LEVEL_SEEDS.contains(base_opt.env_name)){
                throw std::runtime_error(base_opt.env_name + " does not support exploration mode ");
            }
            base_opt.distribution_mode_enum = DISTRIBUTION_MODE_DICT.at("hard");
            base_opt.num_levels = 1;
            base_opt.start_level = EXPLORATION_LEVEL_SEEDS.at(base_opt.env_name);
        }else{
            base_opt.distribution_mode_enum = DISTRIBUTION_MODE_DICT.at(base_opt.distribution_mode);
        }

        opts.push_back( {"env_name", LIBENV_DTYPE_UINT8,
                         static_cast<int>(opt.env_name.size()),
                           (void*)base_opt.env_name.data()} );

        opts.push_back( {"num_levels", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.num_levels)} );

        opts.push_back( {"start_level", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.start_level)} );

        opts.push_back( {"num_actions", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.num_actions)} );

        opts.push_back( {"rand_seed", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.rand_seed)} );

        opts.push_back( {"num_threads", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.num_threads)} );

        opts.push_back( {"resource_root", LIBENV_DTYPE_UINT8,
                         static_cast<int>(base_opt.resource_root.size()),
                         (void*)base_opt.resource_root.data()} );

//        opts.push_back( {"render_human", LIBENV_DTYPE_UINT8, 1,
//                         (void*)(&base_opt.render_human)} );

        opts.push_back( {"center_agent", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.center_agent)} );
        opts.push_back( {"use_generated_assets", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.use_generated_assets)} );
        opts.push_back( {"use_monochrome_assets", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.use_monochrome_assets)} );
        opts.push_back( {"restrict_themes", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.restrict_themes)} );
        opts.push_back( {"use_backgrounds", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.use_backgrounds)} );
        opts.push_back( {"paint_vel_info", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.paint_vel_info)} );

        opts.push_back( {"distribution_mode", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.distribution_mode_enum)} );

        opts.push_back( {"use_sequential_levels", LIBENV_DTYPE_UINT8, 1,
                         (void*)(&base_opt.use_sequential_levels)} );

        opts.push_back( {"debug_mode", LIBENV_DTYPE_INT32, 1,
                         (void*)(&base_opt.debug_mode)} );

        return libenv_options{ opts.data(), int(opts.size()) };
    }

    std::array<char, MAX_STATE_SIZE>  BaseProcgenEnv::getState() {
        get_state(handle, 0, state.data(), MAX_STATE_SIZE);
        return state;
    }

    void BaseProcgenEnv::setState(std::array<char, MAX_STATE_SIZE> && _state){
        state = _state;
        set_state(handle, 0, state.data(), MAX_STATE_SIZE);
    }

    void BaseProcgenEnv::act(int32_t const& ac) {
        assert(action_ref[0]);
        *action_ref[0] = ac;
        libenv_act(handle);
    }

    std::tuple<cv::Mat, float, bool> BaseProcgenEnv::observe() {
        libenv_observe(handle);
        return { obs_data, reward_ref[0], first_ref[0]};
    }

    BaseProcgenEnv::~BaseProcgenEnv() {
        libenv_close(handle);
        delete[] action_ref;
        delete[] obs_ref;

    }

    cv::Mat BaseProcgenEnv::reset() {
        libenv_buffers buff{};
        buff.first = reinterpret_cast<uint8_t*>(first_ref.data());
        buff.rew = reinterpret_cast<float *>(reward_ref.data());

        if(not action_ref){
            action_ref = new int32_t*[1];
            action_ref[0] = new int32_t;
        }
        buff.ac = reinterpret_cast<void**>( action_ref );

        if(not obs_ref){
            obs_ref = new uint8_t*[1];
            obs_ref[0] = obs_data.data;
        }
        buff.ob = reinterpret_cast<void**>( obs_ref );

        libenv_set_buffers(handle, &buff);
        libenv_observe(handle);

        return obs_data;
    }

    ProcgenEnv::ProcgenEnv(BaseProcgenEnv::Option const& option):base(option) {

        m_ObservationSpace = makeBoxSpace<float>( 0, 255, { 64, 64, 3});
        m_ActionSpace = makeDiscreteSpace(15);

    }

    cv::Mat ProcgenEnv::reset() noexcept {
        if(resetData){
            auto _obs = *resetData;
            resetData = std::nullopt;
            return _obs;
        }

        return base.reset();
    }

    StepResponse<cv::Mat> ProcgenEnv::step(const int &action) noexcept {
        base.act(action);
        float reward;
        bool first;
        cv::Mat data;

        std::tie(data, reward, first) = base.observe();

        if(first)
            resetData = data;

        return {data, reward, first, {}};
    }

    void ProcgenEnv::render() {

        cv::Mat render_hires_buf = cv::Mat(RENDER_RES, RENDER_RES, CV_8UC4);

        Game *game = base._game();
        game->render_to_buf(render_hires_buf.data, RENDER_RES, RENDER_RES, true);

        cv::imshow("ProcgenEnv", render_hires_buf);
        cv::waitKey(100);
    }
}