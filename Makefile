DEST_DIR=/usr/lib/lv2

all:
	$(MAKE) -C mod-cv-clock/source
	$(MAKE) -C mod-audio-to-cv/source
	$(MAKE) -C mod-cv-attenuverter/source
	$(MAKE) -C mod-cv-clock/source
	$(MAKE) -C mod-cv-control/source
	$(MAKE) -C mod-cv-switch1/source
	$(MAKE) -C mod-cv-switch2/source
	$(MAKE) -C mod-cv-switch3/source
	$(MAKE) -C mod-cv-switch4/source
	$(MAKE) -C mod-midi-to-cv-mono/source
	$(MAKE) -C mod-midi-to-cv-poly/source
	$(MAKE) -C mod-cv-meter/source
	$(MAKE) -C mod-cv-slew/source
	$(MAKE) -C mod-cv-random/source
	$(MAKE) -C mod-cv-range/source
	$(MAKE) -C mod-cv-abs/source
	$(MAKE) -C mod-cv-round/source
	$(MAKE) -C mod-cv-gate/source


install: all
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 					$(DEST_DIR)
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 					$(DEST_DIR)
	cp -r mod-audio-to-cv/source/mod-audio-to-cv.lv2 			$(DEST_DIR)
	cp -r mod-cv-attenuverter/source/mod-cv-attenuverter.lv2 	$(DEST_DIR)
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 					$(DEST_DIR)
	cp -r mod-cv-control/source/mod-cv-control.lv2 				$(DEST_DIR)
	cp -r mod-cv-switch1/source/mod-cv-switch1.lv2 				$(DEST_DIR)
	cp -r mod-cv-switch2/source/mod-cv-switch2.lv2 				$(DEST_DIR)
	cp -r mod-cv-switch3/source/mod-cv-switch3.lv2 				$(DEST_DIR)
	cp -r mod-cv-switch4/source/mod-cv-switch4.lv2 				$(DEST_DIR)
	cp -r mod-midi-to-cv-mono/source/mod-midi-to-cv-mono.lv2 	$(DEST_DIR)
	cp -r mod-midi-to-cv-poly/source/mod-midi-to-cv-poly.lv2 	$(DEST_DIR)
	cp -r mod-cv-meter/source/mod-cv-meter.lv2 					$(DEST_DIR)
	cp -r mod-cv-slew/source/mod-cv-slew.lv2 					$(DEST_DIR)
	cp -r mod-cv-random/source/mod-cv-random.lv2 				$(DEST_DIR)
	cp -r mod-cv-range/source/mod-cv-range.lv2 					$(DEST_DIR)
	cp -r mod-cv-abs/source/mod-cv-abs.lv2 						$(DEST_DIR)
	cp -r mod-cv-round/source/mod-cv-round.lv2 					$(DEST_DIR)
	cp -r mod-cv-gate/source/mod-cv-gate.lv2 					$(DEST_DIR)


clean:
	$(MAKE) clean -C mod-cv-clock/source
	$(MAKE) clean -C mod-audio-to-cv/source
	$(MAKE) clean -C mod-cv-attenuverter/source
	$(MAKE) clean -C mod-cv-clock/source
	$(MAKE) clean -C mod-cv-control/source
	$(MAKE) clean -C mod-cv-switch1/source
	$(MAKE) clean -C mod-cv-switch2/source
	$(MAKE) clean -C mod-cv-switch3/source
	$(MAKE) clean -C mod-cv-switch4/source
	$(MAKE) clean -C mod-midi-to-cv-mono/source
	$(MAKE) clean -C mod-midi-to-cv-poly/source
	$(MAKE) clean -C mod-cv-meter/source
	$(MAKE) clean -C mod-cv-slew/source
	$(MAKE) clean -C mod-cv-random/source
	$(MAKE) clean -C mod-cv-range/source
	$(MAKE) clean -C mod-cv-abs/source
	$(MAKE) clean -C mod-cv-round/source
	$(MAKE) clean -C mod-cv-gate/source
