/* Copyright (C) 2023 Charles K. Neimog
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>              /* assert */
#include <emscripten/em_asm.h>   /* EM_ASM */
#include <emscripten/webaudio.h> /* WebAudio API  */
#include <z_libpd.h>             /* libpd */

// ========== AUDIO CONFIG =============
uint8_t patchAudioInputs = 1;
uint8_t patchAudioOutputs = 2;
uint8_t wasmAudioWorkletStack[1024 * 1024];
int samplerate = 48000;

// ================ GUI ================
pthread_mutex_t WriteReadMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct pdItem {
  const char *receiverID;
  float f;
  const char *s;
  t_atom *list;
  int listSize;
  int type;
  int changed;
} pdItem;

// ====================
typedef struct pdItemHash {
  pdItem **items;
  int size;
  int count;
} pdItemHash;

struct pdItemHash *receiverHash;

// ====================
void safe_memcpy(void *dest, size_t destSize, const void *src, size_t srcSize) {
  if (dest && src && destSize >= srcSize) {
    memcpy(dest, src, srcSize);
  } else {
    EM_ASM_({ alert("Error on memcpy"); });
  }
}

// ====================
static pdItemHash *CreatePdItemHash(int size) {
  pdItemHash *hash_table = (pdItemHash *)malloc(sizeof(pdItemHash));
  hash_table->size = size;
  hash_table->count = 0;
  hash_table->items = (pdItem **)calloc(size, sizeof(pdItem *));
  return hash_table;
}

// ====================
static int findIndex(const char *key) {
  int index = -1;
  for (int i = 0; i < HTML_IDS_SIZE; i++) {
    if (strcmp(HTML_IDS[i], key) == 0) {
      index = i;
      break;
    }
  }
  if (index == -1) {
    return 0;
  }
  return index;
}

// ====================
static void InsertList(pdItemHash *hash_table, const char *key, int size,
                       t_atom *list) {
  unsigned int index = findIndex(key);
  pthread_mutex_lock(&WriteReadMutex);
  pdItem *item = hash_table->items[index];
  if (item == NULL && hash_table->count <= hash_table->size) {
    item = (pdItem *)malloc(sizeof(pdItem));
    item->receiverID = strdup(key);
    item->list = malloc(sizeof(t_atom) * size);
    safe_memcpy(item->list, sizeof(t_atom) * size, list, sizeof(t_atom) * size);
    item->listSize = size;
    item->type = A_GIMME;
    item->changed = 1;
    hash_table->items[index] = item;
    hash_table->count++;
  } else if (hash_table->count > hash_table->size) {
    EM_ASM_({
      alert("Hash table is full, please report this in " +
            "github.com/charlesneimog/pd4web");
    });
  } else if (item != NULL) {
    item->list = list;
    item->changed = 1;
    item->type = A_GIMME;
  }
  pthread_mutex_unlock(&WriteReadMutex);
  return;
}

// ====================
static void InsertFloat(pdItemHash *hash_table, const char *key, float f) {
  unsigned int index = findIndex(key);
  pthread_mutex_lock(&WriteReadMutex);
  pdItem *item = hash_table->items[index];
  if (item == NULL && hash_table->count <= hash_table->size) {
    item = (pdItem *)malloc(sizeof(pdItem));
    item->receiverID = strdup(key);
    item->f = f;
    item->type = A_FLOAT;
    item->changed = 1;
    hash_table->items[index] = item;
    hash_table->count++;
  } else if (hash_table->count > hash_table->size) {
    EM_ASM_({
      alert("Hash table is full, please report this in " +
            "github.com/charlesneimog/pd4web");
    });
  } else if (item != NULL) {
    item->f = f;
    item->changed = 1;
    item->type = A_FLOAT;
  }
  pthread_mutex_unlock(&WriteReadMutex);
  return;
}

// ====================
static void InsertSymbol(pdItemHash *hash_table, const char *key,
                         const char *thing) {
  unsigned int index = findIndex(key);
  pthread_mutex_lock(&WriteReadMutex);
  pdItem *item = hash_table->items[index];
  if (item == NULL && hash_table->count <= hash_table->size) {
    item = (pdItem *)malloc(sizeof(pdItem));
    item->receiverID = strdup(key);
    item->s = strdup(thing);
    item->type = A_SYMBOL;
    item->changed = 1;
    hash_table->items[index] = item;
    hash_table->count++;
  } else if (hash_table->count > hash_table->size) {
    EM_ASM_({
      alert("Hash table is full, please report this in " +
            "github.com/charlesneimog/pd4web");
    });
  } else if (item != NULL) {
    item->s = strdup(thing);
    item->changed = 1;
    item->type = A_SYMBOL;
  }
  pthread_mutex_unlock(&WriteReadMutex);
  return;
}

