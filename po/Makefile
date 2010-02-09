all: ru.mo

GMSGFMT ?= msgfmt

%.mo: %.po
	@lang=`echo $* | sed -e 's,.*/,,'`; \
	rm -f $${lang}.mo && $(GMSGFMT) -o t-$${lang}.mo $${lang}.po && mv t-$${lang}.mo $${lang}.mo
