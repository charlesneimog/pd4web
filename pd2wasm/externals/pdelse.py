import os
import shutil
from .ExternalClass import PureDataExternals
from ..helpers import myprint
from ..pd2wasm import webpdPatch

def else_extra(librarySelf: PureDataExternals):
    '''
    This function copy some things that I already need to compile some externals in cyclone
    '''

    if not os.path.exists(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes")):
        os.makedirs(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))
    folder = os.path.join(librarySelf.folder, "Code_source", "shared")

    for file in os.listdir(folder):
        if file.endswith(".h"):
            shutil.copy(os.path.join(folder, file),
                        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes"))

    for file in os.listdir(folder):
        if file.endswith(".c"):
            shutil.copy(os.path.join(folder, file),
                        os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals"))

    os.remove(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals", "s_elseutf8.c"))
    librarySelf.extraFuncExecuted = True
    if 'sfz~' in librarySelf.usedObjs:
        myprint("sfz~ object is not supported yet", color="red")

    elif 'sfont~' in librarySelf.usedObjs:
        myprint("sfont~ object is not supported yet", color="red")

    elif 'plaits~' in librarySelf.usedObjs:
        # inside the library folder, search recursively for the file plaits~.cpp
        plaitsFile = None
        for root, _, files in os.walk(librarySelf.folder):
            for file in files:
                if file.endswith("plaits~.cpp"):
                    plaitsFile = os.path.join(root, file)
                    plaitsFolder = os.path.dirname(plaitsFile)
                    file_names = [
                        "stmlib/dsp/units.cc",
                        "stmlib/utils/random.cc",
                        "stmlib/dsp/atan.cc",
                        "plaits/dsp/voice.cc",
                        "plaits/dsp/engine/additive_engine.cc",
                        "plaits/dsp/engine/bass_drum_engine.cc",
                        "plaits/dsp/engine/chord_engine.cc",
                        "plaits/dsp/engine/fm_engine.cc",
                        "plaits/dsp/engine/grain_engine.cc",
                        "plaits/dsp/engine/hi_hat_engine.cc",
                        "plaits/dsp/engine/modal_engine.cc",
                        "plaits/dsp/engine/noise_engine.cc",
                        "plaits/dsp/engine/particle_engine.cc",
                        "plaits/dsp/engine/snare_drum_engine.cc",
                        "plaits/dsp/engine/speech_engine.cc",
                        "plaits/dsp/engine/string_engine.cc",
                        "plaits/dsp/engine/swarm_engine.cc",
                        "plaits/dsp/engine/virtual_analog_engine.cc",
                        "plaits/dsp/engine/waveshaping_engine.cc",
                        "plaits/dsp/engine/wavetable_engine.cc",
                        "plaits/dsp/speech/lpc_speech_synth.cc",
                        "plaits/dsp/speech/lpc_speech_synth_controller.cc",
                        "plaits/dsp/speech/lpc_speech_synth_phonemes.cc",
                        "plaits/dsp/speech/lpc_speech_synth_words.cc",
                        "plaits/dsp/speech/naive_speech_synth.cc",
                        "plaits/dsp/speech/sam_speech_synth.cc",
                        "plaits/dsp/physical_modelling/modal_voice.cc",
                        "plaits/dsp/physical_modelling/resonator.cc",
                        "plaits/dsp/physical_modelling/string.cc",
                        "plaits/dsp/physical_modelling/string_voice.cc",
                        "plaits/dsp/engine2/chiptune_engine.cc",
                        "plaits/dsp/engine2/phase_distortion_engine.cc",
                        "plaits/dsp/engine2/six_op_engine.cc",
                        "plaits/dsp/engine2/string_machine_engine.cc",
                        "plaits/dsp/engine2/virtual_analog_vcf_engine.cc",
                        "plaits/dsp/engine2/wave_terrain_engine.cc",
                        "plaits/dsp/fm/algorithms.cc",
                        "plaits/dsp/fm/dx_units.cc",
                        "plaits/dsp/chords/chord_bank.cc",
                        "plaits/resources.cc"
                    ]
                    for plaitsFile in file_names:
                        librarySelf.webpdPatch.sortedSourceFiles.append(os.path.join(plaitsFolder, plaitsFile))

                    
                    plaitsIncludes = os.path.join(plaitsFolder, "plaits")
                    shutil.copytree(plaitsIncludes, os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "plaits"))

                    stmlibrary = os.path.join(plaitsFolder, "stmlib")
                    shutil.copytree(stmlibrary, os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "includes", "stmlib"))
                                    

        

