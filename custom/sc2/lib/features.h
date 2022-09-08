#pragma once
//
// Created by dewe on 7/15/22.
//

#include "s2clientprotocol/raw.pb.h"
#include "s2clientprotocol/sc2api.pb.h"
#include "optional"
#include "actions.h"
#include "variant"
#include "point.h"


namespace sc2 {

    const auto EPSILON = 1e-5f;

    enum class FeatureType{
        SCALAR = 1,
        CATEGORICAL = 2
    };

    enum class PlayerRelative
    {
        NONE = 0,
        SELF = 1,
        ALLY = 2,
        NEUTRAL = 3,
        ENEMY = 4,
    };

    enum class  Visibility {
        HIDDEN = 0,
        SEEN = 1,
        VISIBLE = 2
    };

    enum class Effects {
        none = 0,
        PsiStorm = 1,
        GuardianShield = 2,
        TemporalFieldGrowing = 3,
        TemporalField = 4,
        ThermalLance = 5,
        ScannerSweep = 6,
        NukeDot = 7,
        LiberatorDefenderZoneSetup = 8,
        LiberatorDefenderZone = 9,
        BlindingCloud = 10,
        CorrosiveBile = 11,
        LurkerSpines = 12,
    };

    enum class ScoreCumulative {
        score = 0,
        idle_production_time = 1,
        idle_worker_time = 2,
        total_value_units = 3,
        total_value_structures = 4,
        killed_value_units = 5,
        killed_value_structures = 6,
        collected_minerals = 7,
        collected_vespene = 8,
        collection_rate_minerals = 9,
        collection_rate_vespene = 10,
        spent_minerals = 11,
        spent_vespene = 12
    };

    enum class ScoreByCategory{
        food_used = 0,
        killed_minerals = 1,
        killed_vespene = 2,
        lost_minerals = 3,
        lost_vespene = 4,
        friendly_fire_minerals = 5,
        friendly_fire_vespene = 6,
        used_minerals = 7,
        used_vespene = 8,
        total_used_minerals = 9,
        total_used_vespene = 10
    };

    enum class ScoreCategories{
        none = 0,
        army = 1,
        economy = 2,
        technology = 3,
        upgrade = 4
    };

    enum class ScoreByVital
    {
        total_damage_dealt = 0,
        total_damage_taken = 1,
        total_healed = 2
    };

    enum class ScoreVitals{
        life = 0,
        shields = 1,
        energy = 2
    };

    enum class Player{
        player_id = 0,
        minerals = 1,
        vespene = 2,
        food_used = 3,
        food_cap = 4,
        food_army = 5,
        food_workers = 6,
        idle_worker_count = 7,
        army_count = 8,
        warp_gate_count = 9,
        larva_count = 10
    };

    enum class UnitLayer {
        unit_type = 0,
        player_relative = 1,
        health = 2,
        shields = 3,
        energy = 4,
        transport_slots_taken = 5,
        build_progress = 6
    };

    enum class UnitCounts{
        unit_type = 0,
        count = 1
    };

    enum class FeatureUnit{
        unit_type = 0,
        alliance = 1,
        health = 2,
        shield = 3,
        energy = 4,
        cargo_space_taken = 5,
        build_progress = 6,
        health_ratio = 7,
        shield_ratio = 8,
        energy_ratio = 9,
        display_type = 10,
        owner = 11,
        x = 12,
        y = 13,
        facing = 14,
        radius = 15,
        cloak = 16,
        is_selected = 17,
        is_blip = 18,
        is_powered = 19,
        mineral_contents = 20,
        vespene_contents = 21,
        cargo_space_max = 22,
        assigned_harvesters = 23,
        ideal_harvesters = 24,
        weapon_cooldown = 25,
        order_length = 26,
        order_id_0 = 27,
        order_id_1 = 28,
        tag = 29,
        hallucination = 30,
        buff_id_0 = 31,
        buff_id_1 = 32,
        addon_unit_type = 33,
        active = 34,
        is_on_screen = 35,
        order_progress_0 = 36,
        order_progress_1 = 37,
        order_id_2 = 38,
        order_id_3 = 39,
        is_in_cargo = 40,
        buff_duration_remain = 41,
        buff_duration_max = 42,
        attack_upgrade_level = 43,
        armor_upgrade_level = 44,
        shield_upgrade_level = 45
    };

    enum class EffectPos{
        effect = 0,
        alliance = 1,
        owner = 2,
        radius = 3,
        x = 4,
        y = 5
    };

    enum class Radar{
        x = 0,
        y = 1,
        radius = 2
    };

    enum class ProductionQueue{
        ability_id = 0,
        build_progress = 1
    };

    using Size = std::variant< std::pair<int, int>, int>;

    class Dimensions{
    private:
        Point _screen, _minimap;
    public:
        Dimensions()=default;

