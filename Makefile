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
	$(MAKE) -C mod-cv-to-audio/source
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
	$(MAKE) clean -C mod-cv-to-audio/source
	$(MAKE) clean -C mod-midi-to-cv-mono/source
	$(MAKE) clean -C mod-midi-to-cv-poly/source
