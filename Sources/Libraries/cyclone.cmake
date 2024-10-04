cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")

message(STATUS "PDCMAKE_DIR: ${PDCMAKE_DIR}")
include(${PDCMAKE_DIR}/pd.cmake)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/else STRING
            "PATH where is ROOT of else folder")

include_directories(${LIB_DIR}/Code_source/shared/aubio/src)
include_directories(${LIB_DIR}/Code_source/shared)

project(else)

set(ENABLE_TILDE_TARGET_WARNING off)

# ╭──────────────────────────────────────╮
# │            GUI INTERFACE             │
# ╰──────────────────────────────────────╯
pd_add_external(knob "${LIB_DIR}/Code_source/Compiled/control/knob.c")
pd_add_external(button "${LIB_DIR}/Code_source/Compiled/control/button.c")
pd_add_external(pic "${LIB_DIR}/Code_source/Compiled/control/pic.c")
pd_add_external(keyboard "${LIB_DIR}/Code_source/Compiled/control/keyboard.c")
pd_add_external(pad "${LIB_DIR}/Code_source/Compiled/control/pad.c")
pd_add_external(openfile "${LIB_DIR}/Code_source/Compiled/control/openfile.c")
pd_add_external(colors "${LIB_DIR}/Code_source/Compiled/control/colors.c")

