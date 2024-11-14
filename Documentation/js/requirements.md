`Pd4Web` uses the `WebAudio` API to communicate with `Pd`. This means that you need to have a browser and a server that supports some features.


### <h3 align="center"><code>Pd4Web.init()</code></h3>

`Pd4Web.init()` must be called from a user gesture, like a click or a touch event. This is a security feature of the browser to prevent the execution of scripts without user interaction.

!!! info "Just if you will use the microphone"
    `Pd4Web.init()` must be called from a user gesture **just** if you will use the microphone. 


### <h3 align="center">Cross-Origin Isolation</h3>

COI is a security feature that allows you to restrict how your website interacts with other websites. If COI is not enabled, you will see an error message related with `SharedArrayBuffer` being not defined.

`Pd4Web` uses the script `pd4web.threads.js` to enable COI if it's not enabled from the server. But there is some limitations, first of all you need to create a redirect `index.html` file from the root of the WebSite to an specific folder. This is necessary because the `pd4web.threads.js` script needs to be in the same folder as the `index.html` file. I am yet researching a better way to do this.


--- 

