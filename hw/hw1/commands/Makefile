CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

main.exe: $(OBJ_FILES)
   g++ -o $@ $^

obj/%.o: src/%.cpp
   g++ -c -o $@ $<
