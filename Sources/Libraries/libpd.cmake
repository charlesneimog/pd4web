cmake_minimum_required(VERSION 3.25)
project(libpd C)

set(PD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_arithmetic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_array.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_ctl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_dac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_delay.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_fft.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_fft_fftsg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_filter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_global.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_math.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_osc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_resample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_soundfile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_soundfile_aiff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_soundfile_caf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_soundfile_next.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_soundfile_wave.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/d_ugen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_all_guis.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_array.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_bang.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_canvas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_clone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_editor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_editor_extras.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_graph.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_guiconnect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_mycanvas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_numbox.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_radio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_readwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_rtext.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_scalar.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_slider.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_template.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_text.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_toggle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_traversal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_undo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/g_vumeter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_atom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_binbuf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_class.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_conf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_glob.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_imp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_memory.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_obj.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_pd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/m_sched.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_audio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_audio_dummy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_inter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_inter_gui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_loader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_net.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_path.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_print.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_utf8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_acoustics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_arithmetic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_array.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_connective.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_file.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_gui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_interface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_midi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_net.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_scalar.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_text.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_vexp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_vexp_fun.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_vexp_if.c)

# TODO: Add the following sources
set(PD_BOB_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/bob~/bob~.c)
set(PD_BONK_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/bonk~/bonk~.c)
set(PD_CHOICE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/choice/choice.c)
set(PD_FIDDLE_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/fiddle~/fiddle~.c)
set(PD_LOOP_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/loop~/loop~.c)
set(PD_LRSHIFT_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/lrshift~/lrshift~.c)
set(PD_PDSCHED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/pd~/pdsched.c)
# set(PD_PD_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/pd~/pd~.c)
set(PD_PIQUE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/pique/pique.c)
set(PD_SIGMUND_TILDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/sigmund~/sigmund~.c)
set(PD_STDOUT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/extra/stdout/stdout.c)

set(LIBPD_MAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/s_libpdmidi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_libpdreceive.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/x_libpdreceive.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/z_hooks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/z_hooks.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/z_libpd.c)

set(LIBPD_UTILS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_ringbuffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_ringbuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_print_util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_print_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_queued.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src/util/z_queued.h)

set(LIBPD_SOURCES ${PD_SOURCES} ${LIBPD_MAIN_SOURCES})

add_library(libpd STATIC ${LIBPD_SOURCES})
set_target_properties(libpd PROPERTIES OUTPUT_NAME pd)
target_compile_definitions(libpd PRIVATE -DPD=1 -DUSEAPI_DUMMY=1 -DHAVE_UNISTD_H=1 -DHAVE_LIBDL -DPD_INTERNAL)
target_compile_options(libpd PRIVATE -w)
target_compile_options(libpd PRIVATE $<$<CONFIG:Release>:-ffast-math> $<$<CONFIG:Release>:-funroll-loops>
                                     $<$<CONFIG:Release>:-fomit-frame-pointer> $<$<CONFIG:Release>:-O3>)
target_include_directories(libpd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src)
