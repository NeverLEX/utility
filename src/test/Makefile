include ../Makefile.inc

AR=ar
ARFLAGS=-crv
CXX=g++
CXXFLAGS += -I../common -I../packer
LDFLAGS +=
LDLIBS +=
EXTRA_LDLIBS =
ADDLIBS = ../packer/packer.a

ifeq ($(shell uname), Darwin)
    EXTRA_LDLIBS += -liconv -framework Accelerate
else
    ADDLIBS += $(THIRD_PARTY_ROOT)/lib/libopenblas.a
endif

LIBS =
OBJECT =
BINS = arraylist_test arraymap_test defer_test dev-tools_test huffman_test option-parser_test packer_test

all: $(BINS) $(LIBS)

$(BINS):$(OBJECT) $(LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $@.cc $(OBJECT) $(LDLIBS) $(LIBS) $(EXTRA_LDLIBS) $(ADDLIBS)

$(LIBS):$(OBJECT)
	$(AR) $(ARFLAGS) $@ $(OBJECT)

%.o:%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECT)
	rm -rf $(BINS)
	rm -rf $(LIBS)

