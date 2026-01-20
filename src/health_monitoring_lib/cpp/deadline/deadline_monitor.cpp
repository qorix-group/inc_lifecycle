#include "score/hm/deadline/deadline_monitor.h"


using FFIHandler = void*;


extern "C" {
    FFIHandler create_monitor_builder();

}