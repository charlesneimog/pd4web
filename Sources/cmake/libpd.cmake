cmake_minimum_required(VERSION 2.8.12)
project(libpd C)

set(PD_SOURCES
    Pd4Web/pure-data/src/d_arithmetic.c
    Pd4Web/pure-data/src/d_array.c
    Pd4Web/pure-data/src/d_ctl.c
    Pd4Web/pure-data/src/d_dac.c
    Pd4Web/pure-data/src/d_delay.c
    Pd4Web/pure-data/src/d_fft.c
    Pd4Web/pure-data/src/d_fft_fftsg.c
    Pd4Web/pure-data/src/d_filter.c
    Pd4Web/pure-data/src/d_global.c
    Pd4Web/pure-data/src/d_math.c
    Pd4Web/pure-data/src/d_misc.c
    Pd4Web/pure-data/src/d_osc.c
    Pd4Web/pure-data/src/d_resample.c
    Pd4Web/pure-data/src/d_soundfile.c
    Pd4Web/pure-data/src/d_soundfile_aiff.c
    Pd4Web/pure-data/src/d_soundfile_caf.c
    Pd4Web/pure-data/src/d_soundfile_next.c
    Pd4Web/pure-data/src/d_soundfile_wave.c
    Pd4Web/pure-data/src/d_ugen.c
    Pd4Web/pure-data/src/g_all_guis.c
    Pd4Web/pure-data/src/g_array.c
    Pd4Web/pure-data/src/g_bang.c
    Pd4Web/pure-data/src/g_canvas.c
    Pd4Web/pure-data/src/g_clone.c
    Pd4Web/pure-data/src/g_editor.c
    Pd4Web/pure-data/src/g_editor_extras.c
    Pd4Web/pure-data/src/g_graph.c
    Pd4Web/pure-data/src/g_guiconnect.c
    Pd4Web/pure-data/src/g_io.c
    Pd4Web/pure-data/src/g_mycanvas.c
    Pd4Web/pure-data/src/g_numbox.c
    Pd4Web/pure-data/src/g_radio.c
    Pd4Web/pure-data/src/g_readwrite.c
    Pd4Web/pure-data/src/g_rtext.c
    Pd4Web/pure-data/src/g_scalar.c
    Pd4Web/pure-data/src/g_slider.c
    Pd4Web/pure-data/src/g_template.c
    Pd4Web/pure-data/src/g_text.c
    Pd4Web/pure-data/src/g_toggle.c
    Pd4Web/pure-data/src/g_traversal.c
    Pd4Web/pure-data/src/g_undo.c
    Pd4Web/pure-data/src/g_vumeter.c
    Pd4Web/pure-data/src/m_atom.c
    Pd4Web/pure-data/src/m_binbuf.c
    Pd4Web/pure-data/src/m_class.c
    Pd4Web/pure-data/src/m_conf.c
    Pd4Web/pure-data/src/m_glob.c
    Pd4Web/pure-data/src/m_imp.h
    Pd4Web/pure-data/src/m_memory.c
    Pd4Web/pure-data/src/m_obj.c
    Pd4Web/pure-data/src/m_pd.c
    Pd4Web/pure-data/src/m_sched.c
    Pd4Web/pure-data/src/s_audio.c
    Pd4Web/pure-data/src/s_audio_dummy.c
    Pd4Web/pure-data/src/s_inter.c
    Pd4Web/pure-data/src/s_inter_gui.c
    Pd4Web/pure-data/src/s_loader.c
    Pd4Web/pure-data/src/s_main.c
    Pd4Web/pure-data/src/s_net.c
    Pd4Web/pure-data/src/s_path.c
    Pd4Web/pure-data/src/s_print.c
    Pd4Web/pure-data/src/s_utf8.c
    Pd4Web/pure-data/src/x_acoustics.c
    Pd4Web/pure-data/src/x_arithmetic.c
    Pd4Web/pure-data/src/x_array.c
    Pd4Web/pure-data/src/x_connective.c
    Pd4Web/pure-data/src/x_file.c
    Pd4Web/pure-data/src/x_gui.c
    Pd4Web/pure-data/src/x_interface.c
    Pd4Web/pure-data/src/x_list.c
    Pd4Web/pure-data/src/x_midi.c
    Pd4Web/pure-data/src/x_misc.c
    Pd4Web/pure-data/src/x_net.c
    Pd4Web/pure-data/src/x_scalar.c
    Pd4Web/pure-data/src/x_text.c
    Pd4Web/pure-data/src/x_time.c
    Pd4Web/pure-data/src/x_vexp.c
    Pd4Web/pure-data/src/x_vexp_fun.c
    Pd4Web/pure-data/src/x_vexp_if.c)

# TODO: Add the following sources
set(PD_BOB_TILDE_SOURCE Pd4Web/pure-data/extra/bob~/bob~.c)
set(PD_BONK_TILDE_SOURCE Pd4Web/pure-data/extra/bonk~/bonk~.c)
set(PD_CHOICE_SOURCE Pd4Web/pure-data/extra/choice/choice.c)
set(PD_FIDDLE_TILDE_SOURCE Pd4Web/pure-data/extra/fiddle~/fiddle~.c)
set(PD_LOOP_TILDE_SOURCE Pd4Web/pure-data/extra/loop~/loop~.c)
set(PD_LRSHIFT_TILDE_SOURCE Pd4Web/pure-data/extra/lrshift~/lrshift~.c)
set(PD_PDSCHED_SOURCE Pd4Web/pure-data/extra/pd~/pdsched.c)
# set(PD_PD_TILDE_SOURCE Pd4Web/pure-data/extra/pd~/pd~.c)
set(PD_PIQUE_SOURCE Pd4Web/pure-data/extra/pique/pique.c)
set(PD_SIGMUND_TILDE_SOURCE Pd4Web/pure-data/extra/sigmund~/sigmund~.c)
set(PD_STDOUT_SOURCE Pd4Web/pure-data/extra/stdout/stdout.c)

set(LIBPD_MAIN_SOURCES
    Pd4Web/pure-data/src/s_libpdmidi.c Pd4Web/pure-data/src/x_libpdreceive.c Pd4Web/pure-data/src/x_libpdreceive.h
    Pd4Web/pure-data/src/z_hooks.c Pd4Web/pure-data/src/z_hooks.h Pd4Web/pure-data/src/z_libpd.c)

set(LIBPD_UTILS_SOURCES
    Pd4Web/pure-data/src/util/z_ringbuffer.c Pd4Web/pure-data/src/util/z_ringbuffer.h
    Pd4Web/pure-data/src/util/z_print_util.c Pd4Web/pure-data/src/util/z_print_util.h
    Pd4Web/pure-data/src/util/z_queued.c Pd4Web/pure-data/src/util/z_queued.h)

set(LIBPD_SOURCES ${PD_SOURCES} ${LIBPD_MAIN_SOURCES})

add_library(libpd STATIC ${LIBPD_SOURCES})
set_target_properties(libpd PROPERTIES OUTPUT_NAME pd)
target_compile_definitions(libpd PRIVATE -DPD=1 -DUSEAPI_DUMMY=1 -DHAVE_UNISTD_H=1 -DHAVE_LIBDL -DPD_INTERNAL)
target_compile_options(libpd PRIVATE -w)
target_compile_options(libpd PRIVATE $<$<CONFIG:Release>:-ffast-math> $<$<CONFIG:Release>:-funroll-loops>
                                     $<$<CONFIG:Release>:-fomit-frame-pointer> $<$<CONFIG:Release>:-O3>)
target_include_directories(libpd PRIVATE Pd4Web/pure-data/src)
