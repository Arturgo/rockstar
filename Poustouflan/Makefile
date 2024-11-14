CXX = g++
CXXFLAGS = -std=c++17 -I./ -g -fsanitize=address
TARGET = solver
SRC = main.cpp solver.cpp cost.cpp
INSTANCES_DIR = ../instances
RESULT_DIR = results

# Default rule to compile the solver
all: $(TARGET) run-all

# Compile the solver if necessary
$(TARGET): $(SRC) instance.h solution.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Rule to process a specific instance file
# Usage: make example.out.json (to process ../instances/example.json)
results/%.out.json: $(TARGET)
	@input_file="$(INSTANCES_DIR)/$(basename $@).json"; \
	if [ -f "$$input_file" ]; then \
		echo "Processing instance: $(basename $@)"; \
		cat $$input_file | ./$(TARGET) > $@; \
		echo "Output saved to $@"; \
	else \
		echo "Instance file $$input_file does not exist"; \
	fi

# Process all instances in ../instances directory
run-all: $(TARGET)
	@for file in $(INSTANCES_DIR)/*.json; do \
		instance_name=$$(basename $$file .json); \
		output_file="./results/$$instance_name.out.json"; \
		echo "Processing instance: $$instance_name"; \
		cat $$file | ./$(TARGET) > $$output_file; \
		echo "Output saved to $$output_file"; \
	done

# Clean the build and output files
clean:
	rm -f $(TARGET)
	rm -f ./*.out.json

.PHONY: all run-all clean
