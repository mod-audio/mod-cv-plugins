DEST_DIR=/usr/lib/lv2
all:
	$(MAKE) -C cv-logic-equal/source
	$(MAKE) -C cv-logic-larger/source
	$(MAKE) -C cv-logic-or/source
	$(MAKE) -C cv-logic-smaller/source
	$(MAKE) -C cv-logic-change/source

install: all
	cp -r cv-logic-equal/source/cv-logic-equal.lv2		$(DEST_DIR)
	cp -r cv-logic-larger/source/cv-logic-larger.lv2	$(DEST_DIR)
	cp -r cv-logic-or/source/cv-logic-or.lv2 			$(DEST_DIR)
	cp -r cv-logic-smaller/source/cv-logic-smaller.lv2 	$(DEST_DIR)
	cp -r cv-logic-change/source/cv-logic-change.lv2 	$(DEST_DIR)

clean:
	$(MAKE) clean -C cv-logic-equal/source
	$(MAKE) clean -C cv-logic-larger/source
	$(MAKE) clean -C cv-logic-or/source
	$(MAKE) clean -C cv-logic-smaller/source
	$(MAKE) clean -C cv-logic-change/source
