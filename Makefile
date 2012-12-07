CC=gcc
CFLAGS=-g

POTFILE=po/hoplite.pot
POFILES=${wildcard po/*.po}
MOFILES=${POFILES:.po=/LC_MESSAGES/hoplite.mo}

all: main ${MOFILES}

main: main.c
	${CC} ${CFLAGS} $< -o $@ `pkg-config libxml-2.0 pangocairo --cflags --libs` -lGL -lglut

${POTFILE}: hele_infantry_spearman_b.xml entity.its.xml
	itstool -o ${POTFILE} -i entity.its.xml hele_infantry_spearman_b.xml

%.po: ${POTFILE}
	msgmerge -qU --backup=none $@ $<
	touch $@

%/LC_MESSAGES/hoplite.mo: %.po
	mkdir -p `dirname $@`
	msgfmt -o $@ $<


clean:
	rm main
