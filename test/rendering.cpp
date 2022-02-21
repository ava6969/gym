//
// Created by dewe on 8/23/21.
//

#include "catch.hpp"
#include <GLFW/glfw3.h>

const GLint WIDTH = 720;
const GLint HEIGHT = 480;

TEST_CASE("Try Rendering"){

    glfwInit();

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    glViewport(0, 0, std::max(1, WIDTH), std::max(1, HEIGHT));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, std::max(1, screenWidth), 0, std::max(1, screenHeight), -1, 1);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT);


        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-25.0, -15.0, 0.0);
        glVertex3f(-25.5, 15.0, 0.0);
        glVertex3f(25, 15.0, 0.0);
        glVertex3f(25, -15, 0.0);
        glEnd();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
}