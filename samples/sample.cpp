// sample.cpp - public domain
// authored from 2012-2013 by Adrien Herubel
// modified 2014 by Luca Deltodesco

#include <cmath>
#include <string>
#include <iostream>

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/imguiRenderGL3.h>

int main (int argc, char* argv[])
{
    int width  = 1024;
    int height = 768;

    // Initialise GLFW
    if(!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, 1);

    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(width, height, "imgui sample", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Enable vertical sync (on cards that support it)
    glfwSwapInterval(1);

    // Init UI
    if (!imguiRenderGLInit("DroidSans.ttf"))
    {
        std::cerr << "Could not init GUI renderer" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    double currentGlfwScroll = 0;
    glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y, void* userData)
    {
        *(double*)(userData) += y;
    }, &currentGlfwScroll);

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods, void* userData)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
    }, nullptr);

    glClearColor(0.8f, 0.8f, 0.8f, 1.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // imgui states
    bool checked1 = false;
    bool checked2 = false;
    bool checked3 = true;
    bool checked4 = false;
    float value1 = 50.f;
    float value2 = 30.f;
    int scrollArea1 = 0;
    int scrollArea2 = 0;
    double glfwScroll = 0;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Mouse states
        int mScroll = (int)(glfwScroll - currentGlfwScroll);
        glfwScroll = currentGlfwScroll;

        double mouseX;
        double mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseY = height - mouseY;

        unsigned char mouseButton = 0;
        int leftButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        int rightButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        int middleButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
        int toggle = 0;
        if (leftButton == GLFW_PRESS)
        {
            mouseButton |= IMGUI_MBUT_LEFT;
        }

        imguiBeginFrame(mouseX, mouseY, mouseButton, mScroll);

        imguiBeginScrollArea("Scroll area", 10, 10, width / 5, height - 20, &scrollArea1);
        imguiSeparatorLine();
        imguiSeparator();

        imguiButton("Button");
        imguiButton("Disabled button", false);
        imguiItem("Item");
        imguiItem("Disabled item", false);
        toggle = imguiCheck("Checkbox", checked1);
        if (toggle)
        {
            checked1 = !checked1;
        }

        toggle = imguiCheck("Disabled checkbox", checked2, false);
        if (toggle)
        {
            checked2 = !checked2;
        }

        toggle = imguiCollapse("Collapse", "subtext", checked3);
        if (checked3)
        {
            imguiIndent();
            imguiLabel("Collapsible element");
            imguiUnindent();
        }
        if (toggle)
        {
            checked3 = !checked3;
        }

        toggle = imguiCollapse("Disabled collapse", "subtext", checked4, false);
        if (toggle)
        {
            checked4 = !checked4;
        }

        imguiLabel("Label");
        imguiValue("Value");
        imguiSlider("Slider", &value1, 0.f, 100.f, 1.f);
        imguiSlider("Disabled slider", &value2, 0.f, 100.f, 1.f, false);
        imguiIndent();
        imguiLabel("Indented");
        imguiUnindent();
        imguiLabel("Unindented");

        imguiEndScrollArea();

        imguiBeginScrollArea("Scroll area", 20 + width / 5, 100, width / 5, 510, &scrollArea2);
        imguiSeparatorLine();
        imguiSeparator();
        for (int i = 0; i < 100; ++i)
        {
            imguiLabel("A wall of text");
        }

        imguiEndScrollArea();
        imguiEndFrame();

        imguiDrawText(30 + width / 5 * 2,       height - 20, IMGUI_ALIGN_LEFT,   "Free text", imguiRGBA(32, 192,  32, 192));
        imguiDrawText(30 + width / 5 * 2 + 100, height - 40, IMGUI_ALIGN_RIGHT,  "Free text", imguiRGBA(32,  32, 192, 192));
        imguiDrawText(30 + width / 5 * 2 + 50,  height - 60, IMGUI_ALIGN_CENTER, "Free text", imguiRGBA(192, 32,  32, 192));

        imguiDrawLine(30 + width / 5 * 2, height - 80,  30 + width / 5 * 2 + 100, height - 60,  1.f, imguiRGBA(32, 192,  32, 192));
        imguiDrawLine(30 + width / 5 * 2, height - 100, 30 + width / 5 * 2 + 100, height - 80,  2.f, imguiRGBA(32,  32, 192, 192));
        imguiDrawLine(30 + width / 5 * 2, height - 120, 30 + width / 5 * 2 + 100, height - 100, 3.f, imguiRGBA(192, 32,  32, 192));

        imguiDrawRoundedRect(30 + width / 5 * 2, height - 240, 100, 100, 5.f,  imguiRGBA(32, 192,  32, 192));
        imguiDrawRoundedRect(30 + width / 5 * 2, height - 350, 100, 100, 10.f, imguiRGBA(32,  32, 192, 192));
        imguiDrawRoundedRect(30 + width / 5 * 2, height - 470, 100, 100, 20.f, imguiRGBA(192, 32,  32, 192));

        imguiDrawRect(30 + width / 5 * 2, height - 590, 100, 100, imguiRGBA(32, 192, 32, 192));
        imguiDrawRect(30 + width / 5 * 2, height - 710, 100, 100, imguiRGBA(32, 32, 192, 192));
        imguiDrawRect(30 + width / 5 * 2, height - 830, 100, 100, imguiRGBA(192, 32, 32,192));

        imguiRenderGLDraw(width, height);

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Clean UI
    imguiRenderGLDestroy();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

