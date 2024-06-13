#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/webaudio.h>

#include <util/z_print_util.h>
#include <z_libpd.h>

#include "audio-config.h"

#define PD4WEB_MAJOR_VERSION 2
#define PD4WEB_MINOR_VERSION 0
#define PD4WEB_MICRO_VERSION 0

static uint8_t wasmAudioWorkletStack[1024 * 1024];
// ╭─────────────────────────────────────╮
// │             Main Class              │
// ╰─────────────────────────────────────╯
class Pd4Web {
  public:
    Pd4Web() {
        //
    }
    EMSCRIPTEN_KEEPALIVE void Init();
    void SuspendAudio();
    void ResumeAudio();

    void Process(uintptr_t input_ptr, uintptr_t output_ptr,
                 unsigned channel_count) {
        float *input_buffer = reinterpret_cast<float *>(input_ptr);
        float *output_buffer = reinterpret_cast<float *>(output_ptr);
    }

    // libpd HOOKs
    void receiveMessage(const char *source, const char *symbol, int argc,
                        t_atom *argv);

  private:
    void Pd4WebInitExternals();

    bool Pd4WebInit = false;
    EMSCRIPTEN_WEBAUDIO_T Context;
    bool pdInit = false;

    // Hooks
    static void receivePrint(const char *s);
    // void receiveBang(const char *s);
    // void receiveFloat(const char *source, float value);
    // void receiveSymbol(const char *source, const char *symbol);
};

// ╭─────────────────────────────────────╮
// │  Bind C++ functions to JavaScript   │
// ╰─────────────────────────────────────╯
EMSCRIPTEN_BINDINGS(WebPd) {
    emscripten::class_<Pd4Web>("Pd4Web")
        .constructor<>() // Default constructor
        .function("init", &Pd4Web::Init)
        .function("suspendAudio", &Pd4Web::SuspendAudio)
        .function("resumeAudio", &Pd4Web::ResumeAudio);
}
