//
// Created by dewe on 6/16/22.
//

#include "car_racing.h"


int main(){

    gym::CarRacing race(false);

    static std::vector<float> a{0, 0, 0};
    static bool restart = false;
    race.initRenderer();

    auto key_callback = [](GLFWwindow* window, int k, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS){
            if (k==0xff0d) restart = true;
            if (k==GLFW_KEY_LEFT)  a[0] = -1.0;
            if (k==GLFW_KEY_RIGHT) a[0] = +1.0;
            if (k==GLFW_KEY_UP)    a[1] = +1.0;
            if (k==GLFW_KEY_DOWN)  a[2] = +0.8;   // set 1.0 for wheels to block to zero rotation
        }else if(action == GLFW_RELEASE){
            if (k==GLFW_KEY_LEFT  and a[0]==-1.0) a[0] = 0;
            if (k==GLFW_KEY_RIGHT and a[0]==+1.0) a[0] = 0;
            if (k==GLFW_KEY_UP)    a[1] = 0;
            if (k==GLFW_KEY_DOWN)  a[2] = 0;
        }
    };

    glfwSetKeyCallback(race.win(), key_callback);

    while ( !glfwWindowShouldClose(race.win()) ){
        auto obs = race.reset();
        float total_reward = 0.0;
        int steps = 0;
        restart = false;
        while(true){
            auto resp  = race.step(a);
            total_reward += resp.reward;
            if(steps % 200 == 0 or resp.done){
                printf("\naction %+0.2f %+0.2f %+0.2f \n", a[0], a[1], a[2]);
                printf("step %i total_reward %+0.2f\n", steps, total_reward);
            }
            cv::imshow("chart", resp.observation);
            cv::waitKey(1);
            steps++;
            race.render();
            if( resp.done or restart)
                break;
        }
    }


}