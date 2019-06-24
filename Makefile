all:
	$(MAKE) -C mod-cv-clock/source
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

clean:
	$(MAKE) clean -C mod-cv-clock/source
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

install:
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 /usr/lib/lv2/
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 /usr/lib/lv2/
	cp -r mod-audio-to-cv/source/mod-audio-to-cv.lv2 /usr/lib/lv2/
	cp -r mod-cv-attenuverter/source/mod-cv-attenuverter.lv2 /usr/lib/lv2/
	cp -r mod-cv-clock/source/mod-cv-clock.lv2 /usr/lib/lv2/
	cp -r mod-cv-control/source/mod-cv-control.lv2 /usr/lib/lv2/
	cp -r mod-cv-switch1/source/mod-cv-switch1.lv2 /usr/lib/lv2/
	cp -r mod-cv-switch2/source/mod-cv-switch2.lv2 /usr/lib/lv2/
	cp -r mod-cv-switch3/source/mod-cv-switch3.lv2 /usr/lib/lv2/
	cp -r mod-cv-switch4/source/mod-cv-switch4.lv2 /usr/lib/lv2/
	cp -r mod-midi-to-cv-mono/source/mod-midi-to-cv-mono.lv2 /usr/lib/lv2/
	cp -r mod-midi-to-cv-poly/source/mod-midi-to-cv-poly.lv2 /usr/lib/lv2/
