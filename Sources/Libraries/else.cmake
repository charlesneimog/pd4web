cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")

message(STATUS "PDCMAKE_DIR: ${PDCMAKE_DIR}")
include(${PDCMAKE_DIR}/pd.cmake)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/else)

include_directories(${LIB_DIR}/Source/Shared/aubio/src)
include_directories(${LIB_DIR}/Source/Shared)

project(else)

set(ENABLE_TILDE_TARGET_WARNING off)

# ╭──────────────────────────────────────╮
# │            GUI INTERFACE             │
# ╰──────────────────────────────────────╯
pd_add_external(knob "${LIB_DIR}/Source/Control/knob.c")
pd_add_external(button "${LIB_DIR}/Source/Control/button.c")
pd_add_external(pic "${LIB_DIR}/Source/Control/pic.c")
pd_add_external(keyboard "${LIB_DIR}/Source/Control/keyboard.c")
pd_add_external(pad "${LIB_DIR}/Source/Control/pad.c")
pd_add_external(openfile "${LIB_DIR}/Source/Control/openfile.c")
pd_add_external(colors "${LIB_DIR}/Source/Control/colors.c")

# ╭──────────────────────────────────────╮
# │               CONTROL                │
# ╰──────────────────────────────────────╯
pd_add_external(args "${LIB_DIR}/Source/Control/args.c")
pd_add_external(ctl.in "${LIB_DIR}/Source/Control/ctl.in.c")
pd_add_external(ctl.out "${LIB_DIR}/Source/Control/ctl.out.c")
pd_add_external(ceil "${LIB_DIR}/Source/Control/ceil.c")
pd_add_external(suspedal "${LIB_DIR}/Source/Control/suspedal.c")
pd_add_external(voices "${LIB_DIR}/Source/Control/voices.c")
pd_add_external(buffer "${LIB_DIR}/Source/Control/buffer.c")
pd_add_external(bicoeff "${LIB_DIR}/Source/Control/bicoeff.c")
pd_add_external(bicoeff2 "${LIB_DIR}/Source/Control/bicoeff2.c")
pd_add_external(click "${LIB_DIR}/Source/Control/click.c")
pd_add_external(canvas.active "${LIB_DIR}/Source/Control/canvas.active.c")
pd_add_external(canvas.bounds "${LIB_DIR}/Source/Control/canvas.bounds.c")
pd_add_external(canvas.edit "${LIB_DIR}/Source/Control/canvas.edit.c")
pd_add_external(canvas.gop "${LIB_DIR}/Source/Control/canvas.gop.c")
pd_add_external(canvas.mouse "${LIB_DIR}/Source/Control/canvas.mouse.c")
pd_add_external(canvas.name "${LIB_DIR}/Source/Control/canvas.name.c")
pd_add_external(canvas.pos "${LIB_DIR}/Source/Control/canvas.pos.c")
pd_add_external(canvas.setname "${LIB_DIR}/Source/Control/canvas.setname.c")
pd_add_external(canvas.vis "${LIB_DIR}/Source/Control/canvas.vis.c")
pd_add_external(canvas.zoom "${LIB_DIR}/Source/Control/canvas.zoom.c")
pd_add_external(properties "${LIB_DIR}/Source/Control/properties.c")
pd_add_external(break "${LIB_DIR}/Source/Control/break.c")
pd_add_external(cents2ratio "${LIB_DIR}/Source/Control/cents2ratio.c")
pd_add_external(changed "${LIB_DIR}/Source/Control/changed.c")
pd_add_external(gcd "${LIB_DIR}/Source/Control/gcd.c")
pd_add_external(dir "${LIB_DIR}/Source/Control/dir.c")
pd_add_external(datetime "${LIB_DIR}/Source/Control/datetime.c")
pd_add_external(default "${LIB_DIR}/Source/Control/default.c")
pd_add_external(dollsym "${LIB_DIR}/Source/Control/dollsym.c")
pd_add_external(floor "${LIB_DIR}/Source/Control/floor.c")
pd_add_external(fold "${LIB_DIR}/Source/Control/fold.c")
pd_add_external(hot "${LIB_DIR}/Source/Control/hot.c")
pd_add_external(hz2rad "${LIB_DIR}/Source/Control/hz2rad.c")
pd_add_external(initmess "${LIB_DIR}/Source/Control/initmess.c")
pd_add_external(keycode "${LIB_DIR}/Source/Control/keycode.c")
pd_add_external(lb "${LIB_DIR}/Source/Extra/Aliases/lb.c")
pd_add_external(limit "${LIB_DIR}/Source/Control/limit.c")
pd_add_external(loadbanger "${LIB_DIR}/Source/Control/loadbanger.c")
pd_add_external(merge "${LIB_DIR}/Source/Control/merge.c")
pd_add_external(fontsize "${LIB_DIR}/Source/Control/fontsize.c")
pd_add_external(format "${LIB_DIR}/Source/Control/format.c")
pd_add_external(message "${LIB_DIR}/Source/Control/message.c")
pd_add_external(messbox "${LIB_DIR}/Source/Control/messbox.c")
pd_add_external(metronome "${LIB_DIR}/Source/Control/metronome.c")
pd_add_external(mouse "${LIB_DIR}/Source/Control/mouse.c")
pd_add_external(mpe.in "${LIB_DIR}/Source/Control/mpe.in.c")
pd_add_external(noteinfo "${LIB_DIR}/Source/Control/noteinfo.c")
pd_add_external(note.in "${LIB_DIR}/Source/Control/note.in.c")
pd_add_external(note.out "${LIB_DIR}/Source/Control/note.out.c")
pd_add_external(order "${LIB_DIR}/Source/Control/order.c")
pd_add_external(osc.route "${LIB_DIR}/Source/Control/osc.route.c")
pd_add_external(osc.parse "${LIB_DIR}/Source/Control/osc.parse.c")
pd_add_external(osc.format "${LIB_DIR}/Source/Control/osc.format.c")
pd_add_external(factor "${LIB_DIR}/Source/Control/factor.c")
pd_add_external(float2bits "${LIB_DIR}/Source/Control/float2bits.c")
pd_add_external(panic "${LIB_DIR}/Source/Control/panic.c")
pd_add_external(pgm.in "${LIB_DIR}/Source/Control/pgm.in.c")
pd_add_external(pgm.out "${LIB_DIR}/Source/Control/pgm.out.c")
pd_add_external(pipe2 "${LIB_DIR}/Source/Control/pipe2.c")
pd_add_external(bend.in "${LIB_DIR}/Source/Control/bend.in.c")
pd_add_external(bend.out "${LIB_DIR}/Source/Control/bend.out.c")
pd_add_external(pack2 "${LIB_DIR}/Source/Control/pack2.c")
pd_add_external(quantizer "${LIB_DIR}/Source/Control/quantizer.c")
pd_add_external(rad2hz "${LIB_DIR}/Source/Control/rad2hz.c")
pd_add_external(ratio2cents "${LIB_DIR}/Source/Control/ratio2cents.c")
pd_add_external(rescale "${LIB_DIR}/Source/Control/rescale.c")
pd_add_external(rint "${LIB_DIR}/Source/Control/rint.c")
pd_add_external(router "${LIB_DIR}/Source/Control/router.c")
pd_add_external(route2 "${LIB_DIR}/Source/Control/route2.c")
pd_add_external(routeall "${LIB_DIR}/Source/Control/routeall.c")
pd_add_external(routetype "${LIB_DIR}/Source/Control/routetype.c")
pd_add_external(receiver "${LIB_DIR}/Source/Control/receiver.c")
pd_add_external(retrieve "${LIB_DIR}/Source/Control/retrieve.c")
pd_add_external(selector "${LIB_DIR}/Source/Control/selector.c")
pd_add_external(sender "${LIB_DIR}/Source/Control/sender.c")
pd_add_external(separate "${LIB_DIR}/Source/Control/separate.c")
pd_add_external(symbol2any "${LIB_DIR}/Source/Control/symbol2any.c")
pd_add_external(slice "${LIB_DIR}/Source/Control/slice.c")
pd_add_external(sort "${LIB_DIR}/Source/Control/sort.c")
pd_add_external(spread "${LIB_DIR}/Source/Control/spread.c")
pd_add_external(touch.in "${LIB_DIR}/Source/Control/touch.in.c")
pd_add_external(touch.out "${LIB_DIR}/Source/Control/touch.out.c")
pd_add_external(ptouch.in "${LIB_DIR}/Source/Control/ptouch.in.c")
pd_add_external(ptouch.out "${LIB_DIR}/Source/Control/ptouch.out.c")
pd_add_external(trunc "${LIB_DIR}/Source/Control/trunc.c")
pd_add_external(unmerge "${LIB_DIR}/Source/Control/unmerge.c")
pd_add_external(var "${LIB_DIR}/Source/Control/var.c")

