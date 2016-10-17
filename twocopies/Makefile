SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
DEPENDENCIES=$(SOURCES:.c=.d)
BINS=$(SOURCES:.c=)

CPPFLAGS=-std=c99 -Wall -Wextra -pedantic

all: $(BINS)

%d: %c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: clean

clean:
	$(RM) $(OBJECTS) $(DEPENDENCIES) $(BINS)

include $(DEPENDENCIES)
