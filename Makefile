CXX = g++
CXXFLAGS = -g -std=c++11 -Wall

SRCDIR = .
OBJDIR = obj
BINDIR = bin

SRC = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/message/*.cpp) $(wildcard $(SRCDIR)/config/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))

TARGETS = $(patsubst $(SRCDIR)/test/%.cpp,$(BINDIR)/%,$(wildcard $(SRCDIR)/test/*.cpp))

.PRECIOUS: $(OBJDIR)/message/%.o $(OBJDIR)/test/%.o $(OBJDIR)/%.o $(OBJDIR)/config/%.o

all: $(TARGETS)

$(BINDIR)/%: $(OBJ) $(OBJDIR)/test/%.o
	@mkdir	-p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR)/message/%.o: $(SRCDIR)/message/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)