# ╭──────────────────────────────────────╮
# │               CONTROL                │
# ╰──────────────────────────────────────╯
pd_add_external(args "${LIB_DIR}/Code_source/Compiled/control/args.c")
pd_add_external(ctl.in "${LIB_DIR}/Code_source/Compiled/control/ctl.in.c")
pd_add_external(ctl.out "${LIB_DIR}/Code_source/Compiled/control/ctl.out.c")
pd_add_external(ceil "${LIB_DIR}/Code_source/Compiled/control/ceil.c")
pd_add_external(suspedal "${LIB_DIR}/Code_source/Compiled/control/suspedal.c")
pd_add_external(voices "${LIB_DIR}/Code_source/Compiled/control/voices.c")
pd_add_external(buffer "${LIB_DIR}/Code_source/Compiled/control/buffer_obj.c")
pd_add_external(bicoeff "${LIB_DIR}/Code_source/Compiled/control/bicoeff.c")
pd_add_external(bicoeff2 "${LIB_DIR}/Code_source/Compiled/control/bicoeff2.c")
pd_add_external(click "${LIB_DIR}/Code_source/Compiled/control/click.c")
pd_add_external(canvas.active "${LIB_DIR}/Code_source/Compiled/control/canvas.active.c")
pd_add_external(canvas.bounds "${LIB_DIR}/Code_source/Compiled/control/canvas.bounds.c")
pd_add_external(canvas.edit "${LIB_DIR}/Code_source/Compiled/control/canvas.edit.c")
pd_add_external(canvas.gop "${LIB_DIR}/Code_source/Compiled/control/canvas.gop.c")
pd_add_external(canvas.mouse "${LIB_DIR}/Code_source/Compiled/control/canvas.mouse.c")
pd_add_external(canvas.name "${LIB_DIR}/Code_source/Compiled/control/canvas.name.c")
pd_add_external(canvas.pos "${LIB_DIR}/Code_source/Compiled/control/canvas.pos.c")
pd_add_external(canvas.setname "${LIB_DIR}/Code_source/Compiled/control/canvas.setname.c")
pd_add_external(canvas.vis "${LIB_DIR}/Code_source/Compiled/control/canvas.vis.c")
pd_add_external(canvas.zoom "${LIB_DIR}/Code_source/Compiled/control/canvas.zoom.c")
pd_add_external(properties "${LIB_DIR}/Code_source/Compiled/control/properties.c")
pd_add_external(break "${LIB_DIR}/Code_source/Compiled/control/break.c")
pd_add_external(cents2ratio "${LIB_DIR}/Code_source/Compiled/control/cents2ratio.c")
pd_add_external(changed "${LIB_DIR}/Code_source/Compiled/control/changed.c")
pd_add_external(gcd "${LIB_DIR}/Code_source/Compiled/control/gcd.c")
pd_add_external(dir "${LIB_DIR}/Code_source/Compiled/control/dir.c")
pd_add_external(datetime "${LIB_DIR}/Code_source/Compiled/control/datetime.c")
pd_add_external(default "${LIB_DIR}/Code_source/Compiled/control/default.c")
pd_add_external(dollsym "${LIB_DIR}/Code_source/Compiled/control/dollsym.c")
pd_add_external(floor "${LIB_DIR}/Code_source/Compiled/control/floor.c")
pd_add_external(fold "${LIB_DIR}/Code_source/Compiled/control/fold.c")
pd_add_external(hot "${LIB_DIR}/Code_source/Compiled/control/hot.c")
pd_add_external(hz2rad "${LIB_DIR}/Code_source/Compiled/control/hz2rad.c")
pd_add_external(initmess "${LIB_DIR}/Code_source/Compiled/control/initmess.c")
pd_add_external(keycode "${LIB_DIR}/Code_source/Compiled/control/keycode.c")
pd_add_external(lb "${LIB_DIR}/Code_source/Compiled/extra_source/Aliases/lb.c")
pd_add_external(limit "${LIB_DIR}/Code_source/Compiled/control/limit.c")
pd_add_external(loadbanger "${LIB_DIR}/Code_source/Compiled/control/loadbanger.c")
pd_add_external(merge "${LIB_DIR}/Code_source/Compiled/control/merge.c")
pd_add_external(fontsize "${LIB_DIR}/Code_source/Compiled/control/fontsize.c")
pd_add_external(format "${LIB_DIR}/Code_source/Compiled/control/format.c")
pd_add_external(message "${LIB_DIR}/Code_source/Compiled/control/message.c")
pd_add_external(msgbox "${LIB_DIR}/Code_source/Compiled/control/messbox.c")
pd_add_external(metronome "${LIB_DIR}/Code_source/Compiled/control/metronome.c")
pd_add_external(mouse "${LIB_DIR}/Code_source/Compiled/control/mouse.c")
pd_add_external(mpe.in "${LIB_DIR}/Code_source/Compiled/control/mpe.in.c")
pd_add_external(noteinfo "${LIB_DIR}/Code_source/Compiled/control/noteinfo.c")
pd_add_external(note.in "${LIB_DIR}/Code_source/Compiled/control/note.in.c")
pd_add_external(note.out "${LIB_DIR}/Code_source/Compiled/control/note.out.c")
pd_add_external(order "${LIB_DIR}/Code_source/Compiled/control/order.c")
pd_add_external(osc.route "${LIB_DIR}/Code_source/Compiled/control/osc.route.c")
pd_add_external(osc.parse "${LIB_DIR}/Code_source/Compiled/control/osc.parse.c")
pd_add_external(osc.format "${LIB_DIR}/Code_source/Compiled/control/osc.format.c")
pd_add_external(factor "${LIB_DIR}/Code_source/Compiled/control/factor.c")
pd_add_external(float2bits "${LIB_DIR}/Code_source/Compiled/control/float2bits.c")
pd_add_external(panic "${LIB_DIR}/Code_source/Compiled/control/panic.c")
pd_add_external(pgm.in "${LIB_DIR}/Code_source/Compiled/control/pgm.in.c")
pd_add_external(pgm.out "${LIB_DIR}/Code_source/Compiled/control/pgm.out.c")
pd_add_external(pipe2 "${LIB_DIR}/Code_source/Compiled/control/pipe2.c")
pd_add_external(bend.in "${LIB_DIR}/Code_source/Compiled/control/bend.in.c")
pd_add_external(bend.out "${LIB_DIR}/Code_source/Compiled/control/bend.out.c")
pd_add_external(pack2 "${LIB_DIR}/Code_source/Compiled/control/pack2.c")
pd_add_external(quantizer "${LIB_DIR}/Code_source/Compiled/control/quantizer.c")
pd_add_external(rad2hz "${LIB_DIR}/Code_source/Compiled/control/rad2hz.c")
pd_add_external(ratio2cents "${LIB_DIR}/Code_source/Compiled/control/ratio2cents.c")
pd_add_external(rescale "${LIB_DIR}/Code_source/Compiled/control/rescale.c")
pd_add_external(rint "${LIB_DIR}/Code_source/Compiled/control/rint.c")
pd_add_external(router "${LIB_DIR}/Code_source/Compiled/control/router.c")
pd_add_external(route2 "${LIB_DIR}/Code_source/Compiled/control/route2.c")
pd_add_external(routeall "${LIB_DIR}/Code_source/Compiled/control/routeall.c")
pd_add_external(routetype "${LIB_DIR}/Code_source/Compiled/control/routetype.c")
pd_add_external(receiver "${LIB_DIR}/Code_source/Compiled/control/receiver.c")
pd_add_external(retrieve "${LIB_DIR}/Code_source/Compiled/control/retrieve.c")
pd_add_external(selector "${LIB_DIR}/Code_source/Compiled/control/selector.c")
pd_add_external(sender "${LIB_DIR}/Code_source/Compiled/control/sender.c")
pd_add_external(separate "${LIB_DIR}/Code_source/Compiled/control/separate.c")
pd_add_external(symbol2any "${LIB_DIR}/Code_source/Compiled/control/symbol2any.c")
pd_add_external(slice "${LIB_DIR}/Code_source/Compiled/control/slice.c")
pd_add_external(sort "${LIB_DIR}/Code_source/Compiled/control/sort.c")
pd_add_external(spread "${LIB_DIR}/Code_source/Compiled/control/spread.c")
pd_add_external(touch.in "${LIB_DIR}/Code_source/Compiled/control/touch.in.c")
pd_add_external(touch.out "${LIB_DIR}/Code_source/Compiled/control/touch.out.c")
pd_add_external(ptouch.in "${LIB_DIR}/Code_source/Compiled/control/ptouch.in.c")
pd_add_external(ptouch.out "${LIB_DIR}/Code_source/Compiled/control/ptouch.out.c")
pd_add_external(trunc "${LIB_DIR}/Code_source/Compiled/control/trunc.c")
pd_add_external(unmerge "${LIB_DIR}/Code_source/Compiled/control/unmerge.c")
pd_add_external(var "${LIB_DIR}/Code_source/Compiled/control/var.c")

