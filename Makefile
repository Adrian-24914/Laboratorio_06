CXX = g++
CXXFLAGS = -O2 -pthread -std=c++17
SRC_DIR = src
BIN_DIR = bin
DATA_DIR = data
SCRIPTS_DIR = scripts

P1 = $(BIN_DIR)/p1_counter
P2 = $(BIN_DIR)/p2_ring
P3 = $(BIN_DIR)/p3_rw
P4 = $(BIN_DIR)/p4_deadlock
P5 = $(BIN_DIR)/p5_pipeline

all: $(P1) $(P2) $(P3) $(P4) $(P5)

$(BIN_DIR)/%: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: run_p1 run_p2 run_p3 run_p4 run_p5

run_p1: $(P1)
	cd $(SCRIPTS_DIR) && bash run_p1.sh

run_p2: $(P2)
	cd $(SCRIPTS_DIR) && bash run_p2.sh

run_p3: $(P3)
	cd $(SCRIPTS_DIR) && bash run_p3.sh

run_p4: $(P4)
	cd $(SCRIPTS_DIR) && bash run_p4.sh

run_p5: $(P5)
	cd $(SCRIPTS_DIR) && bash run_p5.sh

clean:
	rm -rf $(BIN_DIR) $(DATA_DIR)/*.csv
