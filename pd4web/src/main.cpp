#include <emscripten.h>
#include <emscripten/bind.h>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <z_libpd.h>             

class Pd4Web {
public:
    EMSCRIPTEN_KEEPALIVE int pd4web_init();

private:
    void *p;
    bool Pd4WebInit = false;

};


int Pd4Web::pd4web_init() {
    if (Pd4WebInit) {
        printf("Pd4Web already initialized\n");
        return 0;
    }

    return 0;
}


// ╭─────────────────────────────────────╮
// │  Bind C++ functions to JavaScript   │
// ╰─────────────────────────────────────╯
EMSCRIPTEN_BINDINGS(WebPd) {
  emscripten::register_vector<char>("VectorChar");
  emscripten::register_vector<float>("VectorFloat");
  emscripten::class_<Pd4Web>("Pd4Web")
      .constructor<>() // Default constructor
      // .function("getVectorSize", &Pd4Web::getVectorSize);
}
