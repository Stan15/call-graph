# Makefile.llvm - Compile the project to LLVM IR with clang -S.
# NOTE: Use -g flag to generate debug information (file location/line/column)

CC = clang
CXX = clang++
CFLAGS = -Wall -Iinclude -Isrc/math -Isrc/utils -Ilib
LLVM_FLAGS = -S -emit-llvm -g $(CFLAGS)

SRCS = src/main.c src/math/arithmetic.c src/utils/logger.c lib/extra.c 
LL_FILES = $(patsubst %.c,build/llvm/%.ll,$(SRCS))
LINKED_LL = build/llvm/linked.ll

# File path for our custom plugin pass (JSON call graph output using the new PM)
PASS_SRC = CallGraphJson.cpp
PASS_SO = build/CallGraphJson.so
CALL_GRAPH = build/callgraph.json

all: call-graph

# Build all C files into LLVM IR (.ll) files.
build/llvm/%.ll: %.c
	@mkdir -p $(dir $@)
	$(CC) $(LLVM_FLAGS) $< -o $@

# Link all the .ll files into one module.
$(LINKED_LL): $(LL_FILES)
	llvm-link $(LL_FILES) -o $(LINKED_LL)

# Compile the custom pass shared library that we just made to generate JSON call graphs of stuff. We have to compile the plugin so that we can actually run it later on.
$(PASS_SO): $(PASS_SRC)
	@mkdir -p $(dir $@)
	$(CXX) -shared -fPIC `llvm-config --cxxflags` $(PASS_SRC) -o $(PASS_SO) `llvm-config --ldflags --libs core passes support`

# Generate the call graph JSON output using the plugin we created - cg-json
# We load our plugin with -load-pass-plugin and then invoke our pass (registered as "cg-json-new").
# Our pass uses -cg-json-output to specify the output file.
call-graph: $(LINKED_LL) $(PASS_SO)
	opt -load-pass-plugin=./$(PASS_SO) -passes='cg-json' -cg-json-output=$(CALL_GRAPH) $(LINKED_LL) > /dev/null
	@echo "Call graph JSON saved to $(CALL_GRAPH)"

clean:
	rm -rf build
