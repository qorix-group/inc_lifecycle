.PHONY: run
run: cargo build deadline_monitor logic_monitor heartbeat_monitor health_monitor
	build/deadline_monitor
	build/logic_monitor
	build/heartbeat_monitor
	build/health_monitor

.PHONY: deadline_monitor
deadline_monitor:
	g++ -std=c++23 deadline_monitor.cpp -l:libhealth_monitor.a -L../../target/debug/ -I../src/ -o build/deadline_monitor

.PHONY: logic_monitor
logic_monitor:
	g++ -std=c++23 logic_monitor.cpp -l:libhealth_monitor.a -L../../target/debug/ -I../src/ -o build/logic_monitor


.PHONY: heartbeat_monitor
heartbeat_monitor:
	g++ -std=c++23 heartbeat_monitor.cpp -l:libhealth_monitor.a -L../../target/debug/ -I../src/ -o build/heartbeat_monitor

.PHONY: health_monitor
health_monitor:
	g++ -std=c++23 health_monitor.cpp -l:libhealth_monitor.a -L../../target/debug/ -I../src/ -o build/health_monitor

build:
	mkdir -p build/

.PHONY: cargo
cargo:
	cargo build
