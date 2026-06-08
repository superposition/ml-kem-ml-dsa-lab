use pqcore::vector_runner::run_manifest;
use std::path::PathBuf;

fn main() {
    let mut manifest = PathBuf::from("test-vectors/manifest.json");
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        match arg.as_str() {
            "--manifest" => {
                let Some(value) = args.next() else {
                    eprintln!("--manifest requires a path");
                    std::process::exit(2);
                };
                manifest = PathBuf::from(value);
            }
            "--help" | "-h" => {
                println!("Usage: rust-vector-runner [--manifest test-vectors/manifest.json]");
                return;
            }
            other => {
                eprintln!("unknown argument: {other}");
                std::process::exit(2);
            }
        }
    }

    let report = match run_manifest(&manifest) {
        Ok(report) => report,
        Err(error) => {
            eprintln!("Rust vector runner failed: {error}");
            std::process::exit(1);
        }
    };

    for case in &report.cases {
        println!("{case}");
    }
    for vector_set in &report.vector_sets {
        println!(
            "vector_set={} {}",
            vector_set.vector_set_id, vector_set.status
        );
    }
    println!("{}", report.status);

    if report.has_failures() {
        std::process::exit(1);
    }
}