# ╭──────────────────────────────────────╮
# │                SIGNAL                │
# ╰──────────────────────────────────────╯
pd_add_external(above~ "${LIB_DIR}/Code_source/Compiled/audio/above~.c")
pd_add_external(add~ "${LIB_DIR}/Code_source/Compiled/audio/add~.c")
pd_add_external(allpass.2nd~ "${LIB_DIR}/Code_source/Compiled/audio/allpass.2nd~.c")
pd_add_external(allpass.rev~ "${LIB_DIR}/Code_source/Compiled/audio/allpass.rev~.c")
pd_add_external(bitnormal~ "${LIB_DIR}/Code_source/Compiled/audio/bitnormal~.c")
pd_add_external(comb.rev~ "${LIB_DIR}/Code_source/Compiled/audio/comb.rev~.c")
pd_add_external(comb.filt~ "${LIB_DIR}/Code_source/Compiled/audio/comb.filt~.c")
pd_add_external(adsr~ "${LIB_DIR}/Code_source/Compiled/audio/adsr~.c")
pd_add_external(asr~ "${LIB_DIR}/Code_source/Compiled/audio/asr~.c")
pd_add_external(bandpass~ "${LIB_DIR}/Code_source/Compiled/audio/bandpass~.c")
pd_add_external(bandstop~ "${LIB_DIR}/Code_source/Compiled/audio/bandstop~.c")
pd_add_external(bl.imp~ "${LIB_DIR}/Code_source/Compiled/audio/bl.imp~.c")
pd_add_external(bl.imp2~ "${LIB_DIR}/Code_source/Compiled/audio/bl.imp2~.c")
pd_add_external(bl.saw~ "${LIB_DIR}/Code_source/Compiled/audio/bl.saw~.c")
pd_add_external(bl.saw2~ "${LIB_DIR}/Code_source/Compiled/audio/bl.saw2~.c")
pd_add_external(bl.square~ "${LIB_DIR}/Code_source/Compiled/audio/bl.square~.c")
pd_add_external(bl.tri~ "${LIB_DIR}/Code_source/Compiled/audio/bl.tri~.c")
pd_add_external(bl.vsaw~ "${LIB_DIR}/Code_source/Compiled/audio/bl.vsaw~.c")
pd_add_external(blocksize~ "${LIB_DIR}/Code_source/Compiled/audio/blocksize~.c")
pd_add_external(biquads~ "${LIB_DIR}/Code_source/Compiled/audio/biquads~.c")
pd_add_external(car2pol~ "${LIB_DIR}/Code_source/Compiled/audio/car2pol~.c")
pd_add_external(ceil~ "${LIB_DIR}/Code_source/Compiled/audio/ceil~.c")
pd_add_external(cents2ratio~ "${LIB_DIR}/Code_source/Compiled/audio/cents2ratio~.c")
pd_add_external(changed~ "${LIB_DIR}/Code_source/Compiled/audio/changed~.c")
pd_add_external(changed2~ "${LIB_DIR}/Code_source/Compiled/audio/changed2~.c")
pd_add_external(conv~ "${LIB_DIR}/Code_source/Compiled/audio/conv~.c")
pd_add_external(crackle~ "${LIB_DIR}/Code_source/Compiled/audio/crackle~.c")
pd_add_external(crossover~ "${LIB_DIR}/Code_source/Compiled/audio/crossover~.c")
pd_add_external(cusp~ "${LIB_DIR}/Code_source/Compiled/audio/cusp~.c")
pd_add_external(db2lin~ "${LIB_DIR}/Code_source/Compiled/audio/db2lin~.c")
pd_add_external(decay~ "${LIB_DIR}/Code_source/Compiled/audio/decay~.c")
pd_add_external(decay2~ "${LIB_DIR}/Code_source/Compiled/audio/decay2~.c")
pd_add_external(downsample~ "${LIB_DIR}/Code_source/Compiled/audio/downsample~.c")
pd_add_external(drive~ "${LIB_DIR}/Code_source/Compiled/audio/drive~.c")
pd_add_external(detect~ "${LIB_DIR}/Code_source/Compiled/audio/detect~.c")
pd_add_external(envgen~ "${LIB_DIR}/Code_source/Compiled/audio/envgen~.c")
pd_add_external(eq~ "${LIB_DIR}/Code_source/Compiled/audio/eq~.c")
pd_add_external(fbsine2~ "${LIB_DIR}/Code_source/Compiled/audio/fbsine2~.c")
pd_add_external(fdn.rev~ "${LIB_DIR}/Code_source/Compiled/audio/fdn.rev~.c")
pd_add_external(floor~ "${LIB_DIR}/Code_source/Compiled/audio/floor~.c")
pd_add_external(fold~ "${LIB_DIR}/Code_source/Compiled/audio/fold~.c")
pd_add_external(freq.shift~ "${LIB_DIR}/Code_source/Compiled/audio/freq.shift~.c")
pd_add_external(gbman~ "${LIB_DIR}/Code_source/Compiled/audio/gbman~.c")
pd_add_external(gate2imp~ "${LIB_DIR}/Code_source/Compiled/audio/gate2imp~.c")
pd_add_external(get~ "${LIB_DIR}/Code_source/Compiled/audio/get~.c")
pd_add_external(giga.rev~ "${LIB_DIR}/Code_source/Compiled/audio/giga.rev~.c")
pd_add_external(glide~ "${LIB_DIR}/Code_source/Compiled/audio/glide~.c")
pd_add_external(glide2~ "${LIB_DIR}/Code_source/Compiled/audio/glide2~.c")
pd_add_external(henon~ "${LIB_DIR}/Code_source/Compiled/audio/henon~.c")
pd_add_external(highpass~ "${LIB_DIR}/Code_source/Compiled/audio/highpass~.c")
pd_add_external(highshelf~ "${LIB_DIR}/Code_source/Compiled/audio/highshelf~.c")
pd_add_external(ikeda~ "${LIB_DIR}/Code_source/Compiled/audio/ikeda~.c")
pd_add_external(impseq~ "${LIB_DIR}/Code_source/Compiled/audio/impseq~.c")
pd_add_external(trunc~ "${LIB_DIR}/Code_source/Compiled/audio/trunc~.c")
pd_add_external(lastvalue~ "${LIB_DIR}/Code_source/Compiled/audio/lastvalue~.c")
pd_add_external(latoocarfian~ "${LIB_DIR}/Code_source/Compiled/audio/latoocarfian~.c")
pd_add_external(lorenz~ "${LIB_DIR}/Code_source/Compiled/audio/lorenz~.c")
pd_add_external(lincong~ "${LIB_DIR}/Code_source/Compiled/audio/lincong~.c")
pd_add_external(lin2db~ "${LIB_DIR}/Code_source/Compiled/audio/lin2db~.c")
pd_add_external(logistic~ "${LIB_DIR}/Code_source/Compiled/audio/logistic~.c")
pd_add_external(loop "${LIB_DIR}/Code_source/Compiled/control/loop.c")
pd_add_external(lop2~ "${LIB_DIR}/Code_source/Compiled/audio/lop2~.c")
pd_add_external(lowpass~ "${LIB_DIR}/Code_source/Compiled/audio/lowpass~.c")
pd_add_external(lowshelf~ "${LIB_DIR}/Code_source/Compiled/audio/lowshelf~.c")
pd_add_external(mov.rms~ "${LIB_DIR}/Code_source/Compiled/audio/mov.rms~.c")
pd_add_external(mtx~ "${LIB_DIR}/Code_source/Compiled/audio/mtx~.c")
pd_add_external(mtx.mc~ "${LIB_DIR}/Code_source/Compiled/audio/mtx.mc~.c")
pd_add_external(match~ "${LIB_DIR}/Code_source/Compiled/audio/match~.c")
pd_add_external(mov.avg~ "${LIB_DIR}/Code_source/Compiled/audio/mov.avg~.c")
pd_add_external(median~ "${LIB_DIR}/Code_source/Compiled/audio/median~.c")
pd_add_external(merge~ "${LIB_DIR}/Code_source/Compiled/audio/merge~.c")
pd_add_external(nchs~ "${LIB_DIR}/Code_source/Compiled/audio/nchs~.c")
pd_add_external(nyquist~ "${LIB_DIR}/Code_source/Compiled/audio/nyquist~.c")
pd_add_external(op~ "${LIB_DIR}/Code_source/Compiled/audio/op~.c")
pd_add_external(pol2car~ "${LIB_DIR}/Code_source/Compiled/audio/pol2car~.c")
pd_add_external(power~ "${LIB_DIR}/Code_source/Compiled/audio/power~.c")
pd_add_external(peak~ "${LIB_DIR}/Code_source/Compiled/audio/peak~.c")
pd_add_external(phaseseq~ "${LIB_DIR}/Code_source/Compiled/audio/phaseseq~.c")
pd_add_external(pulsecount~ "${LIB_DIR}/Code_source/Compiled/audio/pulsecount~.c")
pd_add_external(pick~ "${LIB_DIR}/Code_source/Compiled/audio/pick~.c")
pd_add_external(pimpmul~ "${LIB_DIR}/Code_source/Compiled/audio/pimpmul~.c")
pd_add_external(pulsediv~ "${LIB_DIR}/Code_source/Compiled/audio/pulsediv~.c")
pd_add_external(quad~ "${LIB_DIR}/Code_source/Compiled/audio/quad~.c")
pd_add_external(quantizer~ "${LIB_DIR}/Code_source/Compiled/audio/quantizer~.c")
pd_add_external(ramp~ "${LIB_DIR}/Code_source/Compiled/audio/ramp~.c")
pd_add_external(range~ "${LIB_DIR}/Code_source/Compiled/audio/range~.c")
pd_add_external(ratio2cents~ "${LIB_DIR}/Code_source/Compiled/audio/ratio2cents~.c")
pd_add_external(rescale~ "${LIB_DIR}/Code_source/Compiled/audio/rescale~.c")
pd_add_external(rint~ "${LIB_DIR}/Code_source/Compiled/audio/rint~.c")
pd_add_external(repeat~ "${LIB_DIR}/Code_source/Compiled/audio/repeat~.c")
pd_add_external(resonant~ "${LIB_DIR}/Code_source/Compiled/audio/resonant~.c")
pd_add_external(resonant2~ "${LIB_DIR}/Code_source/Compiled/audio/resonant2~.c")
pd_add_external(rms~ "${LIB_DIR}/Code_source/Compiled/audio/rms~.c")
pd_add_external(sh~ "${LIB_DIR}/Code_source/Compiled/audio/sh~.c")
pd_add_external(schmitt~ "${LIB_DIR}/Code_source/Compiled/audio/schmitt~.c")
pd_add_external(slice~ "${LIB_DIR}/Code_source/Compiled/audio/slice~.c")
pd_add_external(lag~ "${LIB_DIR}/Code_source/Compiled/audio/lag~.c")
pd_add_external(lag2~ "${LIB_DIR}/Code_source/Compiled/audio/lag2~.c")
pd_add_external(sig2float~ "${LIB_DIR}/Code_source/Compiled/audio/sig2float~.c")
pd_add_external(slew~ "${LIB_DIR}/Code_source/Compiled/audio/slew~.c")
pd_add_external(slew2~ "${LIB_DIR}/Code_source/Compiled/audio/slew2~.c")
pd_add_external(s2f~ "${LIB_DIR}/Code_source/Compiled/extra_source/Aliases/s2f~.c")
pd_add_external(sequencer~ "${LIB_DIR}/Code_source/Compiled/audio/sequencer~.c")
pd_add_external(select~ "${LIB_DIR}/Code_source/Compiled/audio/select~.c")
pd_add_external(sr~ "${LIB_DIR}/Code_source/Compiled/audio/sr~.c")
pd_add_external(status~ "${LIB_DIR}/Code_source/Compiled/audio/status~.c")
pd_add_external(standard~ "${LIB_DIR}/Code_source/Compiled/audio/standard~.c")
pd_add_external(sum~ "${LIB_DIR}/Code_source/Compiled/audio/sum~.c")
pd_add_external(sigs~ "${LIB_DIR}/Code_source/Compiled/audio/sigs~.c")
pd_add_external(susloop~ "${LIB_DIR}/Code_source/Compiled/audio/susloop~.c")
pd_add_external(svfilter~ "${LIB_DIR}/Code_source/Compiled/audio/svfilter~.c")
pd_add_external(trig.delay~ "${LIB_DIR}/Code_source/Compiled/audio/trig.delay~.c")
pd_add_external(trig.delay2~ "${LIB_DIR}/Code_source/Compiled/audio/trig.delay2~.c")
pd_add_external(timed.gate~ "${LIB_DIR}/Code_source/Compiled/audio/timed.gate~.c")
pd_add_external(toggleff~ "${LIB_DIR}/Code_source/Compiled/audio/toggleff~.c")
pd_add_external(trighold~ "${LIB_DIR}/Code_source/Compiled/audio/trighold~.c")
pd_add_external(unmerge~ "${LIB_DIR}/Code_source/Compiled/audio/unmerge~.c")
pd_add_external(vu~ "${LIB_DIR}/Code_source/Compiled/audio/vu~.c")
# pd_add_external(vcf2~ "${LIB_DIR}/Code_source/Compiled/audio/vcf2~.c")
pd_add_external(xmod~ "${LIB_DIR}/Code_source/Compiled/audio/xmod~.c")
pd_add_external(xmod2~ "${LIB_DIR}/Code_source/Compiled/audio/xmod2~.c")
pd_add_external(wrap2 "${LIB_DIR}/Code_source/Compiled/control/wrap2.c")
pd_add_external(wrap2~ "${LIB_DIR}/Code_source/Compiled/audio/wrap2~.c")
pd_add_external(zerocross~ "${LIB_DIR}/Code_source/Compiled/audio/zerocross~.c")

