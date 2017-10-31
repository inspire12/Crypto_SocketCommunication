TARGETS = \
	server\
	client

AddOpt = -l pthread

all : $(TARGETS)

$(TARGETS) : $(patsubst %,%.c,$(TARGETS))
	$(CC) -o $@ $@.c $(AddOpt)
	@rm -rf $@.o

clean:
	rm -rf $(TARGETS)
	@echo "Clear ~"
