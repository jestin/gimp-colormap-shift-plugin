GIMPTOOL = gimptool-2.0
PROGRAM = colormap-shift
GCC = gcc
GIMPCFLAGS = $(shell gimptool-2.0 --cflags)
GIMPLIBS = $(shell gimptool-2.0 --libs)
IGNORE_WARNINGS = -Wno-deprecated-declarations
XML2CFLAGS = $(shell xml2-config --cflags)
XML2LIBS = $(shell xml2-config --libs)
DIALOG_UI_FILE = plug-in-file-colormap-shift.ui

$(PROGRAM): colormap-shift.c
	$(GCC) $(GIMPCFLAGS) $(XML2CFLAGS) $(IGNORE_WARNINGS) -o $(PROGRAM) colormap-shift.c $(GIMPLIBS) $(XML2LIBS)

install: $(PROGRAM)
	$(GIMPTOOL) --install-bin $(PROGRAM)

install-ui:
	cp $(DIALOG_UI_FILE) `$(GIMPTOOL) --gimpdatadir`/ui/plug-ins/$(DIALOG_UI_FILE)

uninstall: $(PROGRAM)
	$(GIMPTOOL) --uninstall-bin $(PROGRAM)
	# rm `$(GIMPTOOL) --gimpdatadir`/ui/plug-ins/$(DIALOG_UI_FILE)

all: $(PROGRAM)

run: install
	gimp

tags:
	ctags * --recurse

clean:
	rm -f *.o $(PROGRAM)

