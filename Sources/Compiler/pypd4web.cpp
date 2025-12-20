#define Py_LIMITED_API 0x03080000

#include <Python.h>

#include "pd4web_compiler.hpp"

#include <string>
#include <vector>

namespace {

struct Pd4WebCompilerObject {
    PyObject_HEAD
    Pd4Web *cpp;
};

static int as_std_string(PyObject *obj, std::string &out) {
    if (!PyUnicode_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "expected str");
        return -1;
    }

    PyObject *utf8 = PyUnicode_AsUTF8String(obj);
    if (!utf8) {
        return -1;
    }

    char *data = nullptr;
    Py_ssize_t size = 0;
    if (PyBytes_AsStringAndSize(utf8, &data, &size) != 0) {
        Py_DECREF(utf8);
        return -1;
    }

    out.assign(data, static_cast<size_t>(size));
    Py_DECREF(utf8);
    return 0;
}

static int collect_strings(PyObject *iterable, std::vector<std::string> &out) {
    PyObject *iter = PyObject_GetIter(iterable);
    if (!iter) {
        PyErr_SetString(PyExc_TypeError, "expected an iterable of str");
        return -1;
    }

    PyObject *item = nullptr;
    while ((item = PyIter_Next(iter)) != nullptr) {
        std::string value;
        int ok = as_std_string(item, value);
        Py_DECREF(item);
        if (ok != 0) {
            Py_DECREF(iter);
            return -1;
        }
        out.push_back(std::move(value));
    }
    Py_DECREF(iter);

    if (PyErr_Occurred()) {
        return -1;
    }

    return 0;
}

static int ensure_cpp(Pd4WebCompilerObject *self) {
    if (!self->cpp) {
        PyErr_SetString(PyExc_RuntimeError, "Pd4WebCompiler is not initialized");
        return -1;
    }
    return 0;
}

static int Pd4WebCompiler_init(PyObject *self_obj, PyObject *args, PyObject *kwargs) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    self->cpp = nullptr;

    const char *kwlist[] = {"home", nullptr};
    PyObject *home_obj = nullptr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", const_cast<char **>(kwlist),
                                     &home_obj)) {
        return -1;
    }

    std::string home;
    if (home_obj) {
        if (as_std_string(home_obj, home) != 0) {
            return -1;
        }
    }

    try {
        self->cpp = new Pd4Web(home);
    } catch (const std::exception &ex) {
        PyErr_SetString(PyExc_RuntimeError, ex.what());
        return -1;
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "Pd4Web construction failed");
        return -1;
    }

    return 0;
}

static void Pd4WebCompiler_dealloc(PyObject *self_obj) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    delete self->cpp;
    self->cpp = nullptr;

    freefunc free_fn = reinterpret_cast<freefunc>(
        PyType_GetSlot(Py_TYPE(self_obj), Py_tp_free));
    if (free_fn) {
        free_fn(self_obj);
    }
}

static PyObject *Pd4WebCompiler_parse_args(PyObject *self_obj, PyObject *args,
                                           PyObject *kwargs) {
    (void)kwargs;
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *seq = nullptr;
    if (!PyArg_ParseTuple(args, "O", &seq)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    std::vector<std::string> values;
    if (collect_strings(seq, values) != 0) {
        return nullptr;
    }

    std::vector<char *> argv(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        argv[i] = const_cast<char *>(values[i].c_str());
    }

    self->cpp->parseArgs(static_cast<int>(values.size()), argv.data());
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_init_engine(PyObject *self_obj, PyObject *Py_UNUSED(args)) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    bool ok = self->cpp->init();
    return PyBool_FromLong(ok ? 1 : 0);
}

static PyObject *Pd4WebCompiler_compile_patch(PyObject *self_obj, PyObject *Py_UNUSED(args)) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    bool ok = self->cpp->compilePatch();
    return PyBool_FromLong(ok ? 1 : 0);
}

