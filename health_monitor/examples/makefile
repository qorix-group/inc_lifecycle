SOURCES := $(wildcard *.cpp)
TESTS := $(addprefix build/,$(basename $(SOURCES)))

.PHONY: run
run: cargo build $(TESTS)
	@ for test in $(TESTS); do echo "Running $$test"; ./$$test; done

.PHONY: cargo
cargo:
	cargo build

# Creates the build folder
build:
	mkdir -p build/

build/%: %.cpp
	g++ -std=c++23 $< -l:libhealth_monitor.a -L../../target/debug/ -I../src/ -o $@

build/heartbeat_monitor: heartbeat_monitor.cpp
	# g++ -std=c++23 heartbeat_monitor.cpp ../alive_monitor/src/cpp/alive_monitor.cpp -l:libhealth_monitor.a -l:libalive_monitor.a -L../../target/debug/ -I../src/ -I../alive_monitor/include/ -o build/heartbeat_monitor
