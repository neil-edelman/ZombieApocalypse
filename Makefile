# Makefile 1.1 (GNU Make 3.81; MacOSX gcc 4.2.1; MacOSX MinGW 4.3.0)

PROJ  := ZombieApocalypse
VA    := 0
VB    := 1
FILES := Open Map Vector Bitmap Camera Text Obj Widget Input Buffer
SDIR  := src
BDIR  := bin
BACK  := backup
ICON  := icon.ico
EXTRA := Makefile.windows
INST  := $(PROJ)-$(VA)_$(VB)
OBJS  := $(patsubst %,$(BDIR)/%.o,$(FILES))
SRCS  := $(patsubst %,$(SDIR)/%.c,$(FILES))
H     := $(patsubst %,$(SDIR)/%.h,$(FILES))

CC   := gcc
CF   := -Wall -O3 -fasm -fomit-frame-pointer -ffast-math -funroll-loops -pedantic -ansi # or -std=c99 -mwindows
OF   := -framework OpenGL -framework GLUT

default: $(BDIR)/$(PROJ)
	cp $(SDIR)/$(ICON) $(BDIR)/$(ICON)
	sips --addIcon $(BDIR)/$(ICON)
	DeRez -only icns $(BDIR)/$(ICON) > $(BDIR)/$(ICON).rsrc
	Rez -append $(BDIR)/$(ICON).rsrc -o $(BDIR)/$(PROJ)
	SetFile -a C $(BDIR)/$(PROJ)
	rm $(BDIR)/$(ICON) $(BDIR)/$(ICON).rsrc

$(BDIR)/$(PROJ): $(OBJS)
	$(CC) $(OF) $(CF) $^ -o $@

$(BDIR)/%.o: $(SDIR)/%.c
	@mkdir -p $(BDIR)
	$(CC) $(CF) -c $? -o $@

.PHONY: setup clean backup

setup: default
	@mkdir -p $(BDIR)/$(INST)
	cp $(BDIR)/$(PROJ) readme.txt gpl.txt copying.txt $(BDIR)/$(INST)
	rm -f $(BDIR)/$(INST)-MacOSX.dmg
	hdiutil create $(BDIR)/$(INST)-MacOSX.dmg -volname "$(PROJ) $(VA).$(VB)" -srcfolder $(BDIR)/$(INST)
	rm -R $(BDIR)/$(INST)

clean:
	-rm -f $(OBJS)

backup:
	@mkdir -p $(BACK)
	zip $(BACK)/$(INST)-`date +%Y-%m-%dT%H%M%S`.zip readme.txt gpl.txt copying.txt Makefile $(SRCS) $(H) $(SDIR)/$(ICON) $(EXTRA)
