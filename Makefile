TARGETS   = thttpserver_downloader

CC        = gcc $(PIEFLAGS)
CXX       = g++ $(PIEFLAGS)
CFLAGS    = 
LFLAGS    = 
# Explicit version of ROOT is used
#ROOTCONF  = $(HOME)/cern/root_v6.22.06/bin/root-config
# Present version of ROOT is used
 ROOTCONF  = root-config
HAS_RPATH = $(shell $(ROOTCONF) --has-rpath)
ifeq ($(HAS_RPATH),yes)
CXXFLAGS  = $(shell $(ROOTCONF) --cflags)
ROOTLIBS  = $(shell $(ROOTCONF) --libs) -lXMLIO
else
CXXFLAGS  = $(shell $(ROOTCONF) --cflags)
ROOTLIBS  = $(shell $(ROOTCONF) --libs) -lXMLIO -Wl,-rpath,$(shell $(ROOTCONF) --libdir) -Wl,--disable-new-dtags
endif

all:	$(TARGETS)
thttpserver_downloader: thttpserver_downloader.o
	$(CXX)     $(LFLAGS) -o $@ $^ $(ROOTLIBS)
%.o: %.cxx
	$(CXX)     $(CXXFLAGS) -c $<
.PHONY : clean
clean:
	rm -rf *.o $(TARGETS)
