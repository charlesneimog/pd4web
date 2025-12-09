cmake_minimum_required(VERSION 3.25)
project(libpd C)

set(ENABLE_TILDE_TARGET_WARNING off)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -matomics -mbulk-memory")

# ╭──────────────────────────────────────╮
# │       Remove print of sys_vgui       │
# ╰──────────────────────────────────────╯
# Função para adicionar 'return;' após o comentário específico, se ainda não
# existir
function(AddReturnAfterComment file start_block return_statement)
  file(READ "${file}" FILE_CONTENTS)
  string(FIND "${FILE_CONTENTS}" "${start_block}\n    return;" POSITION)
  if(POSITION EQUAL -1)
    string(REPLACE "${start_block}" "${start_block}\n    return;" FILE_CONTENTS
                   "${FILE_CONTENTS}")
    file(WRITE ${file} "${FILE_CONTENTS}")
  endif()
endfunction()

addreturnaftercomment(
  ${PD_SOURCE_DIR}/s_inter.c
  "{       /* if there's no TK process just throw it to stderr */"
  "             return;")

# ╭──────────────────────────────────────╮
# │           PureData Sources           │
# ╰──────────────────────────────────────╯
set(PD_SOURCES
    "${PD_SOURCE_DIR}/d_arithmetic.c"
    "${PD_SOURCE_DIR}/d_array.c"
    "${PD_SOURCE_DIR}/d_ctl.c"
    "${PD_SOURCE_DIR}/d_dac.c"
    "${PD_SOURCE_DIR}/d_delay.c"
    "${PD_SOURCE_DIR}/d_fft.c"
    "${PD_SOURCE_DIR}/d_fft_fftsg.c"
    "${PD_SOURCE_DIR}/d_filter.c"
    "${PD_SOURCE_DIR}/d_global.c"
    "${PD_SOURCE_DIR}/d_math.c"
    "${PD_SOURCE_DIR}/d_misc.c"
    "${PD_SOURCE_DIR}/d_osc.c"
    "${PD_SOURCE_DIR}/d_resample.c"
    "${PD_SOURCE_DIR}/d_soundfile.c"
    "${PD_SOURCE_DIR}/d_soundfile_aiff.c"
    "${PD_SOURCE_DIR}/d_soundfile_caf.c"
    "${PD_SOURCE_DIR}/d_soundfile_next.c"
    "${PD_SOURCE_DIR}/d_soundfile_wave.c"
    "${PD_SOURCE_DIR}/d_ugen.c"
    "${PD_SOURCE_DIR}/g_all_guis.c"
    "${PD_SOURCE_DIR}/g_array.c"
    "${PD_SOURCE_DIR}/g_bang.c"
    "${PD_SOURCE_DIR}/g_canvas.c"
    "${PD_SOURCE_DIR}/g_clone.c"
    "${PD_SOURCE_DIR}/g_editor.c"
    "${PD_SOURCE_DIR}/g_editor_extras.c"
    "${PD_SOURCE_DIR}/g_graph.c"
    "${PD_SOURCE_DIR}/g_guiconnect.c"
    "${PD_SOURCE_DIR}/g_io.c"
    "${PD_SOURCE_DIR}/g_mycanvas.c"
    "${PD_SOURCE_DIR}/g_numbox.c"
    "${PD_SOURCE_DIR}/g_radio.c"
    "${PD_SOURCE_DIR}/g_readwrite.c"
    "${PD_SOURCE_DIR}/g_rtext.c"
    "${PD_SOURCE_DIR}/g_scalar.c"
    "${PD_SOURCE_DIR}/g_slider.c"
    "${PD_SOURCE_DIR}/g_template.c"
    "${PD_SOURCE_DIR}/g_text.c"
    "${PD_SOURCE_DIR}/g_toggle.c"
    "${PD_SOURCE_DIR}/g_traversal.c"
    "${PD_SOURCE_DIR}/g_undo.c"
    "${PD_SOURCE_DIR}/g_vumeter.c"
    "${PD_SOURCE_DIR}/m_atom.c"
    "${PD_SOURCE_DIR}/m_binbuf.c"
    "${PD_SOURCE_DIR}/m_class.c"
    "${PD_SOURCE_DIR}/m_conf.c"
    "${PD_SOURCE_DIR}/m_glob.c"
    "${PD_SOURCE_DIR}/m_memory.c"
    "${PD_SOURCE_DIR}/m_obj.c"
    "${PD_SOURCE_DIR}/m_pd.c"
    "${PD_SOURCE_DIR}/m_sched.c"
    "${PD_SOURCE_DIR}/s_audio.c"
    "${PD_SOURCE_DIR}/s_audio_dummy.c"
    "${PD_SOURCE_DIR}/s_inter.c"
    "${PD_SOURCE_DIR}/s_inter_gui.c"
    "${PD_SOURCE_DIR}/s_loader.c"
    "${PD_SOURCE_DIR}/s_main.c"
    "${PD_SOURCE_DIR}/s_net.c"
    "${PD_SOURCE_DIR}/s_path.c"
    "${PD_SOURCE_DIR}/s_print.c"
    "${PD_SOURCE_DIR}/s_utf8.c"
    "${PD_SOURCE_DIR}/x_acoustics.c"
    "${PD_SOURCE_DIR}/x_arithmetic.c"
    "${PD_SOURCE_DIR}/x_array.c"
    "${PD_SOURCE_DIR}/x_connective.c"
    "${PD_SOURCE_DIR}/x_file.c"
    "${PD_SOURCE_DIR}/x_gui.c"
    "${PD_SOURCE_DIR}/x_interface.c"
    "${PD_SOURCE_DIR}/x_list.c"
    "${PD_SOURCE_DIR}/x_midi.c"
    "${PD_SOURCE_DIR}/x_misc.c"
    "${PD_SOURCE_DIR}/x_net.c"
    "${PD_SOURCE_DIR}/x_scalar.c"
    "${PD_SOURCE_DIR}/x_text.c"
    "${PD_SOURCE_DIR}/x_time.c"
    "${PD_SOURCE_DIR}/x_vexp.c"
    "${PD_SOURCE_DIR}/x_vexp_fun.c"
    "${PD_SOURCE_DIR}/x_vexp_if.c")