static PyObject *Pd4WebCompiler_set_patch_file(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    std::string path;
    if (as_std_string(value, path) != 0) {
        return nullptr;
    }

    self->cpp->setPatchFile(path);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_initial_memory(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    long mem = 0;
    if (!PyArg_ParseTuple(args, "l", &mem)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    self->cpp->setInitialMemory(static_cast<int>(mem));
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_patch_zoom(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    long zoom = 0;
    if (!PyArg_ParseTuple(args, "l", &zoom)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    self->cpp->setPatchZoom(static_cast<int>(zoom));
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_output_folder(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    std::string path;
    if (as_std_string(value, path) != 0) {
        return nullptr;
    }

    self->cpp->setOutputFolder(path);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_root_folder(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    std::string path;
    if (as_std_string(value, path) != 0) {
        return nullptr;
    }

    self->cpp->setPd4WebRootFolder(path);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_template_id(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    long value = 0;
    if (!PyArg_ParseTuple(args, "l", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    self->cpp->setTemplateId(static_cast<int>(value));
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_debug_mode(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    int truth = PyObject_IsTrue(value);
    if (truth < 0) {
        return nullptr;
    }

    self->cpp->setDebugMode(truth != 0);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_dev_debug_mode(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    int truth = PyObject_IsTrue(value);
    if (truth < 0) {
        return nullptr;
    }

    self->cpp->setDevDebugMode(truth != 0);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_set_fail_fast(PyObject *self_obj, PyObject *args) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    PyObject *value = nullptr;
    if (!PyArg_ParseTuple(args, "O", &value)) {
        return nullptr;
    }

    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    int truth = PyObject_IsTrue(value);
    if (truth < 0) {
        return nullptr;
    }

    self->cpp->setFailFast(truth != 0);
    Py_RETURN_NONE;
}

static PyObject *Pd4WebCompiler_disable_gui_render(PyObject *self_obj, PyObject *Py_UNUSED(args)) {
    auto *self = reinterpret_cast<Pd4WebCompilerObject *>(self_obj);
    if (ensure_cpp(self) != 0) {
        return nullptr;
    }

    self->cpp->disableGuiRender();
    Py_RETURN_NONE;
}

static PyMethodDef Pd4WebCompiler_methods[] = {
    {"parseArgs", reinterpret_cast<PyCFunction>(Pd4WebCompiler_parse_args), METH_VARARGS,
     "Parse command-line arguments."},
    {"init", reinterpret_cast<PyCFunction>(Pd4WebCompiler_init_engine), METH_NOARGS,
     "Initialize Pd4Web."},
    {"compilePatch", reinterpret_cast<PyCFunction>(Pd4WebCompiler_compile_patch), METH_NOARGS,
     "Compile the current patch."},
    {"setPatchFile", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_patch_file), METH_VARARGS,
     "Set the patch file."},
    {"setInitialMemory", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_initial_memory),
     METH_VARARGS, "Set initial memory."},
    {"setPatchZoom", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_patch_zoom), METH_VARARGS,
     "Set patch zoom."},
    {"setOutputFolder", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_output_folder),
     METH_VARARGS, "Set output folder."},
    {"setPd4WebFilesFolder", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_root_folder),
     METH_VARARGS, "Set Pd4Web files folder."},
    {"setTemplateId", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_template_id),
     METH_VARARGS, "Set template id."},
    {"setDebugMode", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_debug_mode), METH_VARARGS,
     "Enable or disable debug mode."},
    {"setDevDebugMode", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_dev_debug_mode),
     METH_VARARGS, "Enable or disable dev debug mode."},
    {"setFailFast", reinterpret_cast<PyCFunction>(Pd4WebCompiler_set_fail_fast), METH_VARARGS,
     "Enable or disable fail-fast."},
    {"disableGuiRender", reinterpret_cast<PyCFunction>(Pd4WebCompiler_disable_gui_render),
     METH_NOARGS, "Disable GUI rendering."},
    {nullptr, nullptr, 0, nullptr}};

static PyType_Slot Pd4WebCompiler_slots[] = {
    {Py_tp_new, reinterpret_cast<void *>(PyType_GenericNew)},
    {Py_tp_init, reinterpret_cast<void *>(Pd4WebCompiler_init)},
    {Py_tp_dealloc, reinterpret_cast<void *>(Pd4WebCompiler_dealloc)},
    {Py_tp_methods, reinterpret_cast<void *>(Pd4WebCompiler_methods)},
    {0, nullptr}};

static PyType_Spec Pd4WebCompiler_spec = {
    "pypd4web.Pd4WebCompiler",
    sizeof(Pd4WebCompilerObject),
    0,
    Py_TPFLAGS_DEFAULT,
    Pd4WebCompiler_slots,
};

static PyObject *pypd4web_run(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
    (void)kwargs;
    PyObject *seq = nullptr;
    if (!PyArg_ParseTuple(args, "O", &seq)) {
        return nullptr;
    }

    std::vector<std::string> values;
    if (collect_strings(seq, values) != 0) {
        return nullptr;
    }

    Pd4Web pd4web("");
    std::vector<char *> argv(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        argv[i] = const_cast<char *>(values[i].c_str());
    }

    pd4web.parseArgs(static_cast<int>(values.size()), argv.data());
    pd4web.init();
    pd4web.compilePatch();

    Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    {"run", reinterpret_cast<PyCFunction>(pypd4web_run), METH_VARARGS,
     "Run pd4web with command-line arguments."},
    {nullptr, nullptr, 0, nullptr}};

static PyModuleDef pypd4web_module = {
    PyModuleDef_HEAD_INIT,
    "pypd4web",
    "pd4web compiles PureData patches with external objects for Wasm, allowing to run entire "
    "patches in web browsers.",
    -1,
    module_methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr};

} // namespace

PyMODINIT_FUNC PyInit_pypd4web(void) {
    PyObject *module = PyModule_Create(&pypd4web_module);
    if (!module) {
        return nullptr;
    }

    PyObject *type = PyType_FromSpec(&Pd4WebCompiler_spec);
    if (!type) {
        Py_DECREF(module);
        return nullptr;
    }

    if (PyModule_AddObject(module, "Pd4WebCompiler", type) != 0) {
        Py_DECREF(type);
        Py_DECREF(module);
        return nullptr;
    }

    return module;
}
