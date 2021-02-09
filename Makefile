CXX=gcc
CXXFLAGS=-g -Wall
LDFLAGS=
LDLIBS=-lstdc++
INCLUDE=-I include

SRC_DIR=src
BLD_DIR=build
DEP_DIR=dep
EXE_DIR=bin

EXEC=$(EXE_DIR)/mudserver
SOURCES=$(shell find $(SRC_DIR)/*.cpp)
OBJECTS=$(patsubst $(SRC_DIR)/%.cpp,$(BLD_DIR)/%.o,$(SOURCES))

$(EXEC): $(EXE_DIR) $(BLD_DIR) $(DEP_DIR) $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(EXEC) $(OBJECTS) $(LDLIBS)

$(EXE_DIR):
	mkdir $(EXE_DIR)

$(BLD_DIR):
	mkdir $(BLD_DIR)

$(DEP_DIR):
	mkdir $(DEP_DIR)

-include $(patsubst $(BLD_DIR)/%.o,$(DEP_DIR)/%.d,$(OBJECTS))

$(OBJECTS): $(BLD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MT $@ -MM $< > $(DEP_DIR)/$*.d

run:
	@$(EXEC)

clean:
	rm -f $(BLD_DIR)/*.o $(DEP_DIR)/*.d $(EXE_DIR)/*

purge: clean
	rm -d $(BLD_DIR) $(EXE_DIR) $(DEP_DIR)