# ╭──────────────────────────────────────╮
# │                AUBIO                 │
# ╰──────────────────────────────────────╯
file(GLOB_RECURSE AUBIO_SRC1 "${LIB_DIR}/Code_source/shared/aubio/src/*/*.c")
file(GLOB_RECURSE AUBIO_SRC2 "${LIB_DIR}/Code_source/shared/aubio/src/*.c")
file(GLOB_RECURSE AUBIO_SRC3 "${LIB_DIR}/Code_source/Compiled/audio/beat~.c")
pd_add_external(beat~ "${AUBIO_SRC1};${AUBIO_SRC2};${AUBIO_SRC3}")

# ╭──────────────────────────────────────╮
# │                MAGIC                 │
# ╰──────────────────────────────────────╯
set(MAGIC_CODE "${LIB_DIR}/Code_source/shared/magic.c")

pd_add_external(gaussian~ "${LIB_DIR}/Code_source/Compiled/audio/gaussian~.c;${MAGIC_CODE}")
pd_add_external(imp~ "${LIB_DIR}/Code_source/Compiled/extra_source/Aliases/imp~.c;${MAGIC_CODE}")
pd_add_external(impulse~ "${LIB_DIR}/Code_source/Compiled/audio/impulse~.c;${MAGIC_CODE}")
pd_add_external(imp2~ "${LIB_DIR}/Code_source/Compiled/extra_source/Aliases/imp2~.c;${MAGIC_CODE}")
pd_add_external(impulse2~ "${LIB_DIR}/Code_source/Compiled/audio/impulse2~.c;${MAGIC_CODE}")
pd_add_external(parabolic~ "${LIB_DIR}/Code_source/Compiled/audio/parabolic~.c;${MAGIC_CODE}")
pd_add_external(pulse~ "${LIB_DIR}/Code_source/Compiled/audio/pulse~.c;${MAGIC_CODE}")
pd_add_external(saw~ "${LIB_DIR}/Code_source/Compiled/audio/saw~.c;${MAGIC_CODE}")
pd_add_external(saw2~ "${LIB_DIR}/Code_source/Compiled/audio/saw2~.c;${MAGIC_CODE}")
pd_add_external(square~ "${LIB_DIR}/Code_source/Compiled/audio/square~.c;${MAGIC_CODE}")
pd_add_external(tri~ "${LIB_DIR}/Code_source/Compiled/audio/tri~.c;${MAGIC_CODE}")
pd_add_external(vsaw~ "${LIB_DIR}/Code_source/Compiled/audio/vsaw~.c;${MAGIC_CODE}")
pd_add_external(pimp~ "${LIB_DIR}/Code_source/Compiled/audio/pimp~.c;${MAGIC_CODE}")
pd_add_external(numbox~ "${LIB_DIR}/Code_source/Compiled/audio/numbox~.c;${MAGIC_CODE}")

