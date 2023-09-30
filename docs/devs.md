# Documentation for Devs

<p align="center"> Page to document the details of how you can contribute with <code>pd4web</code>. </p>

## <h2 align="center"> **pd4web** </h2>

`pd4web` is a collection of tools with a user interface provided through a Python module called `pd4web`. `pd4web` can be divided into three main parts:

1. The main script, `pd4web.py`.
2. The `externals` folder, consisting of the main file `ExternalClass.py` and files for each library.
3. The `lib` folder, comprising the main file main.py and files for each library.

### <h3 align="center"> **Adding a new PureData Library/External** </h3>

To add a new PureData External you must add a new entry in the `Externals.yaml`. The `.yaml` file work with list of keys, the keys that you must specify is, `name`, `repoUser`, `repoName`, `download_source` or `direct_link`. We have some optional keys too: `singleObject`, `dynamicLibraries`, `extraFunction` and `version`.

<br>
<table class="special-table">
    <thead>
      <tr>
        <th>  Key Name  </th>
        <th>Description</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td><code>name</code></td>
        <td>With name you specify the name of the library, for example <code>else</code>, <code>cyclone</code>, <code>timbreIdLib</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>repoUser</code></td>
        <td>The name of the user where the repo is hosted. For example <code>porres</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>repoName</code></td>
        <td>The name of the repository where it is hosted. For example <code>pd-else</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>download_source</code></td>
        <td>The download source, it must be <code>GITHUB_RELEASES</code> or <code>GITHUB_TAGS</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>direct_link</code></td>
        <td>If you repository dev don't publish Releases or Tags you must use the direct link to a zip file.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>singleObject</code></td>
        <td>When the library is just a single object (<code>earplug~</code> for example) you must mark this as <code>true</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>dynamicLibraries</code></td>
        <td>It is a list of all dynamic libraries used, for example, <code>convolve~</code> uses fftw3 so we need to set <code>["fftw3"]. It should be in the supported dynamic libraries.</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>extraFunction</code></td>
        <td>Name of the Python Function to copy headers and another files, add compilation flags, and others to the webpatch folder. Just used for complex libraries.</td>
      </tr>
    </tbody>
    
  </table>

<br>

Here an example:

``` yaml

SupportedLibraries:
  - name: cyclone # name of the pd library
    download_source: GITHUB_RELEASES # source
    repoUser: porres # in this case, Github user name
    repoName: pd-cyclone # in this case, Github Repo
    version: cyclone_0.7-0 # version
    extraFunction: cyclone_extra # This library needs some extras steps, 
                                 # so we used a function called cyclone_extra, defined in externals.cyclone.
```

#### <h4 align="center"> **Writing the extraFunction** </h4>

The `extraFunction` is responsible for making all extra configurations for a PureData External. The `extraFunction` is defined inside a file inside the `externals` folder. It will be automatically imported and used when you specify it on `extraFunction` key in the `Externals.yaml` file.

`pd4web` configures a `.c` file for compilation. To set up the compilation, we assume that: 

1. All header files `(.h)` are located within the `webpatch/includes` folder.
2. All the files to be compiled `(.c)` are located within `webpatch/externals`.
3. All additional files, such as external configurations, data files, and others, are situated within `webpatch/data`. Therefore, you need to place each file in its respective folder.

??? tip "Example of Extra Function for pd-else"

    ``` python
    def else_extra(librarySelf: PureDataExternals):
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

        os.remove(os.path.join(librarySelf.PROJECT_ROOT, "webpatch", "externals", "s_elseutf8.c")) # conflit with pd source.
        librarySelf.extraFuncExecuted = True
        if 'sfz~' in librarySelf.usedObjs:
            librarySelf.webpdPatch.print("sfz~ object is not supported yet", color="red")

        elif 'sfont~' in librarySelf.usedObjs:
            librarySelf.webpdPatch.print("sfont~ object is not supported yet", color="red")

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

    ```