# ╭──────────────────────────────────────╮
# │                SIGNAL                │
# ╰──────────────────────────────────────╯
pd_add_external(above~ "${LIB_DIR}/Source/Audio/above~.c")
pd_add_external(add~ "${LIB_DIR}/Source/Audio/add~.c")
pd_add_external(allpass.2nd~ "${LIB_DIR}/Source/Audio/allpass.2nd~.c")
pd_add_external(allpass.rev~ "${LIB_DIR}/Source/Audio/allpass.rev~.c")
pd_add_external(bitnormal~ "${LIB_DIR}/Source/Audio/bitnormal~.c")
pd_add_external(comb.rev~ "${LIB_DIR}/Source/Audio/comb.rev~.c")
pd_add_external(comb.filt~ "${LIB_DIR}/Source/Audio/comb.filt~.c")
pd_add_external(adsr~ "${LIB_DIR}/Source/Audio/adsr~.c")
pd_add_external(asr~ "${LIB_DIR}/Source/Audio/asr~.c")
pd_add_external(bandpass~ "${LIB_DIR}/Source/Audio/bandpass~.c")
pd_add_external(bandstop~ "${LIB_DIR}/Source/Audio/bandstop~.c")
pd_add_external(bl.imp~ "${LIB_DIR}/Source/Audio/bl.imp~.c")
pd_add_external(bl.imp2~ "${LIB_DIR}/Source/Audio/bl.imp2~.c")
pd_add_external(bl.saw~ "${LIB_DIR}/Source/Audio/bl.saw~.c")
pd_add_external(bl.saw2~ "${LIB_DIR}/Source/Audio/bl.saw2~.c")
pd_add_external(bl.square~ "${LIB_DIR}/Source/Audio/bl.square~.c")
pd_add_external(bl.tri~ "${LIB_DIR}/Source/Audio/bl.tri~.c")
pd_add_external(bl.vsaw~ "${LIB_DIR}/Source/Audio/bl.vsaw~.c")
pd_add_external(blocksize~ "${LIB_DIR}/Source/Audio/blocksize~.c")
pd_add_external(biquads~ "${LIB_DIR}/Source/Audio/biquads~.c")
pd_add_external(car2pol~ "${LIB_DIR}/Source/Audio/car2pol~.c")
pd_add_external(ceil~ "${LIB_DIR}/Source/Audio/ceil~.c")
pd_add_external(cents2ratio~ "${LIB_DIR}/Source/Audio/cents2ratio~.c")
pd_add_external(changed~ "${LIB_DIR}/Source/Audio/changed~.c")
pd_add_external(changed2~ "${LIB_DIR}/Source/Audio/changed2~.c")
pd_add_external(conv~ "${LIB_DIR}/Source/Audio/conv~.c")
pd_add_external(crackle~ "${LIB_DIR}/Source/Audio/crackle~.c")
pd_add_external(crossover~ "${LIB_DIR}/Source/Audio/crossover~.c")
pd_add_external(cusp~ "${LIB_DIR}/Source/Audio/cusp~.c")
pd_add_external(db2lin~ "${LIB_DIR}/Source/Audio/db2lin~.c")
pd_add_external(decay~ "${LIB_DIR}/Source/Audio/decay~.c")
pd_add_external(decay2~ "${LIB_DIR}/Source/Audio/decay2~.c")
pd_add_external(downsample~ "${LIB_DIR}/Source/Audio/downsample~.c")
pd_add_external(drive~ "${LIB_DIR}/Source/Audio/drive~.c")
pd_add_external(detect~ "${LIB_DIR}/Source/Audio/detect~.c")
pd_add_external(envgen~ "${LIB_DIR}/Source/Audio/envgen~.c")
pd_add_external(eq~ "${LIB_DIR}/Source/Audio/eq~.c")
pd_add_external(fbsine2~ "${LIB_DIR}/Source/Audio/fbsine2~.c")
pd_add_external(fdn.rev~ "${LIB_DIR}/Source/Audio/fdn.rev~.c")
pd_add_external(floor~ "${LIB_DIR}/Source/Audio/floor~.c")
pd_add_external(fold~ "${LIB_DIR}/Source/Audio/fold~.c")
pd_add_external(freq.shift~ "${LIB_DIR}/Source/Audio/freq.shift~.c")
pd_add_external(gbman~ "${LIB_DIR}/Source/Audio/gbman~.c")
pd_add_external(gate2imp~ "${LIB_DIR}/Source/Audio/gate2imp~.c")
pd_add_external(get~ "${LIB_DIR}/Source/Audio/get~.c")
pd_add_external(giga.rev~ "${LIB_DIR}/Source/Audio/giga.rev~.c")
pd_add_external(glide~ "${LIB_DIR}/Source/Audio/glide~.c")
pd_add_external(glide2~ "${LIB_DIR}/Source/Audio/glide2~.c")
pd_add_external(henon~ "${LIB_DIR}/Source/Audio/henon~.c")
pd_add_external(highpass~ "${LIB_DIR}/Source/Audio/highpass~.c")
pd_add_external(highshelf~ "${LIB_DIR}/Source/Audio/highshelf~.c")
pd_add_external(ikeda~ "${LIB_DIR}/Source/Audio/ikeda~.c")
pd_add_external(impseq~ "${LIB_DIR}/Source/Audio/impseq~.c")
pd_add_external(trunc~ "${LIB_DIR}/Source/Audio/trunc~.c")
pd_add_external(lastvalue~ "${LIB_DIR}/Source/Audio/lastvalue~.c")
pd_add_external(latoocarfian~ "${LIB_DIR}/Source/Audio/latoocarfian~.c")
pd_add_external(lorenz~ "${LIB_DIR}/Source/Audio/lorenz~.c")
pd_add_external(lincong~ "${LIB_DIR}/Source/Audio/lincong~.c")
pd_add_external(lin2db~ "${LIB_DIR}/Source/Audio/lin2db~.c")
pd_add_external(logistic~ "${LIB_DIR}/Source/Audio/logistic~.c")
pd_add_external(loop "${LIB_DIR}/Source/Control/loop.c")
pd_add_external(lop2~ "${LIB_DIR}/Source/Audio/lop2~.c")
pd_add_external(lowpass~ "${LIB_DIR}/Source/Audio/lowpass~.c")
pd_add_external(lowshelf~ "${LIB_DIR}/Source/Audio/lowshelf~.c")
pd_add_external(mov.rms~ "${LIB_DIR}/Source/Audio/mov.rms~.c")
pd_add_external(mtx~ "${LIB_DIR}/Source/Audio/mtx~.c")
pd_add_external(mtx.mc~ "${LIB_DIR}/Source/Audio/mtx.mc~.c")
pd_add_external(match~ "${LIB_DIR}/Source/Audio/match~.c")
pd_add_external(mov.avg~ "${LIB_DIR}/Source/Audio/mov.avg~.c")
pd_add_external(median~ "${LIB_DIR}/Source/Audio/median~.c")
pd_add_external(merge~ "${LIB_DIR}/Source/Audio/merge~.c")
pd_add_external(nchs~ "${LIB_DIR}/Source/Audio/nchs~.c")
pd_add_external(nyquist~ "${LIB_DIR}/Source/Audio/nyquist~.c")
pd_add_external(op~ "${LIB_DIR}/Source/Audio/op~.c")
pd_add_external(pol2car~ "${LIB_DIR}/Source/Audio/pol2car~.c")
pd_add_external(power~ "${LIB_DIR}/Source/Audio/power~.c")
pd_add_external(peak~ "${LIB_DIR}/Source/Audio/peak~.c")
pd_add_external(phaseseq~ "${LIB_DIR}/Source/Audio/phaseseq~.c")
pd_add_external(pulsecount~ "${LIB_DIR}/Source/Audio/pulsecount~.c")
pd_add_external(pick~ "${LIB_DIR}/Source/Audio/pick~.c")
pd_add_external(pimpmul~ "${LIB_DIR}/Source/Audio/pimpmul~.c")
pd_add_external(pulsediv~ "${LIB_DIR}/Source/Audio/pulsediv~.c")
pd_add_external(quad~ "${LIB_DIR}/Source/Audio/quad~.c")
pd_add_external(quantizer~ "${LIB_DIR}/Source/Audio/quantizer~.c")
pd_add_external(ramp~ "${LIB_DIR}/Source/Audio/ramp~.c")
pd_add_external(range~ "${LIB_DIR}/Source/Audio/range~.c")
pd_add_external(ratio2cents~ "${LIB_DIR}/Source/Audio/ratio2cents~.c")
pd_add_external(rescale~ "${LIB_DIR}/Source/Audio/rescale~.c")
pd_add_external(rint~ "${LIB_DIR}/Source/Audio/rint~.c")
pd_add_external(repeat~ "${LIB_DIR}/Source/Audio/repeat~.c")
pd_add_external(resonant~ "${LIB_DIR}/Source/Audio/resonant~.c")
pd_add_external(resonant2~ "${LIB_DIR}/Source/Audio/resonant2~.c")
pd_add_external(rms~ "${LIB_DIR}/Source/Audio/rms~.c")
pd_add_external(sh~ "${LIB_DIR}/Source/Audio/sh~.c")
pd_add_external(schmitt~ "${LIB_DIR}/Source/Audio/schmitt~.c")
pd_add_external(slice~ "${LIB_DIR}/Source/Audio/slice~.c")
pd_add_external(lag~ "${LIB_DIR}/Source/Audio/lag~.c")
pd_add_external(lag2~ "${LIB_DIR}/Source/Audio/lag2~.c")
pd_add_external(sig2float~ "${LIB_DIR}/Source/Audio/sig2float~.c")
pd_add_external(slew~ "${LIB_DIR}/Source/Audio/slew~.c")
pd_add_external(slew2~ "${LIB_DIR}/Source/Audio/slew2~.c")
pd_add_external(s2f~ "${LIB_DIR}/Source/Extra/Aliases/s2f~.c")
pd_add_external(sequencer~ "${LIB_DIR}/Source/Audio/sequencer~.c")
pd_add_external(select~ "${LIB_DIR}/Source/Audio/select~.c")
pd_add_external(sr~ "${LIB_DIR}/Source/Audio/sr~.c")
pd_add_external(status~ "${LIB_DIR}/Source/Audio/status~.c")
pd_add_external(standard~ "${LIB_DIR}/Source/Audio/standard~.c")
pd_add_external(sum~ "${LIB_DIR}/Source/Audio/sum~.c")
pd_add_external(sigs~ "${LIB_DIR}/Source/Audio/sigs~.c")
pd_add_external(susloop~ "${LIB_DIR}/Source/Audio/susloop~.c")
pd_add_external(svfilter~ "${LIB_DIR}/Source/Audio/svfilter~.c")
pd_add_external(trig.delay~ "${LIB_DIR}/Source/Audio/trig.delay~.c")
pd_add_external(trig.delay2~ "${LIB_DIR}/Source/Audio/trig.delay2~.c")
pd_add_external(timed.gate~ "${LIB_DIR}/Source/Audio/timed.gate~.c")
pd_add_external(toggleff~ "${LIB_DIR}/Source/Audio/toggleff~.c")
pd_add_external(trighold~ "${LIB_DIR}/Source/Audio/trighold~.c")
pd_add_external(unmerge~ "${LIB_DIR}/Source/Audio/unmerge~.c")
pd_add_external(vu~ "${LIB_DIR}/Source/Audio/vu~.c")
# pd_add_external(vcf2~ "${LIB_DIR}/Source/Audio/vcf2~.c")
pd_add_external(xmod~ "${LIB_DIR}/Source/Audio/xmod~.c")
pd_add_external(xmod2~ "${LIB_DIR}/Source/Audio/xmod2~.c")
pd_add_external(wrap2 "${LIB_DIR}/Source/Control/wrap2.c")
pd_add_external(wrap2~ "${LIB_DIR}/Source/Audio/wrap2~.c")
pd_add_external(zerocross~ "${LIB_DIR}/Source/Audio/zerocross~.c")

