#include "app.h"

//
#include "utils/os_tools.h"

#include <nanogui/nanogui.h>
#include <iostream>

#include "manager/cleanup_manager.h"
#include "manager/startup_manager..h"

using namespace nanogui;

static FormHelper* gui = nullptr;

static int nanogui_initialize(int num, void* data)
{
    //
    if (is_sudo(1))
    {
        OS_PRINT("Run as sudoer!!!");
    }
    //
    OS_PRINT("This app built at %s", COMPILE_TIME_STR(app));
    //
    nanogui::init();
    //
    return 0;
}
REG_STARTUP_FUNC(nanogui_initialize, nanogui_initialize, NULL)

static int nanogui_deinitialize(int num, void* data)
{
    if (gui)
    {
        delete gui;
    }
    nanogui::shutdown();
    return 0;
}
REG_CLEANUP_FUNC(nanogui_deinitialize, nanogui_deinitialize, NULL)

void os_running_loop_impl()
{
    OS_LOGV("%s: Begin", __FILE__);
    enum test_enum
    {
        Item1 = 0,
        Item2,
        Item3
    };

    bool        bvar    = true;
    int         ivar    = 12345678;
    double      dvar    = 3.1415926;
    float       fvar    = (float)dvar;
    std::string strval  = "A string";
    std::string strval2 = "";
    test_enum   enumval = Item2;
    Color       colval(0.5f, 0.5f, 0.7f, 1.f);

    bool use_gl_4_1 = false;  // Set to true to create an OpenGL 4.1 context.

    ref<Screen> screen;

    if (use_gl_4_1)
    {
        // NanoGUI presents many options for you to utilize at your discretion.
        // See include/nanogui/screen.h for what all of these represent.
        screen = new Screen(Vector2i(500, 700),
                            "NanoGUI test [GL 4.1]",
                            /* resizable */ true,
                            /* fullscreen */ false,
                            /* depth_buffer */ true,
                            /* stencil_buffer */ true,
                            /* float_buffer */ false,
                            /* gl_major */ 4,
                            /* gl_minor */ 1);
    } else
    {
        screen = new Screen(Vector2i(500, 700), "NanoGUI test");
    }
    gui = new FormHelper(screen);

    bool enabled = true;

    ref<Window> window = gui->add_window(Vector2i(10, 10), "Form helper example");
    gui->add_group("Basic types");
    gui->add_variable("bool", bvar);
    gui->add_variable("string", strval);
    gui->add_variable("placeholder", strval2)->set_placeholder("placeholder");

    gui->add_group("Validating fields");
    gui->add_variable("int", ivar)->set_spinnable(true);
    gui->add_variable("float", fvar);
    gui->add_variable("double", dvar)->set_spinnable(true);

    gui->add_group("Complex types");
    gui->add_variable("Enumeration", enumval, enabled)->set_items({"Item 1", "Item 2", "Item 3"});
    gui->add_variable("Color", colval);

    gui->add_group("Other widgets");
    gui->add_button("A button", []() { std::cout << "Button pressed." << std::endl; });

    screen->set_visible(true);
    screen->perform_layout();
    window->center();

    nanogui::mainloop(-1);
    OS_LOGV("%s: End", __FILE__);
}

void os_stop_running_loop_impl(int code)
{
    OS_LOGV("%s", __FILE__);
    nanogui::leave();
}
