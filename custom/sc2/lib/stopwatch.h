#pragma once
//
// Created by dewe on 7/16/22.
//
#include "cmath"
#include "string"
#include "boost/algorithm/string.hpp"
#include "sstream"
#include "iostream"
#include "stack"
#include <typeinfo>
#include "variant"
#include "chrono"


namespace sc2{

    template<class Class>
    void with(Class && c, std::function<void()> const& _func){
        c.enter();
        _func();
        c.exit();
    };

    template<class Class>
    void with(Class * c, std::function<void()> const& _func){
        c->enter();
        _func();
        c->exit();
    };

    template<class Class, class Ret>
    Ret with(Class && c, std::function<Ret()> const& _func){
        c.enter();
        Ret ret = _func();
        c.exit();
        return ret;
    };


    class Stat {

    public:
        Stat() {
            reset();
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
            sum_sq += std::pow<float>(val, 2);
        }

        [[nodiscard]] float Sum() const { return sum; }
        [[nodiscard]] float Min() const { return min; }
        [[nodiscard]] float Max() const { return max; }
        [[nodiscard]] float Num() const { return num; }

        [[nodiscard]] float avg() const { num == 0 ? 0 : sum / num; }

        [[nodiscard]] float dev() const {
            if (num == 0)
                return 0.0;
            return sqrt(std::max<double>(0.0, sum_sq / num - pow(sum / num, 2)));
        }

        void merge(Stat const &other) {
            num += other.num;
            min = std::min(min, other.min);
            max = std::max(max, other.max);
            sum += other.sum;
            sum_sq += other.sum_sq;
        }

        Stat(float summation, float average,
             float standard_deviation, float minimum, float maximum, float number) {
            if (number > 0) {
                num = number;
                min = minimum;
                max = maximum;
                sum = summation;
                sum_sq = number * (std::pow(standard_deviation, 2) + std::pow(average, 2));
            }
        }

        Stat(std::vector<float> const& parts):
        Stat(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]){}

        static Stat parse(std::string const& s){
            if( s == "num=0"){
                return {};
            }

            std::vector<float> parts;
            std::vector<std::string> result;
            boost::split(result, s, boost::is_any_of(", "));
            for( auto const& p: result){
                std::vector<std::string> sub_result;
                boost::split(sub_result, p, boost::is_any_of(":"));
                parts.emplace_back(std::stof(sub_result[1]));
            }
            return {parts};
        }

        inline auto str() const {
            std::stringstream ss;
            ss << *this;
            return ss.str();
        }

        friend std::ostream & operator<<(std::ostream& os, Stat const& s){
            if(s.num == 0)
                os << "num=0";
            else{
                os << "sum: " << s.sum
                << ", avg: " << s.avg()
                << ", dev: " << s.dev()
                << ", min: " << s.min
                << ", max: " << s.max
                << ", num: " << s.num << "\n";
            }
            return os;
        }

    private:
        float num =0, min=0, max=0, sum=0, sum_sq=0;

    };

    class StopWatch;

    struct StopWatchContext{
        StopWatchContext()=default;
        StopWatchContext(StopWatch& stopwatch, std::string const& name);

        virtual void enter(){
            start = std::chrono::high_resolution_clock::now();
        }

        virtual void exit();

    protected:
        StopWatch& sw;
        decltype(std::chrono::high_resolution_clock::now()) start;
    };

    struct TracingStopWatchContext : StopWatchContext{

        TracingStopWatchContext(StopWatch& stopwatch, std::string const& name);

        void enter() override;

        void exit() override;

    private:
        void log(std::string const& s){
            std::cerr << s << "\n";
        }
    };

    struct FakeStopWatchContext : StopWatchContext{

        void enter() override {}

        void exit() override {}
    };

    template<typename Callable>
    using return_type_of_t =
            typename decltype(std::function{std::declval<Callable>()})::result_type;

    static auto fake_context = std::make_shared<FakeStopWatchContext>();

    class StopWatch{

        explicit StopWatch(bool enabled=true, bool trace=false);
        inline static std::unique_ptr<StopWatch> inst;

    public:
        static StopWatch* instance();

        auto  cur_stack( ){
            std::stringstream ss;
            auto local_copy = local;
            while (not local_copy.empty()){
                ss << local_copy.top() << ",";
                local_copy.pop();
            }
            return ss.str();
        }

        inline auto operator()(std::string const& name) { return _factory(name); }


        template<class NameFunc, class ... Args>
        static auto _stopwatch(std::string const& name,
                        NameFunc const& func,
                        StopWatch& _sw,
                        Args ... args){
            return with( _sw(name),
                         [&](){ return func( std::forward<Args>(args) ... ); } );
        }

        template<class NameFunc, class ... Args>
        std::function< return_type_of_t<NameFunc>(Args ...) >
                decorate( NameFunc const& name_or_func){
            if( std::getenv("SC2_NO_STOPWATCH")){
                return name_or_func;
            }

            auto decorator = [this](std::string const& name, NameFunc const& func){
                return [&name, this, &func](){
                    return with( (*this)(name), [&func](Args ... args){
                       return func( std::forward(args) ... );
                    });
                };
            };

            return decorator( boost::typeindex::type_id<NameFunc>().name(), name_or_func );
        }

        inline void push( std::string const& name ){ local.push(name); }
        std::string pop(  ){
            auto ret = this->cur_stack();
            local.pop();
            return ret;
        }

        void clear() { _times.clear(); }

        void add( std::string const& name, float duration) { _times[name].add(duration); }
        auto operator[](std::string const& name) const noexcept { return _times.at(name); }

        inline void disable()  { _factory = [](std::string const& ) { return std::static_pointer_cast<StopWatchContext>(fake_context); };}
        inline void enable()  { _factory = [this](std::string const& n ) { return std::make_shared<StopWatchContext>(this, n) ; };}
        inline void trace()  { _factory = [this](std::string const& n ) { return std::make_shared<TracingStopWatchContext>(this, n) ; };}
        inline void custom(std::function< std::shared_ptr<StopWatchContext>(std::string const&) > const& factory ) { _factory = factory; }

        static StopWatch parse(std::string const& s);

        std::string str(float threshold);
        std::string str() { return str(0.1); }

    private:
        std::unordered_map<std::string, Stat> _times;
        std::stack<std::string> local;
        std::function< std::shared_ptr<StopWatchContext>(std::string const&) > _factory;
    };
}