// ====================
static pdItem *GetItem(pdItemHash *hash_table, char *key) {
  unsigned int index = findIndex(key);
  pthread_mutex_lock(&WriteReadMutex);

  pdItem *item = hash_table->items[index];
  if (item != NULL) {
    pthread_mutex_unlock(&WriteReadMutex);
    return item;
  }
  pthread_mutex_unlock(&WriteReadMutex);
  return NULL;
}

// =====================================
// ============= HELPERS ===============
// =====================================
void pdprint(const char *s);

EMSCRIPTEN_KEEPALIVE
void *webpd_malloc(int size) { return malloc(size); }

// ======================================
EMSCRIPTEN_KEEPALIVE
void webpd_free(void *ptr) { free(ptr); }

// ======================================
EMSCRIPTEN_KEEPALIVE
int sendFloatToPd(const char *receiver, float value) {
  libpd_float(receiver, value);
  return 0;
}

// ======================================
EMSCRIPTEN_KEEPALIVE
int sendSymbolToPd(const char *receiver, const char *symbol) {
  return libpd_symbol(receiver, symbol);
}

// ======================================
EMSCRIPTEN_KEEPALIVE
int sendBangToPd(const char *receiver) { return libpd_bang(receiver); }

// ======================================
EMSCRIPTEN_KEEPALIVE
int bindGuiReceiver(const char *receiver) {
  libpd_bind(receiver);
  return 0;
}

// ======================================
// ============= libpd HOOKS ============
// ======================================

void pdprint(const char *s) {
  if (s[0] == '\n') {
    return;
  }
  EM_ASM_({ console.log(UTF8ToString($0)); }, s);
}

// ========================================
void receiveListfromPd(const char *src, int argc, t_atom *argv) {
  InsertList(receiverHash, (char *)src, argc, argv);
}

// ========================================
static void receiveMessageFromPd(const char *src, const char *sym, int argc,
                                 t_atom *argv) {

  if (argc == 0) {
    InsertSymbol(receiverHash, (char *)src, (char *)sym);
  }
}

// ========================================
void receiveFloatfromPd(const char *receiver, float value) {
  InsertFloat(receiverHash, (char *)receiver, value);
}

// ========================================
static void receiveSymbolfromPd(const char *receiver, const char *thing) {
  InsertSymbol(receiverHash, (char *)receiver, (char *)thing);
}

// ========================================
static void receiveBangfromPd(const char *receiver) {
  InsertSymbol(receiverHash, (char *)receiver, "bang");
}
// ========================================
// to remove warning about not defined
void sys_gui_midipreferences(void) { return; }

// ============= WEB AUDIO ================
static EM_BOOL ProcessPdPatch(int numInputs, const AudioSampleFrame *inputs,
                              int numOutputs, AudioSampleFrame *outputs,
                              int numParams, const AudioParamFrame *params,
                              void *userData) {
  int outCh = outputs[0].numberOfChannels;
  float tmpOutputs[128 * 2];
  libpd_process_float(2, inputs[0].data,
                      tmpOutputs); // TODO: ADD INPUTS AND OUTPUTS FOR pd2wasm.
  int outputIndex = 0;
  for (int i = 0; i < outCh; i++) {
    for (int j = i; j < (128 * outCh); j += 2) {
      outputs[0].data[outputIndex] = tmpOutputs[j];
      outputIndex++;
    }
  }
  libpd_bang("pd4webtick");
  return EM_TRUE;
}
// ========================================
// clang-format off
EM_JS(int, GetAudioSampleRate, (EMSCRIPTEN_WEBAUDIO_T audioContext), {
    return emscriptenGetAudioObject(audioContext).sampleRate;
});

// ========================================
EM_JS(void, LoadFinished, (void), {
    JS_LoadFinished();
});

// ========================================
EM_JS(void, AddUIButtons, (EMSCRIPTEN_WEBAUDIO_T audioContext, EMSCRIPTEN_AUDIO_WORKLET_NODE_T audioWorkletNode), {
    audioContext = emscriptenGetAudioObject(audioContext);
    audioWorkletNode = emscriptenGetAudioObject(audioWorkletNode);
    JS_AddUIButtons(audioContext, audioWorkletNode);
});
// clang-format on

