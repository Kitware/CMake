
use std::process::Command;

#[cfg(target_os="macos")]
fn is_pie_executable(executable: &str) -> bool {
    let output = Command::new("otool")
        .args(["-hv", executable])
        .output()
        .expect("Failed to check executable");
    if !output.status.success() {
        panic!("otool exited with non-zero exit code");
    }

    output.stdout.windows(5).any(|w| {
        (w[0] == b' ' || w[0] == b'\t')
            && (w[1..=3] == b"PIE")
            && (w[4] == b' ' || w[4] == b'\n')
    })
}

#[cfg(all(target_family="unix", not(target_os="macos")))]
fn is_pie_executable(executable: &str) -> bool {
    let output = Command::new("readelf")
        .args(["-lW", executable])
        .env("LANG", "C")
        .env("LC_ALL", "C")
        .output()
        .expect("Failed to check executable");
    if !output.status.success() {
        panic!("readelf exited with non-zero exit code");
    }
    let is_pie = output.stdout.windows(20).any(|w| w == b"Elf file type is DYN");
    let is_not_pie = output.stdout.windows(21).any(|w| w == b"Elf file type is EXEC");
    assert!(is_pie ^ is_not_pie, "Cannot determine type of ELF file");
    is_pie
}

fn main() {
    let commands = [
        // The actual path to the commands will be filled in by CMake.
        r#"$<TARGET_FILE:default_RustPie>"#,
        r#"$<TARGET_FILE:disabled_RustPie>"#,
        r#"$<TARGET_FILE:enabled_RustPie>"#,
    ];
    for command in commands {
        println!("Start command: {command:?}");
        let result = Command::new(command)
            .spawn()
            .expect("Failed to launch command")
            .wait()
            .expect("Failure while waiting for process to finish")
            .success();
        if !result {
            panic!("Process exited with non-zero exit code.");
        }
    }

    assert!(
        !is_pie_executable(commands[1]),
        "ERROR: disabled_RustPie must not be a PIE executable, but it is not."
    );
    assert!(
        is_pie_executable(commands[2]),
        "ERROR: enabled_RustPie must be a PIE executable, but it is not."
    );
}
