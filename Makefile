# Makefile Generique
 
EXT = cpp
CXX = g++
EXEC = test
 
CXXFLAGS = -Wall -W -pedantic 
LDFLAGS = 
 
 
SRC = $(wildcard *.$(EXT))
OBJ = $(SRC:.$(EXT)=.o) 

all: $(EXEC)
 
$(EXEC): $(OBJ)
	@$(CXX) -o $@ $^ $(LDFLAGS)
 
$(OBJDIR)/%.o: %.$(EXT)
	@$(CXX) -o $@ -c $< $(CXXFLAGS)
 
clean:
	@rm -rf $(OBJDIR)/*.o
	@rm -f $(EXEC)