// ========================================
void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext,
                                  EM_BOOL success, void *userData) {
  if (!success) {
    return;
  }
  int outputChannelCounts[1] = {2};
  // TODO: Thing about this in multichannel
  EmscriptenAudioWorkletNodeCreateOptions options = {
      .numberOfInputs = 1,
      .numberOfOutputs = 1,
      .outputChannelCounts = outputChannelCounts,
  };

  EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet =
      emscripten_create_wasm_audio_worklet_node(audioContext, "libpd-processor",
                                                &options, &ProcessPdPatch, 0);
  AddUIButtons(audioContext, wasmAudioWorklet);
  libpd_set_listhook(receiveListfromPd);
  libpd_set_floathook(receiveFloatfromPd);
  libpd_set_symbolhook(receiveSymbolfromPd);
  libpd_set_banghook(receiveBangfromPd);
  libpd_set_messagehook(receiveMessageFromPd);

  libpd_set_printhook(pdprint);
  libpd_init();

  for (int i = 0; i < HTML_IDS_SIZE; i++) {
    libpd_bind(HTML_IDS[i]);
  }
  libpd_add_to_search_path("webpatch/data/");
  libpd_add_to_search_path("webpatch/data/Audios/");
  libpd_start_message(1);
  libpd_add_float(1.0f);
  libpd_finish_message("pd", "dsp");
  libpd_init_audio(patchAudioInputs, patchAudioOutputs,
                   GetAudioSampleRate(audioContext));
  if (!libpd_openfile("index.pd", "webpatch/data")) {
    EM_ASM_({ alert("Failed to open patch!"); });
    return;
  }
  EM_ASM_({ JS_LoadFinished(); });
}

// ========================================
void WebAudioWorkletThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext,
                                      EM_BOOL success, void *userData) {
  if (!success) {
    return;
  }
  WebAudioWorkletProcessorCreateOptions opts = {
      .name = "libpd-processor",
  };
  emscripten_create_wasm_audio_worklet_processor_async(
      audioContext, &opts, AudioWorkletProcessorCreated, 0);
}

// ========================================
// clang-format off
EM_JS(void, setFloatValue, (const char* symbol, float value), {
    if (typeof JS_setFloat === "function"){
        JS_setFloat(UTF8ToString(symbol), value);
    } else{
        console.log("JS_setFloat not defined");
    }
});

// ========================================
EM_JS(void, setSymbolValue, (const char* symbol, const char* value), {
    if (typeof JS_setSymbol === "function"){
        JS_setSymbol(UTF8ToString(symbol), UTF8ToString(value));
    } else{
        console.log("JS_setSymbol not defined");
    }
});

// ========================================
EM_JS(void, clearListValue, (const char* symbol), {
    if (typeof JS_setList === "function"){
        window.pd4webGuiValues[symbol] = [];
    } else{
        console.log("JS_setList not defined");
    }
});
// ========================================
EM_JS(void, setListValueFloat, (const char* symbol, int isfloat, const char* valueSymbol, float valueFloat), {
    if (typeof JS_setList === "function"){
        if (isfloat){
            JS_setList(UTF8ToString(symbol), valueFloat);
        } else{
            JS_setList(UTF8ToString(symbol), UTF8ToString(valueSymbol));
        }
    } else{
        console.log("JS_setList not defined");
    }
});
// clang-format on
// ========================================
void PdWebCompiler_Loop() {
  for (int i = 0; i < HTML_IDS_SIZE; i++) {
    pdItem *item = GetItem(receiverHash, HTML_IDS[i]);
    if (item == NULL)
      continue;
    if (item->changed) {
      item->changed = 0;
      if (item->type == A_FLOAT) {
        setFloatValue(HTML_IDS[i], item->f);
      } else if (item->type == A_SYMBOL) {
        setSymbolValue(HTML_IDS[i], item->s);
      } else if (item->type == A_GIMME) {
        clearListValue(HTML_IDS[i]);
        for (int j = 0; j < item->listSize; j++) {
          if (item->list[j].a_type == A_FLOAT) {
            setListValueFloat(HTML_IDS[i], 1, NULL, item->list[j].a_w.w_float);
          } else if (item->list[j].a_type == A_SYMBOL) {
            setListValueFloat(HTML_IDS[i], 0,
                              item->list[j].a_w.w_symbol->s_name, 0);
          }
        }
      }

      else {
        EM_ASM_({ alert("Unknown type"); });
        return;
      }
    }
  }
}

// ========================================
int main() {
  srand(time(NULL));
  assert(!emscripten_current_thread_is_audio_worklet());

  EmscriptenWebAudioCreateAttributes attrs = {
      .latencyHint = "interactive",
  };
  EMSCRIPTEN_WEBAUDIO_T context = emscripten_create_audio_context(&attrs);
  samplerate = GetAudioSampleRate(context);
  emscripten_start_wasm_audio_worklet_thread_async(
      context, wasmAudioWorkletStack, sizeof(wasmAudioWorkletStack),
      WebAudioWorkletThreadInitialized, 0);
  receiverHash = CreatePdItemHash(HTML_IDS_SIZE);
  emscripten_set_main_loop(PdWebCompiler_Loop, 30,
                           1); /* 30 FPS TODO: check this from pd4web */
}
