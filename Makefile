CXX = g++
CXXFLAGS = -std=c++11 -g

SRCDIR = .
OBJDIR = obj
BINDIR = bin

SRC = $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/message/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))

TARGETS = $(patsubst $(SRCDIR)/test/%.cpp,$(BINDIR)/%,$(wildcard $(SRCDIR)/test/*.cpp))



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