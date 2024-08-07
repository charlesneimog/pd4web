# Adding Externals (Devs)

<p align="center"> Page to document the details of how you can contribute with <code>pd4web</code>. </p>

## <h2 align="center"> **pd4web**: Adding Support to Externals </h2>

`pd4web` consists of a set of tools accessible via a Python module named pd4web. Internally, pd4web utilizes `EMSCRIPTEN` to compile external components into dynamic static libraries.

To integrate a new library, you must create a `CMakeLists.txt`. It's highly recommended to use [pd.cmake](https://github.com/pure-data/pd.cmake) due to assumptions made (such as the TARGET name convention like earplug~ becoming earplug_tilde). After creating the `CMakeLists.txt` for the library you need, submit a pull request that modifies `Externals.yaml` within `Sources/pd4web/`. In this file, simply add a new entry similar to the provided example.

``` yaml

  - Name: cyclone # name of the pd library
    Source: GITHUB_RELEASES # source
    Developer: porres # in this case, Github user name
    Repository: pd-cyclone # in this case, Github Repo
    Version: cyclone_0.7-0 # version
```

Below the complete fields for enviroment.

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
        <td><code>Name</code></td>
        <td>With name you specify the name of the library, for example <code>else</code>, <code>cyclone</code>, <code>timbreIdLib</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>Developer</code></td>
        <td>The username of the user where the repository is hosted. For example <code>porres</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>Repository</code></td>
        <td>The name of the repository where it is hosted. For example <code>pd-else</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>Source</code></td>
        <td>The download source, it must be <code>GITHUB_RELEASES</code> or <code>GITHUB_TAGS</code>.</td>
      </tr>
    </tbody>
    
    <tbody>
      <tr>
        <td><code>DirectLink</code></td>
        <td>If you repository dev don't publish Releases or Tags you must use the direct link to a zip file.</td>
      </tr>
    </tbody>
    
    
    <tbody>
      <tr>
        <td><code>Dependencies</code></td>
        <td>It is a list of all dynamic libraries used, for example, <code>convolve~</code> uses fftw3 so we need to set <code>["fftw3"]</code>. It should be in the supported Dynamic Libraries.</code>.</td>
      </tr>
    </tbody>
    
  </table>

<br>


