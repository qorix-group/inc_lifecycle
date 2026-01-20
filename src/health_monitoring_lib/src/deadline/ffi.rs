use core::time::Duration;
use std::os::raw::c_int;

use crate::deadline::deadline_monitor::Deadline;
use crate::deadline::*;
use crate::*;
type FFIHandle = *mut std::ffi::c_void;

const HM_OK: i32 = 0;
const HM_IN_USE: i32 = HM_OK + 1;
const HM_NOT_FOUND: i32 = HM_OK + 2;

struct DeadlineMonitorCpp {
    monitor: DeadlineMonitor,
    // Here we will keep allocation storage for Deadlines
}

impl DeadlineMonitorCpp {
    fn get_deadline(&self, tag: IdentTag) -> Result<FFIHandle, c_int> {
        match self.monitor.get_deadline(&tag) {
            Ok(deadline) => {
                // Now we allocate at runtime. As next step we will add a memory pool for deadlines into self and this way we will not need allocate anymore
                let deadline_handle = Box::into_raw(Box::new(deadline));
                Ok(deadline_handle as FFIHandle)
            },
            Err(DeadlineMonitorError::DeadlineInUse) => Err(HM_IN_USE),
            Err(DeadlineMonitorError::DeadlineNotFound) => Err(HM_NOT_FOUND),
        }
    }
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_create() -> FFIHandle {
    let builder = HealthMonitorBuilder::new();
    let handler = Box::into_raw(Box::new(builder));
    handler as FFIHandle
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_destroy(handler: FFIHandle) {
    assert!(!handler.is_null());
    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    unsafe {
        let _ = Box::from_raw(handler as *mut HealthMonitorBuilder);
    }
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_add_deadline_monitor(
    handler: FFIHandle,
    tag: *const IdentTag,
    monitor: FFIHandle,
) {
    assert!(!handler.is_null());
    assert!(!tag.is_null());
    assert!(!monitor.is_null());

    let tag: IdentTag = unsafe { *tag }; // Copy the IdentTag as this shall be trivally copyable

    let monitor = unsafe { Box::from_raw(monitor as *mut deadline::DeadlineMonitorBuilder) };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut health_monitor_builder: Box<HealthMonitorBuilder> =
        unsafe { Box::from_raw(handler as *mut HealthMonitorBuilder) };
    health_monitor_builder.add_deadline_monitor(tag, *monitor);

    // C++ still owes the lifetime of below handlers
    core::mem::forget(health_monitor_builder);
}

#[no_mangle]
pub extern "C" fn health_monitor_builder_build(handler: FFIHandle) -> FFIHandle {
    assert!(!handler.is_null());

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let health_monitor_builder: Box<HealthMonitorBuilder> =
        unsafe { Box::from_raw(handler as *mut HealthMonitorBuilder) };
    let health_monitor = health_monitor_builder.build();
    let health_monitor_handle = Box::into_raw(Box::new(health_monitor));
    health_monitor_handle as FFIHandle
}

#[no_mangle]
pub extern "C" fn health_monitor_get_deadline_monitor(handler: FFIHandle, tag: *const IdentTag) -> FFIHandle {
    assert!(!handler.is_null());
    assert!(!tag.is_null());

    let tag: IdentTag = unsafe { *tag };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut health_monitor: Box<HealthMonitor> = unsafe { Box::from_raw(handler as *mut HealthMonitor) };
    let res = if let Some(deadline_monitor) = health_monitor.get_deadline_monitor(&tag) {
        let deadline_monitor_handle = Box::into_raw(Box::new(DeadlineMonitorCpp {
            monitor: deadline_monitor,
        }));

        deadline_monitor_handle as FFIHandle
    } else {
        std::ptr::null_mut()
    };

    // C++ still owes the lifetime of below handlers
    core::mem::forget(health_monitor);

    res
}

#[no_mangle]
pub extern "C" fn health_monitor_destroy(handler: FFIHandle) {
    assert!(!handler.is_null());
    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `health_monitor_builder_create`
    // and this must be assured on other side of FFI.
    unsafe {
        let _ = Box::from_raw(handler as *mut HealthMonitor);
    }
}

// #[repr(C)]
// enum SimpleErrors {
//     Success,
//     NullPointer,
//     OutOfMemory,
// }

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_create() -> FFIHandle {
    let builder = DeadlineMonitorBuilder::new();
    let handler = Box::into_raw(Box::new(builder));
    handler as FFIHandle
}

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_destroy(handler: FFIHandle) {
    assert!(!handler.is_null());
    println!("deadline_monitor_builder_destroy called");
    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    println!("Before unsafe block in deadline_monitor_builder_destroy {:?}", handler);
    unsafe {
        let _ = Box::from_raw(handler as *mut DeadlineMonitorBuilder);
    }
}

#[no_mangle]
pub extern "C" fn deadline_monitor_builder_add_deadline(handler: FFIHandle, tag: *const IdentTag, min: u32, max: u32) {
    assert!(!handler.is_null());
    assert!(!tag.is_null());

    let tag: IdentTag = unsafe { *tag };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut monitor: Box<DeadlineMonitorBuilder> = unsafe { Box::from_raw(handler as *mut DeadlineMonitorBuilder) };
    monitor._add_deadline_internal(
        &tag,
        TimeRange::new(Duration::from_millis(min as u64), Duration::from_millis(max as u64)),
    );

    // C++ still owes the lifetime of below handlers
    core::mem::forget(monitor);
}

#[no_mangle]
pub extern "C" fn deadline_monitor_cpp_destroy(handler: FFIHandle) {
    assert!(!handler.is_null());
    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    unsafe {
        let _ = Box::from_raw(handler as *mut DeadlineMonitorCpp);
    }
}

#[no_mangle]
pub extern "C" fn deadline_monitor_cpp_get_deadline(
    handler: FFIHandle,
    tag: *const IdentTag,
    out: *mut FFIHandle,
) -> c_int {
    assert!(!handler.is_null());
    assert!(!tag.is_null());

    let tag: IdentTag = unsafe { *tag };

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let monitor: Box<DeadlineMonitorCpp> = unsafe { Box::from_raw(handler as *mut DeadlineMonitorCpp) };
    let deadline_handle = monitor.get_deadline(tag);

    // C++ still owes the lifetime of below handlers
    core::mem::forget(monitor);

    deadline_handle.map_or_else(
        |err_code| err_code,
        |handle| {
            unsafe {
                *out = handle;
            }
            HM_OK
        },
    )
}

#[no_mangle]
pub extern "C" fn deadline_start(deadline_handle: FFIHandle) -> c_int {
    assert!(!deadline_handle.is_null());

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut deadline: Box<Deadline> = unsafe { Box::from_raw(deadline_handle as *mut Deadline) };
    let res = deadline.start_internal();

    // // C++ still owes the lifetime of below handlers
    core::mem::forget(deadline);

    match res {
        Ok(()) => HM_OK,
        Err(_err) => HM_IN_USE, // For now we have only one error type
    }
}

#[no_mangle]
pub extern "C" fn deadline_stop(deadline_handle: FFIHandle) {
    assert!(!deadline_handle.is_null());

    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    let mut deadline: Box<Deadline> = unsafe { Box::from_raw(deadline_handle as *mut Deadline) };
    deadline.stop_internal();

    // // C++ still owes the lifetime of below handlers
    core::mem::forget(deadline);
}

#[no_mangle]
pub extern "C" fn deadline_destroy(deadline_handle: FFIHandle) {
    assert!(!deadline_handle.is_null());
    // Safety: We ensure that the pointer is valid. We assume that pointer was created by call to `deadline_monitor_builder_create`
    // and this must be assured on other side of FFI.
    unsafe {
        let _ = Box::from_raw(deadline_handle as *mut Deadline);
    }
}
