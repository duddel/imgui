// dear imgui: standalone example application for Android + OpenGL ES 3
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

static EGLDisplay           g_elgDisplay = EGL_NO_DISPLAY;
static EGLSurface           g_eglSurface = EGL_NO_SURFACE;
static EGLContext           g_eglContext = EGL_NO_CONTEXT;
static struct android_app*  g_App = NULL;
static bool                 g_initialized = false;
static char                 g_logTag[] = "ImguiExample";

void init(struct android_app *app)
{
    if (g_initialized)
        return;

    g_App = app;

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    g_elgDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (g_elgDisplay == EGL_NO_DISPLAY)
        __android_log_print(ANDROID_LOG_ERROR, g_logTag, "%s", "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");

    if (eglInitialize(g_elgDisplay, 0, 0) != EGL_TRUE)
        __android_log_print(ANDROID_LOG_ERROR, g_logTag, "%s", "eglInitialize(..) returned with an error");

    const EGLint eglAttribs[] = {
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE};

    EGLint numConfigs = 0;
    if (eglChooseConfig(g_elgDisplay, eglAttribs, nullptr, 0, &numConfigs) != EGL_TRUE)
        __android_log_print(ANDROID_LOG_ERROR, g_logTag, "%s", "eglChooseConfig(..) returned with an error");

    if (numConfigs == 0)
        __android_log_print(ANDROID_LOG_ERROR, g_logTag, "%s", "eglChooseConfig(..) returned 0 matching configs");

    // Get the (first) matching config
    EGLConfig config;
    eglChooseConfig(g_elgDisplay, eglAttribs, &config, 1, &numConfigs);
    EGLint format;
    eglGetConfigAttrib(g_elgDisplay, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(g_App->window, 0, 0, format);

    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    g_eglContext = eglCreateContext(g_elgDisplay, config, EGL_NO_CONTEXT, contextAttribs);

    if (g_eglContext == EGL_NO_CONTEXT)
        __android_log_print(ANDROID_LOG_ERROR, g_logTag, "%s", "eglCreateContext(..) returned EGL_NO_CONTEXT");

    g_eglSurface = eglCreateWindowSurface(g_elgDisplay, config, g_App->window, NULL);
    eglMakeCurrent(g_elgDisplay, g_eglSurface, g_eglSurface, g_eglContext);

    // Dear Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(app);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    // Arbitrary scale-up
    // todo: Put some effort into DPI awareness
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    g_initialized = true;
}

void tick()
{
    // Our state (Dear Imgui)
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if (g_elgDisplay != EGL_NO_DISPLAY)
    {
        ImGuiIO& io = ImGui::GetIO();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(g_elgDisplay, g_eglSurface);
    }
}

void shutdown()
{
    if (!g_initialized)
        return;

    // Cleanup (Dear Imgui)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (g_elgDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(g_elgDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(g_elgDisplay, g_eglContext);

        if (g_eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(g_elgDisplay, g_eglSurface);

        eglTerminate(g_elgDisplay);
    }

    g_elgDisplay = EGL_NO_DISPLAY;
    g_eglContext = EGL_NO_CONTEXT;
    g_eglSurface = EGL_NO_SURFACE;

    g_initialized = false;
}

static void handleAppCmd(struct android_app *app, int32_t appCmd)
{
    switch (appCmd)
    {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        init(app);
        break;
    case APP_CMD_TERM_WINDOW:
        shutdown();
        break;
    case APP_CMD_GAINED_FOCUS:
        break;
    case APP_CMD_LOST_FOCUS:
        break;
    }
}

void android_main(struct android_app *app)
{
    app->onAppCmd = handleAppCmd;

    while (true)
    {
        int ident;
        int outEvents;
        struct android_poll_source *outData;

        // Poll all events. If the app is not visible, this loop blocks until g_initialized == true.
        while ((ident = ALooper_pollAll(g_initialized ? 0 : -1, NULL, &outEvents, (void **)&outData)) >= 0)
        {
            // Process one event
            if (outData != NULL)
                outData->process(app, outData);

            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0)
            {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!g_initialized)
                    shutdown();

                return;
            }
        }

        // Initiate a new frame
        tick();
    }
}
