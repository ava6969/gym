#include "custom/box_world/random_box_world.h"

int main() {
    using namespace gym;
    gym::RandomBoxWorld::Option opt;
    opt.verbose= true;
    gym::RandomBoxWorld env(opt);


    for(int i = 0; i < 5; i++) {
        auto s = env.reset();
        auto ret = 0.0f;

        while (true) {
            env.render(gym::RenderType::HUMAN);

            char key{};
            std::cin.get(key);
            int action = -1;

            switch (key) {
                case 'd': // Left
                    action = 3;
                    break;
                case 'w': // Up
                    action = 0;
                    break;
                case 'a': // Right
                    action = 2;
                    break;
                case 's': // Down
                    action = 1;
                    break;
            }

            if (action > -1) {
                auto resp = env.step(action);
                ret += resp.reward;
                if (resp.done)
                    break;
                s = resp.observation;
            }

        }

        printf("ret: %f", ret);
    }
    return 0;
}