# ╭──────────────────────────────────────╮
# │                AUBIO                 │
# ╰──────────────────────────────────────╯
file(GLOB_RECURSE AUBIO_SRC1 "${LIB_DIR}/Source/Shared/aubio/src/*/*.c")
file(GLOB_RECURSE AUBIO_SRC2 "${LIB_DIR}/Source/Shared/aubio/src/*.c")
file(GLOB_RECURSE AUBIO_SRC3 "${LIB_DIR}/Source/Audio/beat~.c")
pd_add_external(beat~ "${AUBIO_SRC1};${AUBIO_SRC2};${AUBIO_SRC3}")

# ╭──────────────────────────────────────╮
# │                MAGIC                 │
# ╰──────────────────────────────────────╯
set(MAGIC_CODE "${LIB_DIR}/Source/Shared/magic.c")

pd_add_external(gaussian~ "${LIB_DIR}/Source/Audio/gaussian~.c;${MAGIC_CODE}")
pd_add_external(imp~ "${LIB_DIR}/Source/Extra/Aliases/imp~.c;${MAGIC_CODE}")
pd_add_external(impulse~ "${LIB_DIR}/Source/Audio/impulse~.c;${MAGIC_CODE}")
pd_add_external(imp2~ "${LIB_DIR}/Source/Extra/Aliases/imp2~.c;${MAGIC_CODE}")
pd_add_external(impulse2~ "${LIB_DIR}/Source/Audio/impulse2~.c;${MAGIC_CODE}")
pd_add_external(parabolic~ "${LIB_DIR}/Source/Audio/parabolic~.c;${MAGIC_CODE}")
pd_add_external(pulse~ "${LIB_DIR}/Source/Audio/pulse~.c;${MAGIC_CODE}")
pd_add_external(saw~ "${LIB_DIR}/Source/Audio/saw~.c;${MAGIC_CODE}")
pd_add_external(saw2~ "${LIB_DIR}/Source/Audio/saw2~.c;${MAGIC_CODE}")
pd_add_external(square~ "${LIB_DIR}/Source/Audio/square~.c;${MAGIC_CODE}")
pd_add_external(tri~ "${LIB_DIR}/Source/Audio/tri~.c;${MAGIC_CODE}")
pd_add_external(vsaw~ "${LIB_DIR}/Source/Audio/vsaw~.c;${MAGIC_CODE}")
pd_add_external(pimp~ "${LIB_DIR}/Source/Audio/pimp~.c;${MAGIC_CODE}")
pd_add_external(numbox~ "${LIB_DIR}/Source/Audio/numbox~.c;${MAGIC_CODE}")

