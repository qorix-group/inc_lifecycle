use crate::*;

type FFIHandler = *mut std::ffi::c_void;

const HM_OK: i32 = 0;

#[no_mangle]
pub extern "C" fn health_monitor_builder_create() -> FFIHandler {
    let builder = HealthMonitorBuilder::new();
    let handler = Box::into_raw(Box::new(builder));
    handler as FFIHandler
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_add_deadline_monitor(
    handler: FFIHandler,
    tag: *const IdentTag,
    monitor: FFIHandler,
) {
    assert!(!handler.is_null());
    assert!(!tag.is_null());
    assert!(!monitor.is_null());

    let tag: &IdentTag = unsafe { &*tag };
    let monitor: Box<deadline::DeadlineMonitorBuilder> =
        unsafe { Box::from_raw(monitor as *mut deadline::DeadlineMonitorBuilder) };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut health_monitor_builder: Box<HealthMonitorBuilder> =
        unsafe { Box::from_raw(handler as *mut HealthMonitorBuilder) };
    health_monitor_builder.add_deadline_monitor(*tag, *monitor);
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_build(handler: FFIHandler) -> FFIHandler {
    assert!(!handler.is_null());

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let health_monitor_builder: Box<HealthMonitorBuilder> =
        unsafe { Box::from_raw(handler as *mut HealthMonitorBuilder) };
    let health_monitor = health_monitor_builder.build();
    let health_monitor_handle = Box::into_raw(Box::new(health_monitor));
    health_monitor_handle as FFIHandler
}

#[no_mangle]
pub extern "C" fn health_monitor_get_deadline_monitor(handler: FFIHandler, tag: *const IdentTag) -> FFIHandler {
    assert!(!handler.is_null());
    assert!(!tag.is_null());

    let tag: &IdentTag = unsafe { &*tag };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut health_monitor: Box<HealthMonitor> = unsafe { Box::from_raw(handler as *mut HealthMonitor) };
    if let Some(deadline_monitor) = health_monitor.get_deadline_monitor(tag) {
        let deadline_monitor_handle = Box::into_raw(Box::new(deadline_monitor));
        deadline_monitor_handle as FFIHandler
    } else {
        std::ptr::null_mut()
    }
}

// struct DeadlineMonitorCpp {
//     monitor: DeadlineMonitor,
//     deadline_pool: Vec<MaybeUninit<Deadline>>,
//     next_deadline_slot: usize,
// }

// #[repr(C)]
// enum SimpleErrors {
//     Success,
//     NullPointer,
//     OutOfMemory,
// }

// #[no_mangle]
// pub extern "C" fn deadline_monitor_builder_create() -> FFIHandler {
//     let builder = DeadlineMonitorBuilder::new();
//     let handler = Box::into_raw(Box::new(builder));
//     handler as FFIHandler
// }

// #[no_mangle]
// pub extern "C" fn deadline_monitor_builder_destroy(handler: FFIHandler) -> SpecificErrorCode {
//     assert!(!handler.is_null());

//     // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
//     // and this must be assured on other side of FFI.
//     unsafe {
//         Box::from_raw(handler as *mut DeadlineMonitorBuilder);
//     }

//     HM_OK
// }

// #[no_mangle]
// pub extern "C" fn deadline_monitor_builder_add_deadline(handler: FFIHandler, tag: *const IdentTag, min: u32, max: u32) {
//     assert!(!handler.is_null());
//     assert!(!tag.is_null());

//     let tag: &IdentTag = unsafe { &*tag };

//     // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
//     // and this must be assured on other side of FFI.
//     let mut monitor: Box<DeadlineMonitorBuilder> = unsafe { Box::from_raw(handler as *mut DeadlineMonitorBuilder) };
//     monitor.add_deadline(tag, TimeRange::new(min, max));
// }

// #[no_mangle]
// pub extern "C" fn deadline_monitor_get_deadline(handler: FFIHandler, tag: *const IdentTag) -> FFIHandler {
//     assert!(!handler.is_null());
//     assert!(!tag.is_null());

//     let tag: &IdentTag = unsafe { &*tag };

//     // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
//     // and this must be assured on other side of FFI.
//     let monitor: Box<DeadlineMonitorCpp> = unsafe { Box::from_raw(handler as *mut DeadlineMonitorCpp) };
//     let deadline_handle = monitor.get_deadline(tag);
// }
