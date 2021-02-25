// dear imgui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (imgui_impl_opengl3)

// Implemented features:
//  [x] Basic mouse input via touch
//  [x] Open soft keyboard if io.WantTextInput and perform proper keyboard input
//  [x] Handle Unicode characters
//  [ ] Handle physical mouse input

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2020-09-13: Support for Unicode characters
//  2020-08-31: On-screen and physical keyboard input (ASCII characters only)
//  2020-03-02: basic draft, touch input

#include "imgui.h"
#include "imgui_impl_android.h"
#include <ctime>
#include <map>
#include <queue>

// Android
#include <android/native_window.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/log.h>

static double                                   g_Time = 0.0;
static ANativeWindow*                           g_Window;
static char                                     g_LogTag[] = "ImguiExample";
static std::map<int32_t, std::queue<int32_t>>   g_KeyEventQueues;

int32_t ImGui_ImplAndroid_HandleInputEvent(AInputEvent* inputEvent)
{
    ImGuiIO& io = ImGui::GetIO();
    int32_t event_type = AInputEvent_getType(inputEvent);
    switch (event_type)
    {
    case AINPUT_EVENT_TYPE_KEY:
    {
        int32_t event_key_code = AKeyEvent_getKeyCode(inputEvent);
        int32_t event_action = AKeyEvent_getAction(inputEvent);
        int32_t event_meta_state = AKeyEvent_getMetaState(inputEvent);

        io.KeyCtrl = ((event_meta_state & AMETA_CTRL_ON) != 0);
        io.KeyShift = ((event_meta_state & AMETA_SHIFT_ON) != 0);
        io.KeyAlt = ((event_meta_state & AMETA_ALT_ON) != 0);

        switch (event_action)
        {
        // todo: AKEY_EVENT_ACTION_DOWN and AKEY_EVENT_ACTION_UP occur at once
        // as soon as a touch pointer goes up from a key. We use a simple key event queue
        // and process one event per key per ImGui frame in ImGui_ImplAndroid_NewFrame().
        // ...or consider ImGui IO queue, if suitable: https://github.com/ocornut/imgui/issues/2787
        case AKEY_EVENT_ACTION_DOWN:
        case AKEY_EVENT_ACTION_UP:
            g_KeyEventQueues[event_key_code].push(event_action);
            break;
        default:
            break;
        }
        break;
    }
    case AINPUT_EVENT_TYPE_MOTION:
    {
        int32_t event_action = AMotionEvent_getAction(inputEvent);
        int32_t event_pointer_index = (event_action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        int32_t event_pointer_id = AMotionEvent_getPointerId(inputEvent, event_pointer_index);
        event_action &= AMOTION_EVENT_ACTION_MASK;
        switch (event_action)
        {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_UP:
            io.MouseDown[0] = (event_action == AMOTION_EVENT_ACTION_DOWN) ? true : false;
            // intended fallthrough...
        case AMOTION_EVENT_ACTION_MOVE:
            io.MousePos = ImVec2(
                AMotionEvent_getX(inputEvent, event_pointer_index),
                AMotionEvent_getY(inputEvent, event_pointer_index));
            break;
        default:
            break;
        }
    }
        return 1;
    default:
        break;
    }

    return 0;
}

bool ImGui_ImplAndroid_Init(ANativeWindow* window)
{
    g_Window = window;
    g_Time = 0.0;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    // todo: any reasonable io.BackendFlags?
    io.BackendPlatformName = "imgui_impl_android";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = AKEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = AKEYCODE_DPAD_LEFT;   // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_RightArrow] = AKEYCODE_DPAD_RIGHT; // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_UpArrow] = AKEYCODE_DPAD_UP;       // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_DownArrow] = AKEYCODE_DPAD_DOWN;   // also covers physical keyboard arrow key
    io.KeyMap[ImGuiKey_PageUp] = AKEYCODE_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = AKEYCODE_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = AKEYCODE_MOVE_HOME;
    io.KeyMap[ImGuiKey_End] = AKEYCODE_MOVE_END;
    io.KeyMap[ImGuiKey_Insert] = AKEYCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = AKEYCODE_FORWARD_DEL;
    io.KeyMap[ImGuiKey_Backspace] = AKEYCODE_DEL;
    io.KeyMap[ImGuiKey_Space] = AKEYCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = AKEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape] = AKEYCODE_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = AKEYCODE_NUMPAD_ENTER;
    io.KeyMap[ImGuiKey_A] = AKEYCODE_A;
    io.KeyMap[ImGuiKey_C] = AKEYCODE_C;
    io.KeyMap[ImGuiKey_V] = AKEYCODE_V;
    io.KeyMap[ImGuiKey_X] = AKEYCODE_X;
    io.KeyMap[ImGuiKey_Y] = AKEYCODE_Y;
    io.KeyMap[ImGuiKey_Z] = AKEYCODE_Z;

    return true;
}

void ImGui_ImplAndroid_Shutdown()
{
}

void ImGui_ImplAndroid_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Process queued key events
    for (auto& key_queue : g_KeyEventQueues)
    {
        if (key_queue.second.empty())
            continue;
        io.KeysDown[key_queue.first] = (key_queue.second.front() == AKEY_EVENT_ACTION_DOWN);
        key_queue.second.pop();
    }

    // Setup display size (every frame to accommodate for window resizing)
    int32_t window_width = ANativeWindow_getWidth(g_Window);
    int32_t window_height = ANativeWindow_getHeight(g_Window);
    int display_width = window_width;
    int display_height = window_height;

    io.DisplaySize = ImVec2((float)window_width, (float)window_height);
    if (window_width > 0 && window_height > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_width / window_width, (float)display_height / window_height);

    // Setup time step
    struct timespec current_timespec;
    clock_gettime(CLOCK_MONOTONIC, &current_timespec);
    double current_time = (double)(current_timespec.tv_sec) + (current_timespec.tv_nsec / 1000000000.0);
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
    g_Time = current_time;
}
