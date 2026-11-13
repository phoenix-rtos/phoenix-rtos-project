//! Exercises the parts of `std` that go through the Phoenix porting layer.

use std::collections::HashMap;
use std::time::{Duration, Instant, SystemTime};
use std::{env, fs, thread};
use base64::prelude::*;

fn section(name: &str) {
    println!("\n== {name} ==");
}

fn main() {
    section("args + env");
    println!("args      = {:?}", env::args().collect::<Vec<_>>());
    println!("current_dir = {:?}", env::current_dir());

    section("threads");
    let h = thread::spawn(|| (1..=10u64).product::<u64>());
    println!("10!       = {:?}", h.join());
    let shared = std::sync::Arc::new(std::sync::Mutex::new(0u32));
    let hs: Vec<_> = (0..4)
        .map(|_| {
            let s = shared.clone();
            thread::spawn(move || {
                for _ in 0..1000 {
                    *s.lock().unwrap() += 1;
                }
            })
        })
        .collect();
    for h in hs {
        h.join().unwrap();
    }
    println!("mutex sum = {} (expect 4000)", shared.lock().unwrap());

    section("time");
    let t0 = Instant::now();
    thread::sleep(Duration::from_millis(50));
    let slept = t0.elapsed();
    println!("slept     = {slept:?} (expect >= 50ms)");
    println!("monotonic = {}", slept >= Duration::from_millis(50));
    println!("realtime  = {:?}", SystemTime::UNIX_EPOCH.elapsed().map(|d| d.as_secs()));

    section("filesystem");
    let path = "/root/rust-smoke.txt";
    match fs::write(path, b"round trip\n") {
        Ok(()) => {
            println!("write     = ok");
            println!("read back = {:?}", fs::read_to_string(path));
            println!("len       = {:?}", fs::metadata(path).map(|m| m.len()));
            println!("remove    = {:?}", fs::remove_file(path));
        }
        Err(e) => println!("write     = {e}"),
    }
    println!("/bin count = {:?}", fs::read_dir("/bin").map(|d| d.count()));
    match fs::read_dir("/etc") {
        Ok(d) => {
            for e in d.take(3) {
                let e = e.unwrap();
                println!("  {:?} is_file={:?}", e.file_name(), e.file_type().map(|t| t.is_file()));
            }
        }
        Err(e) => println!("read_dir /etc = {e}"),
    }
    println!("psh size  = {:?}", fs::metadata("/bin/psh").map(|m| m.len()));

    section("collections + alloc");
    let mut m = HashMap::new();
    for i in 0..1000u64 {
        m.insert(i, i * i);
    }
    println!("hashmap[999] = {:?}", m.get(&999));
    let mut s = String::new();
    for i in 0..100 {
        s.push_str(&i.to_string());
    }
    println!("string len   = {}", s.len());
    // Over-aligned allocation: libphoenix has no posix_memalign, so this goes
    // through the fallback in std's unix allocator.
    #[repr(align(128))]
    struct Aligned([u8; 128]);
    let boxed = Box::new(Aligned([7; 128]));
    let aligned = (&*boxed as *const Aligned).addr() % 128 == 0;
    println!("align128 ptr = {aligned}, first byte = {}", boxed.0[0]);

    section("errors + panics");
    println!("missing file = {:?}", fs::read_to_string("/definitely/not/here").unwrap_err().kind());
    let caught = std::panic::catch_unwind(|| panic!("boom")).is_err();
    println!("catch_unwind = {caught}");

    section("base64 crate");
    let dec = BASE64_STANDARD.decode(b"+uwgVQA=").unwrap();
    println!("dec = {dec:?}");
    assert_eq!(dec, b"\xFA\xEC\x20\x55\0");


    println!("\ndone");
}

#[cfg(test)]
mod tests {
    // Something for the test runner (test-rust-smoke) and `make test-rust-smoke-run` to run.
    #[test]
    fn allocation_and_formatting_work() {
        let v: Vec<String> = (0..4).map(|i| format!("item{i}")).collect();
        assert_eq!(v.join(","), "item0,item1,item2,item3");
    }

    #[test]
    fn threads_join() {
        let h = std::thread::spawn(|| 6 * 7);
        assert_eq!(h.join().unwrap(), 42);
    }
}
