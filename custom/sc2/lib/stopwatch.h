#pragma once
//
// Created by dewe on 7/16/22.
//
#include "cmath"
#include "string"
#include "sstream"
#include "iostream"
#include "queue"
#include <typeinfo>
#include "variant"
#include "optional"
#include "chrono"
#include "util.h"


namespace sc2{

    class Stat {

    public:
        Stat() {
            this->reset();
        }

        void reset() {
            num = 0, min = 1000000000.f, max = 0, sum = 0, sum_sq = 0;
        }

        void add(float val) {
            num++;
            if (min > val)
                min = val;
            if (max < val)
                max = val;
            sum += val;
            sum_sq += (val*val);
        }

        [[nodiscard]] inline float Sum() const { return sum; }
        [[nodiscard]] inline float Min() const { return min; }
        [[nodiscard]] inline float Max() const { return max; }
        [[nodiscard]] inline float Num() const { return num; }
        [[nodiscard]] inline float avg() const { return num == 0 ? 0 : (sum / num); }

        [[nodiscard]] inline float dev() const {
            if (num == 0) return 0.0;
            return sqrt(std::max<double>(0.0, sum_sq / num - std::pow(sum / num, 2)));
        }

        void merge(Stat const &other);

        Stat(float summation, float average,
             float standard_deviation, float minimum, float maximum, float number);

        explicit Stat(std::vector<float> const& parts):
        Stat(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]){}

        static Stat parse(std::string const& s);

        inline auto str() const {
            std::stringstream ss;
            ss << *this;
            return ss.str();
        }

        friend std::ostream & operator<<(std::ostream& os, Stat const& s);

    private:
        float num =0, min=0, max=0, sum=0, sum_sq=0;

    };

    class StopWatch;

    struct StopWatchContext{
        StopWatchContext()=default;
        StopWatchContext( StopWatch* stopwatch, std::string const& name);

        inline virtual void enter(){
            start = std::chrono::high_resolution_clock::now();
        }

        virtual void exit();

    protected:
        StopWatch* m_sw{nullptr};
        std::optional<TimePoint> start;
    };

    struct TracingStopWatchContext : StopWatchContext{
        TracingStopWatchContext()=default;
        TracingStopWatchContext(StopWatch* stopwatch, std::string const& name);

        void enter() override;

        void exit() override;

    private:
        inline static void log(std::string const& s){
            std::cerr << s << "\n";
        }
    };

    struct FakeStopWatchContext : StopWatchContext{
        FakeStopWatchContext()=default;

        void enter() override {}

        void exit() override {}
    };

    const std::shared_ptr<StopWatchContext> fake_context = std::make_shared<FakeStopWatchContext>();

    struct With;

    class StopWatch{
    public:
        explicit StopWatch(bool enabled=true, bool trace=false);

        std::string  cur_stack( );

        inline auto operator()(std::string const& name) {
            return _factory(name);
        }

        template<class Ret=void>
        std::function< std::function<Ret()> ( std::function<Ret()> )> decorate( std::string const& name){
            if(auto res = getenv("SC2_NO_STOPWATCH"); res and  std::string(res) == "1" ){
                return [](std::function<Ret()> const& func) -> std::function<Ret()> { return func; };
            }

            return [this, name](std::function<Ret()> const& func) -> std::function<Ret()> {
                auto decorator = [this](std::string const& name, std::function<Ret()> const& func) -> std::function<Ret()> {
                    auto _stopwatch = [this, name, &func]() -> Ret {
                        With w{ (*this)(name) };
                        return func();
                    };
                    return _stopwatch;
                };
                return decorator(name, func);
            };
        }

        inline void push( std::string const& name ){
            local.push_back(name);
        }

        std::string pop(  ){
            auto ret = this->cur_stack();
            local.pop_back();
            return ret;
        }

        void clear() { _times.clear(); }

        void add( std::string const& name, float duration) { _times[name].add(duration); }
        auto operator[](std::string const& name) const noexcept { return _times.at(name); }

        inline void disable()  { _factory = [](std::string const& ) { return std::static_pointer_cast<StopWatchContext>(fake_context); };}
        inline void enable()  {
            _factory = [this](std::string const& n ) {
                return std::make_shared<StopWatchContext>(this, n) ;
            };
        }
        inline void trace()  { _factory = [this](std::string const& n ) { return std::make_shared<TracingStopWatchContext>(this, n) ; };}
        inline void custom(std::function< std::shared_ptr<StopWatchContext>(std::string const&) > const& factory ) { _factory = factory; }

        static StopWatch parse(std::string const& s);

        std::string str(float threshold);
        std::string str() { return str(0.1); }

    private:
        std::unordered_map<std::string, Stat> _times;
        std::vector<std::string> local;
        std::function< std::shared_ptr<StopWatchContext>(std::string const&) > _factory;
    };

    struct GlobalStopWatch{
        inline static StopWatch instance{true};
    };

    static StopWatch& sw = GlobalStopWatch::instance;

}
