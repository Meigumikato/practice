

all: kaleidoscope kaleidoscope_jit kaleidoscope_ir

kaleidoscope: 
	clang++ kaleidoscope.cpp -o toy
kaleidoscope_ir: 
	clang++ -g -ggdb kaleidoscope_ir.cpp $(shell llvm-config --cxxflags --ldflags --system-libs --libs core) -O0 -o irtoy
kaleidoscope_jit: 
	clang++ -g -ggdb kaleidoscope_jit.cpp $(shell llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native) -O0 -o jittoy

clean:
	rm -f toy irtoy jittoy


