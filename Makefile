
PROGRAM = Snow-OLSR


EXTENSION	= cpp
OBJECT		= Object/
SOURCE		= Source/


CXX			= g++
LIBRARIES	= -lboost_system -lboost_thread -lpthread

ifeq ($(mode), release)
	CXXFLAGS = -std=c++14 -Wall -pedantic-errors -O2 -s
else
	mode     = debug
	CXXFLAGS = -std=c++14 -Wall -pedantic-errors -O0 -g
endif

SOURCES		= $(shell find $(SOURCE) -name "*.$(EXTENSION)")
OBJECTS		= $(shell find $(OBJECT) -name "*.o")
GENERATED	= $(patsubst $(SOURCE)%.$(EXTENSION), $(OBJECT)%.o, $(SOURCES))

# Make Object Files
makepp_no_builtin = 1

$(OBJECT)%.o: $(SOURCE)%.$(EXTENSION)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build Program
.PHONY: build rebuild clean

build: _build $(GENERATED)
	$(CXX) $(OBJECTS) -o $(PROGRAM) $(LIBRARIES)
	@echo Build succeeded

rebuild: _rebuild clean $(GENERATED)
	$(CXX) $(OBJECTS) -o $(PROGRAM) $(LIBRARIES)
	@echo Build succeeded

clean:
	$(RM) $(PROGRAM) $(GENERATED) $(PROGRAM).tar.gz

# Run Program
.PHONY: debug run leaktest

debug: build
	gdb ./$(PROGRAM)

run: rebuild
	./$(PROGRAM)

leaktest: build
	#algrind ./$(PROGRAM)
	#valgrind --track-origins=yes -v --show-reachable=yes ./$(PROGRAM)
	#valgrind --track-origins=yes -v --show-reachable=yes ./$(PROGRAM)
	valgrind --leak-check=full --show-reachable=yes ./$(PROGRAM)
	#valgrind --leak-check=full -v --show-reachable=yes ./$(PROGRAM)

# Helpers
.PHONY: clear tar readme cloc

clear:
	clear

tar: clear clean
	@echo Creating tar file: $(PROGRAM).tar.gz...
	$(RM) $(PROGRAM).tar.gz
	tar czf $(PROGRAM).tar.gz $(OBJECT) $(SOURCE) Makefile Readme

readme: clear
	@cat Readme
	@echo

cloc: clear
	cloc --by-file $(SOURCE)

# Internal Use
.PHONY: _build _rebuild

_build: clear
	@echo Building $(PROGRAM) - $(mode) Mode
	@echo ---------------------------------
ifneq ($(mode), $(filter $(mode), debug release))
	@echo ERROR: Invalid build mode
	@exit 1
endif

_rebuild: clear
	@echo Rebuilding $(PROGRAM) - $(mode) Mode
	@echo -----------------------------------
ifneq ($(mode), $(filter $(mode), debug release))
	@echo ERROR: Invalid build mode
	@exit 1
endif
