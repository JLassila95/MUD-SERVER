ifeq ($(target-os), windows)
CXX=x86_64-w64-mingw32-gcc
LDLIBS=-lstdc++ -lws2_32
OBJEXT=.win.o
DEPEXT=.win.d
else
CXX=gcc
LDLIBS=-lstdc++
OBJEXT=.o
DEPEXT=.d
endif

CXXFLAGS=-g -Wall
LDFLAGS=
INCLUDE=-I include

SRC_DIR=src
BLD_DIR=build
DEP_DIR=dep
EXE_DIR=bin

ifeq ($(target-os), windows)
EXEC=$(EXE_DIR)/mudserver.exe
else
EXEC=$(EXE_DIR)/mudserver
endif

SOURCES=$(shell find $(SRC_DIR)/*.cpp)
OBJECTS=$(patsubst $(SRC_DIR)/%.cpp,$(BLD_DIR)/%$(OBJEXT),$(SOURCES))

$(EXEC): $(EXE_DIR) $(BLD_DIR) $(DEP_DIR) $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(EXEC) $(OBJECTS) $(LDLIBS)

$(EXE_DIR):
	mkdir $(EXE_DIR)

$(BLD_DIR):
	mkdir $(BLD_DIR)

$(DEP_DIR):
	mkdir $(DEP_DIR)

-include $(patsubst $(BLD_DIR)/%$(OBJEXT),$(DEP_DIR)/%$(DEPEXT),$(OBJECTS))

$(OBJECTS): $(BLD_DIR)/%$(OBJEXT): $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MT $@ -MM $< > $(DEP_DIR)/$*$(DEPEXT)

run:
	@$(EXEC)

win-run:
	@wine $(EXEC).exe

clean:
	rm -f $(BLD_DIR)/* $(DEP_DIR)/* $(EXE_DIR)/*

purge: clean
	rm -d $(BLD_DIR) $(EXE_DIR) $(DEP_DIR)
