CXXFLAGS=-O3 -std=c++11 -Wall -fopenmp
RM=rm -f
EXEC=paralelo

all: $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(EXEC).cpp -c -o $(EXEC).o
	$(CXX) $(CXXFLAGS) $(EXEC).o -o $(EXEC)

clean:
	$(RM) $(EXEC).o $(EXEC)

.PHONY: all $(EXEC)