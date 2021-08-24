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

CXXFLAGS += -std=c++11 -I. -g
CCFLAGS += -std=c11 -I. -g -Wno-discarded-qualifiers
LDFLAGS += -lm -ltensorflow -g

all: runner infer
$(BINDIR): ; mkdir -p $(BINDIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(BINDIR)/runner: api/internally_implemented.o runner.o tf.o stdlib.o th.o
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: runner
runner: $(BINDIR)/runner | $(BINDIR)

$(BINDIR)/infer: infer.o tf.o stdlib.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: infer
infer: $(BINDIR)/infer | $(BINDIR)

clean:
	-$(RM) *.o
	-$(RM) $(BINDIR)/runner
	-$(RM) $(BINDIR)/infer