# ╭──────────────────────────────────────╮
# │                BUFFER                │
# ╰──────────────────────────────────────╯
set(ELSE_BUFFER "${LIB_DIR}/Source/Shared/buffer.c")

pd_add_external(fader~ "${LIB_DIR}/Source/Audio/fader~.c;${ELSE_BUFFER}")
pd_add_external(autofade~ "${LIB_DIR}/Source/Audio/autofade~.c;${ELSE_BUFFER}")
pd_add_external(autofade.mc~ "${LIB_DIR}/Source/Audio/autofade.mc~.c;${ELSE_BUFFER}")
pd_add_external(autofade2~ "${LIB_DIR}/Source/Audio/autofade2~.c;${ELSE_BUFFER}")
pd_add_external(autofade2.mc~ "${LIB_DIR}/Source/Audio/autofade2.mc~.c;${ELSE_BUFFER}")
pd_add_external(balance~ "${LIB_DIR}/Source/Audio/balance~.c;${ELSE_BUFFER}")
pd_add_external(pan~ "${LIB_DIR}/Source/Audio/pan~.c;${ELSE_BUFFER}")
pd_add_external(pan.mc~ "${LIB_DIR}/Source/Audio/pan.mc~.c;${ELSE_BUFFER}")
pd_add_external(pan2~ "${LIB_DIR}/Source/Audio/pan2~.c;${ELSE_BUFFER}")
pd_add_external(pan4~ "${LIB_DIR}/Source/Audio/pan4~.c;${ELSE_BUFFER}")
pd_add_external(rotate~ "${LIB_DIR}/Source/Audio/rotate~.c;${ELSE_BUFFER}")
pd_add_external(rotate.mc~ "${LIB_DIR}/Source/Audio/rotate.mc~.c;${ELSE_BUFFER}")
pd_add_external(spread~ "${LIB_DIR}/Source/Audio/spread~.c;${ELSE_BUFFER}")
pd_add_external(spread.mc~ "${LIB_DIR}/Source/Audio/spread.mc~.c;${ELSE_BUFFER}")
pd_add_external(xfade~ "${LIB_DIR}/Source/Audio/xfade~.c;${ELSE_BUFFER}")
pd_add_external(xfade.mc~ "${LIB_DIR}/Source/Audio/xfade.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate~ "${LIB_DIR}/Source/Audio/xgate~.c;${ELSE_BUFFER}")
pd_add_external(xgate.mc~ "${LIB_DIR}/Source/Audio/xgate.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate2.mc~ "${LIB_DIR}/Source/Audio/xgate2.mc~.c;${ELSE_BUFFER}")
pd_add_external(xgate2~ "${LIB_DIR}/Source/Audio/xgate2~.c;${ELSE_BUFFER}")
pd_add_external(xselect~ "${LIB_DIR}/Source/Audio/xselect~.c;${ELSE_BUFFER}")
pd_add_external(xselect.mc~ "${LIB_DIR}/Source/Audio/xselect.mc~.c;${ELSE_BUFFER}")
pd_add_external(xselect2~ "${LIB_DIR}/Source/Audio/xselect2~.c;${ELSE_BUFFER}")
pd_add_external(xselect2.mc~ "${LIB_DIR}/Source/Audio/xselect2.mc~.c;${ELSE_BUFFER}")
pd_add_external(sin~ "${LIB_DIR}/Source/Audio/sin~.c;${ELSE_BUFFER}")
pd_add_external(fm~ "${LIB_DIR}/Source/Audio/fm~.c;${ELSE_BUFFER}")
pd_add_external(pm~ "${LIB_DIR}/Source/Audio/pm~.c;${ELSE_BUFFER}")
pd_add_external(pm2~ "${LIB_DIR}/Source/Audio/pm2~.c;${ELSE_BUFFER}")
pd_add_external(pm4~ "${LIB_DIR}/Source/Audio/pm4~.c;${ELSE_BUFFER}")
pd_add_external(pm6~ "${LIB_DIR}/Source/Audio/pm6~.c;${ELSE_BUFFER}")
pd_add_external(shaper~ "${LIB_DIR}/Source/Audio/shaper~.c;${ELSE_BUFFER}")
pd_add_external(tabreader "${LIB_DIR}/Source/Control/tabreader.c;${ELSE_BUFFER}")
pd_add_external(tabreader~ "${LIB_DIR}/Source/Audio/tabreader~.c;${ELSE_BUFFER}")
pd_add_external(function "${LIB_DIR}/Source/Control/function.c;${ELSE_BUFFER}")
pd_add_external(function~ "${LIB_DIR}/Source/Audio/function~.c;${ELSE_BUFFER}")
pd_add_external(tabwriter~ "${LIB_DIR}/Source/Audio/tabwriter~.c;${ELSE_BUFFER}")
pd_add_external(del~ "${LIB_DIR}/Source/Audio/del~.c;${ELSE_BUFFER}")
pd_add_external(fbdelay~ "${LIB_DIR}/Source/Audio/fbdelay~.c;${ELSE_BUFFER}")
pd_add_external(ffdelay~ "${LIB_DIR}/Source/Audio/ffdelay~.c;${ELSE_BUFFER}")
pd_add_external(filterdelay~ "${LIB_DIR}/Source/Audio/filterdelay~.c;${ELSE_BUFFER}")

# ╭──────────────────────────────────────╮
# │               BUFMAGIC               │
# ╰──────────────────────────────────────╯
list(APPEND bufmagic ${ELSE_BUFFER})
list(APPEND bufmagic ${MAGIC_CODE})

pd_add_external(cosine~ "${LIB_DIR}/Source/Audio/cosine~.c;${bufmagic}")
pd_add_external(fbsine~ "${LIB_DIR}/Source/Audio/fbsine~.c;${bufmagic}")
pd_add_external(sine~ "${LIB_DIR}/Source/Audio/sine~.c;${bufmagic}")
pd_add_external(wavetable~ "${LIB_DIR}/Source/Audio/wavetable~.c;${bufmagic}")
pd_add_external(wt~ "${LIB_DIR}/Source/Extra/Aliases/wt~.c;${bufmagic}")
pd_add_external(wt2d~ "${LIB_DIR}/Source/Audio/wt2d~.c;${bufmagic}")
pd_add_external(tabplayer~ "${LIB_DIR}/Source/Audio/tabplayer~.c;${bufmagic}")

# ╭──────────────────────────────────────╮
# │               RANDBUF                │
# ╰──────────────────────────────────────╯
list(APPEND randbuf "${LIB_DIR}/Source/Shared/random.c")
list(APPEND randbuf "${LIB_DIR}/Source/Shared/buffer.c")

pd_add_external(gendyn~ "${LIB_DIR}/Source/Audio/gendyn~.c;${randbuf}")

# ╭──────────────────────────────────────╮
# │              RANDMAGIC               │
# ╰──────────────────────────────────────╯
list(APPEND randmagic ${MAGIC_CODE})
list(APPEND randmagic "${LIB_DIR}/Source/Shared/random.c")
pd_add_external(brown~ "${LIB_DIR}/Source/Audio/brown~.c;${randmagic}")

# ╭──────────────────────────────────────╮
# │                 RAND                 │
# ╰──────────────────────────────────────╯
set(rand "${LIB_DIR}/Source/Shared/random.c")

pd_add_external(white~ "${LIB_DIR}/Source/Audio/white~.c;${rand}")
pd_add_external(pink~ "${LIB_DIR}/Source/Audio/pink~.c;${rand}")
pd_add_external(gray~ "${LIB_DIR}/Source/Audio/gray~.c;${rand}")
pd_add_external(pluck~ "${LIB_DIR}/Source/Audio/pluck~.c;${rand}")
pd_add_external(rand.u "${LIB_DIR}/Source/Control/rand.u.c;${rand}")
pd_add_external(rand.hist "${LIB_DIR}/Source/Control/rand.hist.c;${rand}")
pd_add_external(rand.i "${LIB_DIR}/Source/Control/rand.i.c;${rand}")
pd_add_external(rand.i~ "${LIB_DIR}/Source/Audio/rand.i~.c;${rand}")
pd_add_external(rand.f "${LIB_DIR}/Source/Control/rand.f.c;${rand}")
pd_add_external(rand.f~ "${LIB_DIR}/Source/Audio/rand.f~.c;${rand}")
pd_add_external(randpulse~ "${LIB_DIR}/Source/Audio/randpulse~.c;${rand}")
pd_add_external(randpulse2~ "${LIB_DIR}/Source/Audio/randpulse2~.c;${rand}")
pd_add_external(lfnoise~ "${LIB_DIR}/Source/Audio/lfnoise~.c;${rand}")
pd_add_external(rampnoise~ "${LIB_DIR}/Source/Audio/rampnoise~.c;${rand}")
pd_add_external(stepnoise~ "${LIB_DIR}/Source/Audio/stepnoise~.c;${rand}")
pd_add_external(dust~ "${LIB_DIR}/Source/Audio/dust~.c;${rand}")
pd_add_external(dust2~ "${LIB_DIR}/Source/Audio/dust2~.c;${rand}")
pd_add_external(chance "${LIB_DIR}/Source/Control/chance.c;${rand}")
pd_add_external(chance~ "${LIB_DIR}/Source/Audio/chance~.c;${rand}")
pd_add_external(tempo~ "${LIB_DIR}/Source/Audio/tempo~.c;${rand}")

# ╭──────────────────────────────────────╮
# │                 MIDI                 │
# ╰──────────────────────────────────────╯
list(APPEND midi_src "${LIB_DIR}/Source/Shared/mifi.c" "${LIB_DIR}/Source/Shared/elsefile.c")
pd_add_external(midi "${LIB_DIR}/Source/Control/midi.c;${midi_src}")

# ╭──────────────────────────────────────╮
# │                 FILE                 │
# ╰──────────────────────────────────────╯
pd_add_external(rec "${LIB_DIR}/Source/Control/rec.c;${LIB_DIR}/Source/Shared/elsefile.c")

# ╭──────────────────────────────────────╮
# │                SMAGIC                │
# ╰──────────────────────────────────────╯
set(magic "${LIB_DIR}/Source/Shared/magic.c")
pd_add_external(scope~ "${LIB_DIR}/Source/Audio/scope~.c;${magic}")

# ╭──────────────────────────────────────╮
# │                 UTF                  │
# ╰──────────────────────────────────────╯
# set(utf "${LIB_DIR}/Source/Shared/s_elseutf8.c")
pd_add_external(note "${LIB_DIR}/Source/Control/note.c;")

# ╭──────────────────────────────────────╮
# │                PLAITS                │
# ╰──────────────────────────────────────╯
file(GLOB_RECURSE PLAITS_SRC "${LIB_DIR}/Source/Audio/plaits~/*.cc")
list(APPEND PLAITS_SRC "${LIB_DIR}/Source/Audio/plaits~/plaits~.cpp")
include_directories("${LIB_DIR}/Source/Audio/plaits~/")
pd_add_external(plaits~ "${PLAITS_SRC}")

# ╭──────────────────────────────────────╮
# │                SFONT                 │
# ╰──────────────────────────────────────╯
if(NOT PD4WEB)
    set(BUILD_SFONT ON)
    if(BUILD_SFONT)

        list(APPEND SFONT_SRC "${LIB_DIR}/Source/Audio/sfont~/sfont~.c")
        list(APPEND SFONT_SRC "${LIB_DIR}/Source/Shared/elsefile.c")

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
set(CIRCUIT_ROOT "${LIB_DIR}/Source/Audio/circuit~")

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
file(GLOB Control_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/control/*.pd")
file(GLOB Audio_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/audio/*.pd")
file(GLOB Extra_Abs "${CMAKE_CURRENT_SOURCE_DIR}/Abstractions/extra_abs/*.pd")
file(GLOB Tcl_Extra "${CMAKE_CURRENT_SOURCE_DIR}/Source/Extra/*.tcl")
file(GLOB Scope3D "${CMAKE_CURRENT_SOURCE_DIR}/Source/Audio/scope3d~.pd_lua")
file(GLOB Help_Files "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/Help-files/*.pd")
file(GLOB Extra_Files "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/extra_files/*")
file(GLOB README "${CMAKE_CURRENT_SOURCE_DIR}/Documentation/README.pdf")
file(GLOB Lua_Files "${CMAKE_CURRENT_SOURCE_DIR}/Source/Control/lua/*.lua")
file(GLOB Lua_Files "${CMAKE_CURRENT_SOURCE_DIR}/Source/Control/lua/*.pd_lua")
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
