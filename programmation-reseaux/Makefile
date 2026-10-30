SUBDIRS = $(wildcard td/*/) $(wildcard tp/*/)

all clean:
	@for d in $(SUBDIRS); do $(MAKE) -C $$d $@ || exit 1; done

.PHONY: all clean
