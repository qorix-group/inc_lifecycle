type FFIHandler = *mut std::ffi::c_void;

#[repr(C)]
enum SimpleErrors {}

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_create() -> FFIHandler {
    let builder = DeadlineMonitorBuilder::new();
    let handler = Box::into_raw(Box::new(builder));
    handler as FFIHandler
}

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_destroy(handler: FFIHandler) {
    if !handler.is_null() {
        unsafe {
            Box::from_raw(handler as *mut DeadlineMonitorBuilder);
        }
    }
}

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_add_deadline(handler: FFIHandler) {
    assert!(!handler.is_null());
}
