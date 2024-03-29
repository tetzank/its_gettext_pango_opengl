# dependencies:
#  - libxml 2
#  - pango
#  - cairo
#  - freeglut
#  - itstool
#  - gettext

CC=gcc
CFLAGS=-g

POTFILE=po/hoplite.pot
POFILES=${wildcard po/*.po}
MOFILES=${POFILES:.po=/LC_MESSAGES/hoplite.mo}

all: main ${MOFILES}

main: main.c
	${CC} ${CFLAGS} $< -o $@ `pkg-config libxml-2.0 pangocairo --cflags --libs` -lGL -lglut

${POTFILE}: hele_infantry_spearman_b.xml entity.its.xml
	@# execute ITS rules in entity.its.xml to extract translatable strings
	itstool -o ${POTFILE} -i entity.its.xml hele_infantry_spearman_b.xml

%.po: ${POTFILE}
	@# merge string changes into translation files
	@# only changes po file when there are string changes
	msgmerge -qU --backup=none $@ $<
	@# always touch file so make is happy and isn't redoing this everytime
	touch $@

%/LC_MESSAGES/hoplite.mo: %.po
	@# initial create message directory for language
	mkdir -p `dirname $@`
	@# create binary message catalog
	msgfmt -o $@ $<


clean:
	rm main