        inline auto screen() const { return _screen; }
        inline Dimensions& screen( int di )  { _screen = {di, di}; return *this; }
        inline Dimensions& screen( SC2APIProtocol::Size2DI const& di )  { _screen = di; return *this; }

        inline auto minimap() const { return _minimap; }
        inline Dimensions& minimap( int di )  { _minimap = {di, di};  return *this; }
        inline Dimensions& minimap( SC2APIProtocol::Size2DI const& di )  { _screen = di; return *this; }

        bool operator==(Dimensions const&) const = default;
        friend std::ostream& operator<<( std::ostream& os, Dimensions const& dim){
            os << "Dimensions(screen=" << dim._screen << ", minimap=" << dim._minimap << ")";
            return os;
        }

    };

    struct AgentInterfaceFormat{
        struct Option{
            std::optional<Dimensions> feature_dimensions{}, rgb_dimensions{};
            std::optional<std::variant<Point, int>> raw_resolution{};
            std::optional<ActionSpace> action_space{};
            std::optional<int> camera_width_world_units{};
            bool use_feature_units{false}, use_raw_units{false}, use_raw_actions{false};
            int max_raw_actions{512}, max_selected_units{30};
            bool use_unit_counts{false}, use_camera_position{false}, show_cloaked{false}, show_burrowed_shadows{false},
            show_placeholders{false}, hide_specific_actions{true};
            std::optional< std::function<int()> > action_delay_fn{};
            bool send_observation_proto{false}, crop_to_playable_area{false}, raw_crop_to_playable_area{false},
            allow_cheating_layers{false}, add_cargo_to_units{false};
        } opt;

    private:
        Dimensions m_action_dimensions;
    public:
        AgentInterfaceFormat()=default;
        explicit AgentInterfaceFormat(Option  );
        inline auto use_feature_units() const { return opt.use_feature_units; }
        inline auto use_unit_counts() const { return opt.use_unit_counts; }
        inline auto use_raw_units() const { return opt.use_raw_units; }
        inline auto use_raw_actions() const { return opt.use_raw_actions; }
        inline auto use_camera_position() const { return opt.use_camera_position; }
        inline auto send_observation_proto() const { return opt.send_observation_proto; }
        inline auto show_cloaked() const { return opt.show_cloaked; }
        inline auto show_burrowed_shadows() const { return opt.show_burrowed_shadows; }
        inline auto show_placeholders() const { return opt.show_placeholders; }
        inline auto raw_crop_to_playable_area() const { return opt.raw_crop_to_playable_area; }
        inline auto has_feature_dimensions() const { return opt.feature_dimensions.has_value(); }
        inline auto feature_dimensions() const { return *opt.feature_dimensions; }
        inline auto* feature_dimensions()  { return &(opt.feature_dimensions.value()); }
        inline auto camera_width_world_units() const { return *opt.camera_width_world_units; }
        inline auto crop_to_playable_area() const { return opt.crop_to_playable_area; }
        inline auto allow_cheating_layers() const { return opt.allow_cheating_layers; }
        inline auto has_rgb_dimensions() const { return opt.rgb_dimensions.has_value(); }
        inline auto rgb_dimensions() const { return *opt.rgb_dimensions; }
        inline auto action_dimensions() const { return m_action_dimensions; }
        inline auto max_selected_units() const { return opt.max_selected_units; }
        inline auto* rgb_dimensions()  { return &opt.rgb_dimensions.value(); }

        template<typename T>
        inline auto& raw_resolution() { return (std::get<Point>(opt.raw_resolution.value())); }
        inline auto raw_resolution() { return opt.raw_resolution; }
    };

    struct Features{

        Features(std::optional<AgentInterfaceFormat> const& agent_interface_format=std::nullopt,
                 std::optional<SC2APIProtocol::Size2DI> const& map_size=std::nullopt,
                 std::optional<std::unordered_map<int, SC2APIProtocol::Race>> const& requested_races=std::nullopt,
                 std::string const& map_name="unknown");

    private:
        AgentInterfaceFormat m_agentInterfaceFormat;
        SC2APIProtocol::Size2DI m_mapSize;
        std::string m_mapName;
        bool m_sendObservationProto;
        bool m_raw;
        ValidActions m_validFunctions;
        std::vector<std::string> m_rawTags;
        std::optional<std::unordered_map<int, SC2APIProtocol::Race>> m_requested_races;
        void initCamera(Dimensions* ,
                        std::optional<SC2APIProtocol::Size2DI > const&, int,
                                std::optional<std::variant<Point, int>> const&  );
    };

    ValidActions initValidFunctions(Dimensions const&  );

    ValidActions initValidRawFunctions(std::optional<std::variant<Point, int>> const& raw_resolution,
                                       int max_selected_units);

    Features fromGameInfo(SC2APIProtocol::ResponseGameInfo const& game_info,
                          AgentInterfaceFormat const& agent,
                          std::string mapName="");


};
