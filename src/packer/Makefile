include ../Makefile.inc

AR=ar
ARFLAGS=-crv
CXX=g++
CXXFLAGS += -I../common
LDFLAGS +=
LDLIBS +=
EXTRA_LDLIBS =
ADDLIBS =

LIBS = packer.a
OBJECT = huffman.o packer.o
BINS = resource-packer

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


