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
CCSRCS += main.c tf.c stdlib.c

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

$(BINDIR)/runner: $(OBJS) | $(BINDIR)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)
runner: $(BINDIR)/runner

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BINDIR)/runner