function(pd_add_external_if_source_exists name source)
  if(EXISTS ${source})
    pd_add_external(${name} ${source})
  endif()
endfunction()

pd_add_external_if_source_exists(bob~ "${PD_SOURCE_DIR}/../extra/bob~/bob~.c")
pd_add_external_if_source_exists(bonk~ "${PD_SOURCE_DIR}/../extra/bonk~/bonk~.c")
pd_add_external_if_source_exists(choice "${PD_SOURCE_DIR}/../extra/choice/choice.c")
pd_add_external_if_source_exists(fiddle~ "${PD_SOURCE_DIR}/../extra/fiddle~/fiddle~.c")
pd_add_external_if_source_exists(loop~ "${PD_SOURCE_DIR}/../extra/loop~/loop~.c")
pd_add_external_if_source_exists(lrshift~ "${PD_SOURCE_DIR}/../extra/lrshift~/lrshift~.c")
pd_add_external_if_source_exists(pique "${PD_SOURCE_DIR}/../extra/pique/pique.c")
pd_add_external_if_source_exists(sigmund~ "${PD_SOURCE_DIR}/../extra/sigmund~/sigmund~.c")
# pd_add_external_if_source_exists(stdout
# ${PD_SOURCE_DIR}/../extra/stdout/stdout.c)

# ╭──────────────────────────────────────╮
# │            Libpd Sources             │
# ╰──────────────────────────────────────╯
set(LIBPD_MAIN_SOURCES
    "${PD_SOURCE_DIR}/x_libpdreceive.c" "${PD_SOURCE_DIR}/s_libpdmidi.c"
    "${PD_SOURCE_DIR}/z_hooks.c" "${PD_SOURCE_DIR}/z_libpd.c")
set(LIBPD_UTILS_SOURCES
    "${PD_SOURCE_DIR}/z_ringbuffer.c" "${PD_SOURCE_DIR}/z_print_util.c"
    "${PD_SOURCE_DIR}/z_queued.c")
set(LIBPD_SOURCES ${PD_SOURCES} ${LIBPD_MAIN_SOURCES} ${LIBPD_UTILS_SOURCES})

add_library(libpd STATIC ${LIBPD_SOURCES})
set_target_properties(libpd PROPERTIES OUTPUT_NAME pd)
target_compile_definitions(
  libpd PRIVATE -DPD=1 -DUSEAPI_DUMMY=1 -DHAVE_UNISTD_H=1 -DHAVE_LIBDL
                -DPD_INTERNAL)

target_compile_options(libpd PRIVATE -w)
target_compile_options(
  libpd
  PRIVATE $<$<CONFIG:Release>:-ffast-math> $<$<CONFIG:Release>:-funroll-loops>
          $<$<CONFIG:Release>:-fomit-frame-pointer> $<$<CONFIG:Release>:-O3>)
target_include_directories(libpd PRIVATE ${PD_SOURCE_DIR}/src)
