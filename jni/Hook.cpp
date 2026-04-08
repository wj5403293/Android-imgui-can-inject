#include <thread>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <imgui/android_native_app_glue.h>

#include "Dobby/dobby.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_android.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Imgui/字体.h"

#define LOG_TAG    "myinject"
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


static float ScreenWidth = 700.0, ScreenHeight = 400.0; //窗口大小
static float fontsize = 40.0; //字体大小
static bool g_Initialized = false; //imgui是否初始化
//static struct android_app*  g_App = nullptr;
static ANativeWindow* g_Window = nullptr; //ANativeWindow


void android_main(struct android_app* app){}


//hook函数initializeMotionEvent，用于imgui触摸
void (*orig_initializeMotionEvent)(void *thiz, void *ex_ab, void *ex_ac);
void hooked_initializeMotionEvent(void *thiz, void *ex_ab, void *ex_ac) {
    try {
        orig_initializeMotionEvent(thiz, ex_ab, ex_ac);
        ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz); 
    } catch (const std::exception &e) {
        return;
    }
}

 
//hook函数CreateWindowSurface，用于获取ANativeWindow
EGLSurface (*orig_eglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
EGLSurface hooked_eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,EGLNativeWindowType win, const EGLint* attrib_list)
{        
    g_Window = (ANativeWindow*)win;
    return orig_eglCreateWindowSurface(dpy, config, win, attrib_list);    
}


//hook函数EglSwapBuffers，用于渲染imgui
EGLBoolean (*orig_eglSwapBuffers)(EGLDisplay dpy,EGLSurface surface);
EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy,EGLSurface surface)
{
    if(!g_Initialized)//初始化
    {        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
   
        ImGui::StyleColorsDark();
        
        ImFontConfig font_cfg;
        font_cfg.MergeMode = true; //
        font_cfg.SizePixels = fontsize;
        io.Fonts->AddFontFromMemoryTTF((void *)font_v, font_v_size, fontsize, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        
        ImGui_ImplAndroid_Init(g_Window);
        ImGui_ImplOpenGL3_Init("#version 300 es");   
        io.Fonts->Build();
        g_Initialized = true;
    }

    ImGuiIO& io = ImGui::GetIO();
    
    glDepthFunc(GL_ALWAYS); 
    //开始绘制
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    
    ImGui::SetNextWindowSize(ImVec2(ScreenWidth,ScreenHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Test Window");
    
    ImGui::Text("This is some useful text.");   
    if (ImGui::Button("Button", ImVec2(150, 50))) {
          LOGD("点击了按钮");
    }
    static bool checkbox1 = false;
    static bool checkbox2 = false;
    ImGui::Checkbox("复选框1", &checkbox1);
    ImGui::Checkbox("复选框2", &checkbox2);
    
    
    ImGui::End();   //结束
    
    ImGui::EndFrame();  
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    return orig_eglSwapBuffers(dpy, surface);
}


void Hook() { //启动hook
    void* addr_eglCreateWindowSurface = DobbySymbolResolver("libEGL.so", "eglCreateWindowSurface");
    DobbyHook(addr_eglCreateWindowSurface,(void*)hooked_eglCreateWindowSurface,(void**)&orig_eglCreateWindowSurface);
               
    void* addr_eglSwapBuffers = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    DobbyHook(addr_eglSwapBuffers,(void*)&hooked_eglSwapBuffers,(void**)&orig_eglSwapBuffers);
           
    void *addr_initializeMotionEvent = DobbySymbolResolver(("/system/lib/libinput.so"), ("_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE")); 
    DobbyHook((void *)addr_initializeMotionEvent, (void *)hooked_initializeMotionEvent, (void **)&orig_initializeMotionEvent);    
}
    

__attribute__((constructor)) void _init() { //入口
    LOGD("注入成功");    
    std::thread Main_thread(Hook); 
    Main_thread.detach();
}

//by Chen 2025.12.27
//源码于https://github.com/user3253/Android-imgui-can-inject
