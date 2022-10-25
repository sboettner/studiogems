include dpf/Makefile.base.mk

all: dgl plugins

.PHONY: plugins
plugins: dgl
	$(MAKE) all -C plugins


dgl:
ifeq ($(HAVE_CAIRO_OR_OPENGL),true)
	$(MAKE) -C dpf/dgl FILE_BROWSER_DISABLED=true
endif


install:
	install -D -t /usr/local/lib/ladspa bin/OpalChorus-ladspa.so
	install -D -t /usr/local/lib/dssi bin/OpalChorus-dssi.so
	install -D -t /usr/local/lib/dssi/OpalChorus-dssi bin/OpalChorus-dssi/OpalChorus_ui

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C plugins