# ╭──────────────────────────────────────╮
# │                BUFFER                │
# ╰──────────────────────────────────────╯
set(ELSE_BUFFER "${LIB_DIR}/Code_source/shared/buffer.c")

pd_add_external(fader~ "${LIB_DIR}/Code_source/Compiled/audio/fader~.c;${ELSE_BUFFER}")
pd_add_external(autofade~ "${LIB_DIR}/Code_source/Compiled/audio/autofade~.c;${ELSE_BUFFER}")
pd_add_external(autofade.mc~ "${LIB_DIR}/Code_source/Compiled/audio/autofade.mc~.c;${ELSE_BUFFER}")
pd_add_external(autofade2~ "${LIB_DIR}/Code_source/Compiled/audio/autofade2~.c;${ELSE_BUFFER}")
pd_add_external(autofade2.mc~
                "${LIB_DIR}/Code_source/Compiled/audio/autofade2.mc~.c;${ELSE_BUFFER}")
pd_add_external(balance~ "${LIB_DIR}/Code_source/Compiled/audio/balance~.c;${ELSE_BUFFER}")
pd_add_external(pan~ "${LIB_DIR}/Code_source/Compiled/audio/pan~.c;${ELSE_BUFFER}")
pd_add_external(pan.mc~ "${LIB_DIR}/Code_source/Compiled/audio/pan.mc~.c;${ELSE_BUFFER}")
pd_add_external(pan2~ "${LIB_DIR}/Code_source/Compiled/audio/pan2~.c;${ELSE_BUFFER}")
pd_add_external(pan4~ "${LIB_DIR}/Code_source/Compiled/audio/pan4~.c;${ELSE_BUFFER}")
pd_add_external(rotate~ "${LIB_DIR}/Code_source/Compiled/audio/rotate~.c;${ELSE_BUFFER}")
pd_add_external(rotate.mc~ "${LIB_DIR}/Code_source/Compiled/audio/rotate.mc~.c;${ELSE_BUFFER}")
pd_add_external(spread~ "${LIB_DIR}/Code_source/Compiled/audio/spread~.c;${ELSE_BUFFER}")
pd_add_external(spread.mc~ "${LIB_DIR}/Code_source/Compiled/audio/spread.mc~.c;${ELSE_BUFFER}")
pd_add_external(xfade~ "${LIB_DIR}/Code_source/Compiled/audio/xfade~.c;${ELSE_BUFFER}")
pd_add_external(xfade.mc~ "${LIB_DIR}/Code_source/Compiled/audio/xfade.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate~ "${LIB_DIR}/Code_source/Compiled/audio/xgate~.c;${ELSE_BUFFER}")
pd_add_external(xgate.mc~ "${LIB_DIR}/Code_source/Compiled/audio/xgate.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate2.mc~ "${LIB_DIR}/Code_source/Compiled/audio/xgate2.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate2~ "${LIB_DIR}/Code_source/Compiled/audio/xgate2~.c;${ELSE_BUFFER}")
pd_add_external(xselect~ "${LIB_DIR}/Code_source/Compiled/audio/xselect~.c;${ELSE_BUFFER}")
pd_add_external(xselect.mc~ "${LIB_DIR}/Code_source/Compiled/audio/xselect.mc~.c;${ELSE_BUFFER}")
pd_add_external(xselect2~ "${LIB_DIR}/Code_source/Compiled/audio/xselect2~.c;${ELSE_BUFFER}")
pd_add_external(xselect2.mc~ "${LIB_DIR}/Code_source/Compiled/audio/xselect2.mc~.c;${ELSE_BUFFER}")
pd_add_external(sin~ "${LIB_DIR}/Code_source/Compiled/audio/sin~.c;${ELSE_BUFFER}")
pd_add_external(fm~ "${LIB_DIR}/Code_source/Compiled/audio/fm~.c;${ELSE_BUFFER}")
pd_add_external(pm~ "${LIB_DIR}/Code_source/Compiled/audio/pm~.c;${ELSE_BUFFER}")
pd_add_external(pm2~ "${LIB_DIR}/Code_source/Compiled/audio/pm2~.c;${ELSE_BUFFER}")
pd_add_external(pm4~ "${LIB_DIR}/Code_source/Compiled/audio/pm4~.c;${ELSE_BUFFER}")
pd_add_external(pm6~ "${LIB_DIR}/Code_source/Compiled/audio/pm6~.c;${ELSE_BUFFER}")
pd_add_external(shaper~ "${LIB_DIR}/Code_source/Compiled/audio/shaper~.c;${ELSE_BUFFER}")
pd_add_external(tabreader "${LIB_DIR}/Code_source/Compiled/control/tabreader.c;${ELSE_BUFFER}")
pd_add_external(tabreader~ "${LIB_DIR}/Code_source/Compiled/audio/tabreader~.c;${ELSE_BUFFER}")
pd_add_external(function "${LIB_DIR}/Code_source/Compiled/control/function.c;${ELSE_BUFFER}")
pd_add_external(function~ "${LIB_DIR}/Code_source/Compiled/audio/function~.c;${ELSE_BUFFER}")
pd_add_external(tabwriter~ "${LIB_DIR}/Code_source/Compiled/audio/tabwriter~.c;${ELSE_BUFFER}")
pd_add_external(del~ "${LIB_DIR}/Code_source/Compiled/audio/del~.c;${ELSE_BUFFER}")
pd_add_external(fbdelay~ "${LIB_DIR}/Code_source/Compiled/audio/fbdelay~.c;${ELSE_BUFFER}")
pd_add_external(ffdelay~ "${LIB_DIR}/Code_source/Compiled/audio/ffdelay~.c;${ELSE_BUFFER}")
pd_add_external(filterdelay~ "${LIB_DIR}/Code_source/Compiled/audio/filterdelay~.c;${ELSE_BUFFER}")

# ╭──────────────────────────────────────╮
# │               BUFMAGIC               │
# ╰──────────────────────────────────────╯
list(APPEND bufmagic ${ELSE_BUFFER})
list(APPEND bufmagic ${MAGIC_CODE})

pd_add_external(cosine~ "${LIB_DIR}/Code_source/Compiled/audio/cosine~.c;${bufmagic}")
pd_add_external(fbsine~ "${LIB_DIR}/Code_source/Compiled/audio/fbsine~.c;${bufmagic}")
pd_add_external(sine~ "${LIB_DIR}/Code_source/Compiled/audio/sine~.c;${bufmagic}")
pd_add_external(wavetable~ "${LIB_DIR}/Code_source/Compiled/audio/wavetable~.c;${bufmagic}")
pd_add_external(wt~ "${LIB_DIR}/Code_source/Compiled/extra_source/Aliases/wt~.c;${bufmagic}")
pd_add_external(wt2d~ "${LIB_DIR}/Code_source/Compiled/audio/wt2d~.c;${bufmagic}")
pd_add_external(tabplayer~ "${LIB_DIR}/Code_source/Compiled/audio/tabplayer~.c;${bufmagic}")

# ╭──────────────────────────────────────╮
# │               RANDBUF                │
# ╰──────────────────────────────────────╯
list(APPEND randbuf "${LIB_DIR}/Code_source/shared/random.c")
list(APPEND randbuf "${LIB_DIR}/Code_source/shared/buffer.c")

pd_add_external(gendyn~ "${LIB_DIR}/Code_source/Compiled/audio/gendyn~.c;${randbuf}")

# ╭──────────────────────────────────────╮
# │              RANDMAGIC               │
# ╰──────────────────────────────────────╯
list(APPEND randmagic ${MAGIC_CODE})
list(APPEND randmagic "${LIB_DIR}/Code_source/shared/random.c")
pd_add_external(brown~ "${LIB_DIR}/Code_source/Compiled/audio/brown~.c;${randmagic}")

# ╭──────────────────────────────────────╮
# │                 RAND                 │
# ╰──────────────────────────────────────╯
set(rand "${LIB_DIR}/Code_source/shared/random.c")

pd_add_external(white~ "${LIB_DIR}/Code_source/Compiled/audio/white~.c;${rand}")
pd_add_external(pink~ "${LIB_DIR}/Code_source/Compiled/audio/pink~.c;${rand}")
pd_add_external(gray~ "${LIB_DIR}/Code_source/Compiled/audio/gray~.c;${rand}")
pd_add_external(pluck~ "${LIB_DIR}/Code_source/Compiled/audio/pluck~.c;${rand}")
pd_add_external(rand.u "${LIB_DIR}/Code_source/Compiled/control/rand.u.c;${rand}")
pd_add_external(rand.hist "${LIB_DIR}/Code_source/Compiled/control/rand.hist.c;${rand}")
pd_add_external(rand.i "${LIB_DIR}/Code_source/Compiled/control/rand.i.c;${rand}")
pd_add_external(rand.i~ "${LIB_DIR}/Code_source/Compiled/audio/rand.i~.c;${rand}")
pd_add_external(rand.f "${LIB_DIR}/Code_source/Compiled/control/rand.f.c;${rand}")
pd_add_external(rand.f~ "${LIB_DIR}/Code_source/Compiled/audio/rand.f~.c;${rand}")
pd_add_external(randpulse~ "${LIB_DIR}/Code_source/Compiled/audio/randpulse~.c;${rand}")
pd_add_external(randpulse2~ "${LIB_DIR}/Code_source/Compiled/audio/randpulse2~.c;${rand}")
pd_add_external(lfnoise~ "${LIB_DIR}/Code_source/Compiled/audio/lfnoise~.c;${rand}")
pd_add_external(rampnoise~ "${LIB_DIR}/Code_source/Compiled/audio/rampnoise~.c;${rand}")
pd_add_external(stepnoise~ "${LIB_DIR}/Code_source/Compiled/audio/stepnoise~.c;${rand}")
pd_add_external(dust~ "${LIB_DIR}/Code_source/Compiled/audio/dust~.c;${rand}")
pd_add_external(dust2~ "${LIB_DIR}/Code_source/Compiled/audio/dust2~.c;${rand}")
pd_add_external(chance "${LIB_DIR}/Code_source/Compiled/control/chance.c;${rand}")
pd_add_external(chance~ "${LIB_DIR}/Code_source/Compiled/audio/chance~.c;${rand}")
pd_add_external(tempo~ "${LIB_DIR}/Code_source/Compiled/audio/tempo~.c;${rand}")

# ╭──────────────────────────────────────╮
# │                 MIDI                 │
# ╰──────────────────────────────────────╯
list(APPEND midi_src "${LIB_DIR}/Code_source/shared/mifi.c"
     "${LIB_DIR}/Code_source/shared/elsefile.c")
pd_add_external(midi "${LIB_DIR}/Code_source/Compiled/control/midi.c;${midi_src}")

# ╭──────────────────────────────────────╮
# │                 FILE                 │
# ╰──────────────────────────────────────╯
pd_add_external(
    rec "${LIB_DIR}/Code_source/Compiled/control/rec.c;${LIB_DIR}/Code_source/shared/elsefile.c")

# ╭──────────────────────────────────────╮
# │                SMAGIC                │
# ╰──────────────────────────────────────╯
set(magic "${LIB_DIR}/Code_source/shared/magic.c")
pd_add_external(oscope~ "${LIB_DIR}/Code_source/Compiled/audio/oscope~.c;${magic}")

# ╭──────────────────────────────────────╮
# │                 UTF                  │
# ╰──────────────────────────────────────╯
set(utf "${LIB_DIR}/Code_source/shared/s_elseutf8.c")
pd_add_external(note "${LIB_DIR}/Code_source/Compiled/control/note.c;${utf}")

# ╭──────────────────────────────────────╮
# │                PLAITS                │
# ╰──────────────────────────────────────╯
file(GLOB_RECURSE PLAITS_SRC "${LIB_DIR}/Code_source/Compiled/audio/plaits~/*.cc")
list(APPEND PLAITS_SRC "${LIB_DIR}/Code_source/Compiled/audio/plaits~/plaits~.cpp")
include_directories("${LIB_DIR}/Code_source/Compiled/audio/plaits~/")
pd_add_external(plaits~ "${PLAITS_SRC}")

# ╭──────────────────────────────────────╮
# │                SFONT                 │
# ╰──────────────────────────────────────╯
if(NOT PD4WEB)
    set(BUILD_SFONT ON)
    if(BUILD_SFONT)

        list(APPEND SFONT_SRC "${LIB_DIR}/Code_source/Compiled/audio/sfont~/sfont~.c")
        list(APPEND SFONT_SRC "${LIB_DIR}/Code_source/shared/elsefile.c")

        # ─────────── SFONT LIBRARIES ─────────
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
        set(BUILD_SHARED_LIBS OFF)

        set(enable-aufile OFF)
        set(enable-dbus OFF)
        set(enable-ipv6 OFF)
        set(enable-jack OFF)
        set(enable-ladspa OFF)
        set(enable-libinstpatch OFF)
        set(enable-libsndfile OFF)
        set(enable-midishare OFF)
        set(enable-opensles OFF)
        set(enable-oboe OFF)
        set(enable-network OFF)
        set(enable-oss OFF)
        set(enable-dsound OFF)
        set(enable-wasapi OFF)
        set(enable-waveout OFF)
        set(enable-winmidi OFF)
        set(enable-sdl2 OFF)
        set(enable-pulseaudio OFF)
        set(enable-pipewire OFF)
        set(enable-readline OFF)
        set(enable-threads OFF)
        set(enable-openmp OFF)
        set(enable-alsa OFF)
        set(enable-systemd OFF)
        add_subdirectory(Resources/fluidsynth EXCLUDE_FROM_ALL)
        set_target_properties(libfluidsynth PROPERTIES POSITION_INDEPENDENT_CODE ON)
        pd_add_external(sfont~ "${SFONT_SRC}" TARGET sfont_tilde)
        target_link_libraries(sfont_tilde PRIVATE libfluidsynth)
    endif()
endif()

# ╭──────────────────────────────────────╮
# │               CIRCUIT                │
# ╰──────────────────────────────────────╯
set(CIRCUIT_ROOT "${LIB_DIR}/Code_source/Compiled/audio/circuit~")

file(GLOB AMD_SRC "${CIRCUIT_ROOT}/Libraries/AMD/*.c")
file(GLOB BTF_SRC "${CIRCUIT_ROOT}/Libraries/BTF/*.c")
file(GLOB COLAMD_SRC "${CIRCUIT_ROOT}/Libraries/COLAMD/*.c")
file(GLOB COLAMD_SRC "${CIRCUIT_ROOT}/Libraries/COLAMD/*.c")
file(GLOB KLU_SRC "${CIRCUIT_ROOT}/Libraries/KLU/*.c")
file(GLOB SAMPLERATE_SRC "${CIRCUIT_ROOT}/Libraries/libsamplerate/*.c")
set(SUITE_SRC "${CIRCUIT_ROOT}/Libraries/SuiteSparse_config/SuiteSparse_config.c")
set(SIMULATOR_SRC "${CIRCUIT_ROOT}/Source/Simulator.cpp")
set(CIRCUIT_SRC "${CIRCUIT_ROOT}/Source/circuit~.c")

pd_add_external(
    circuit~
    "${CIRCUIT_SRC};${AMD_SRC};${BTF_SRC};${COLAMD_SRC};${KLU_SRC};${SAMPLERATE_SRC};${SUITE_SRC};${SIMULATOR_SRC}"
    TARGET
    circuit_tilde)

target_include_directories(
    circuit_tilde
    PRIVATE ${CIRCUIT_ROOT}/Libraries
            ${CIRCUIT_ROOT}/Libraries/AMD
            ${CIRCUIT_ROOT}/Libraries/BTF
            ${CIRCUIT_ROOT}/Libraries/KLU
            ${CIRCUIT_ROOT}/Libraries/libsamplerate
            ${CIRCUIT_ROOT}/Libraries/SuiteSparse_config
            ${CIRCUIT_ROOT}/Source)

target_compile_features(circuit_tilde PRIVATE cxx_std_17)

# ╭──────────────────────────────────────╮
# │             EXTRA FILES              │
# ╰──────────────────────────────────────╯
file(GLOB Control_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Abstractions/control/*.pd")
file(GLOB Audio_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Abstractions/audio/*.pd")
file(GLOB Extra_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Abstractions/extra_abs/*.pd")
file(GLOB Tcl_Extra "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Compiled/extra_source/*.tcl")
file(GLOB Scope3D "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Compiled/audio/scope3d~.pd_lua")
file(GLOB Help_Files "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/Help-files/*.pd")
file(GLOB Extra_Files "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/extra_files/*")
file(GLOB README "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/README.pdf")
file(GLOB Lua_Files "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Compiled/control/lua/*.lua")
file(GLOB Lua_Files "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Compiled/control/lua/*.pd_lua")
set(SF_Fonts "${CMAKE_CURRENT_SOURCE_DIR}/Code_source/Compiled/audio/sfont~/sf")

pd_add_datafile(
    else
    "${Control_Abs};${Audio_Abs};${Extra_Abs};${Tcl_Extra};${Scope3D};${Help_Files};${Extra_Files};${README};${Lua_Files};${SF_Fonts}"
)

# get_property(all_static_targets GLOBAL PROPERTY "${PROJECT_NAME}_STATIC_LIBRARIES")
# message("STATIC LIBRARIES: ${all_static_targets}")

# ╭──────────────────────────────────────╮
# │          Dynamic Libraries           │
# ╰──────────────────────────────────────╯
if(WIN32)
    target_link_libraries(osc.format PRIVATE ws2_32)
    target_link_libraries(osc.parse PRIVATE ws2_32)
endif()

add_definitions(-DHAVE_STRUCT_TIMESPEC)
