all:
	$(MAKE) -C cv-logic-equal/source
	$(MAKE) -C cv-logic-larger/source
	$(MAKE) -C cv-logic-or/source
	$(MAKE) -C cv-logic-smaller/source

install: all
	mkdir bundles
	cp -r cv-logic-equal/source/cv-logic-equal.lv2 bundles/
	cp -r cv-logic-larger/source/cv-logic-larger.lv2 bundles/
	cp -r cv-logic-or/source/cv-logic-or.lv2 bundles/
	cp -r cv-logic-smaller/source/cv-logic-smaller.lv2 bundles/

clean:
	$(MAKE) clean -C cv-logic-equal/source
	$(MAKE) clean -C cv-logic-larger/source
	$(MAKE) clean -C cv-logic-or/source
	$(MAKE) clean -C cv-logic-smaller/source
