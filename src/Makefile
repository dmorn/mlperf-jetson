BINDIR ?= $(abspath bin)

CXX             ?= g++
CC              ?= gcc
AS              ?= as
AR              ?= ar
LD              ?= ld
NM              ?= nm
OBJDUMP         ?= objdump
OBJCOPY         ?= objcopy
SIZE            ?= size

RM = rm -f
ARFLAGS := -csr

CPPSRCS += api/internally_implemented.cpp
CCSRCS += main.c

OBJS += $(patsubst %.cpp,%.o,$(CPPSRCS))
OBJS += $(patsubst %.c,%.o,$(CCSRCS))

CXXFLAGS += -std=c++11 -I. -g
CCFLAGS += -std=c11 -I. -g -Wno-discarded-qualifiers
LDFLAGS += -lm -ltensorflow -g

all: runner
$(BINDIR): ; mkdir -p $(BINDIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

runner: download_deps $(OBJS) $(BINDIR)
	$(CXX) -o $(BINDIR)/$@ $(OBJS) $(LDFLAGS)

runner_msan : download_deps $(OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=memory -o $@ $(OBJS) $(LDFLAGS)

runner_asan : download_deps $(OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=address -o $@ $(OBJS) $(LDFLAGS)

runner_ubsan : download_deps $(OBJS)
	$(CXX) $(CXXFLAGS) -fsanitize=undefined -o $@ $(OBJS) $(LDFLAGS)

download_deps:
	$(DOWNLOAD_DEPS)

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BINDIR)/runner

clean_downloads:
	$(RM) -r $(DOWNLOADS)
