include Makefile.inc

DIRS= ../src ../tests

.PHONY: clean

all: $(TESTNAME)

lib: $(LIBNAME)

test: $(TESTNAME)
	bash -c ./$(TESTNAME)

valgrind: $(TESTNAME)
	valgrind -v --log-fd=1 --suppressions=regex.supp ./$(TESTNAME) | less

perf: $(TESTNAME)
	bash -c "./$(TESTNAME) p"

$(LIBNAME) : force_look
	cd ../src; $(MAKE)

$(TESTNAME) : force_look $(LIBNAME)
	cd ../tests; $(MAKE)


clean:
	$(ECHO) cleaning all
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

force_look :
	true
