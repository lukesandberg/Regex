include Makefile.inc

DIRS= ../src ../tests

.PHONY: clean

all: $(TESTNAME)

lib: $(LIBNAME)

test: $(TESTNAME)
	bash -c ./$(TESTNAME)	

perf: $(TESTNAME)
	bash -c "./$(TESTNAME) p"

$(LIBNAME) : force_look
	$(ECHO) checking lib
	cd ../src; $(MAKE) $(MFLAGS)

$(TESTNAME) : force_look $(LIBNAME)
	$(ECHO) checking test
	cd ../tests; $(MAKE) $(MFLAGS)


clean:
	$(ECHO) cleaning all
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

force_look :
	true
