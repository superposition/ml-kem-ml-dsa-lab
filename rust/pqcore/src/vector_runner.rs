use serde::Deserialize;
use std::collections::{BTreeMap, BTreeSet};
use std::error::Error;
use std::fmt;
use std::fs;
use std::io;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum VectorStatus {
    Passed,
    Pending,
    Skipped,
    Failed,
}

impl VectorStatus {
    fn as_str(self) -> &'static str {
        match self {
            VectorStatus::Passed => "PASSED",
            VectorStatus::Pending => "PENDING",
            VectorStatus::Skipped => "SKIPPED",
            VectorStatus::Failed => "FAILED",
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct VectorCaseLocation {
    pub vector_set_id: String,
    pub scheme: String,
    pub mode: String,
    pub parameter_set: String,
    pub group_id: String,
    pub test_case_id: String,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct VectorCaseReport {
    pub status: VectorStatus,
    pub location: VectorCaseLocation,
    pub reason: String,
}

impl VectorCaseReport {
    pub fn failure(
        vector_set_id: impl Into<String>,
        scheme: impl Into<String>,
        mode: impl Into<String>,
        parameter_set: impl Into<String>,
        group_id: impl Into<String>,
        test_case_id: impl Into<String>,
        reason: impl Into<String>,
    ) -> Self {
        Self {
            status: VectorStatus::Failed,
            location: VectorCaseLocation {
                vector_set_id: vector_set_id.into(),
                scheme: scheme.into(),
                mode: mode.into(),
                parameter_set: parameter_set.into(),
                group_id: group_id.into(),
                test_case_id: test_case_id.into(),
            },
            reason: reason.into(),
        }
    }
}

impl fmt::Display for VectorCaseReport {
    fn fmt(&self, output: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            output,
            "{} vector_set={} scheme={} mode={} parameter_set={} group_id={} test_case_id={} reason={}",
            self.status.as_str(),
            self.location.vector_set_id,
            self.location.scheme,
            self.location.mode,
            self.location.parameter_set,
            self.location.group_id,
            self.location.test_case_id,
            self.reason
        )
    }
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub struct VectorSummary {
    pub passed: usize,
    pub pending: usize,
    pub skipped: usize,
    pub failed: usize,
}

impl VectorSummary {
    fn record(&mut self, status: VectorStatus) {
        match status {
            VectorStatus::Passed => self.passed += 1,
            VectorStatus::Pending => self.pending += 1,
            VectorStatus::Skipped => self.skipped += 1,
            VectorStatus::Failed => self.failed += 1,
        }
    }
}

impl fmt::Display for VectorSummary {
    fn fmt(&self, output: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            output,
            "summary passed={} pending={} skipped={} failed={}",
            self.passed, self.pending, self.skipped, self.failed
        )
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct VectorSetSummary {
    pub vector_set_id: String,
    pub status: VectorSummary,
}

#[derive(Clone, Debug, Default, Eq, PartialEq)]
pub struct VectorRunReport {
    pub cases: Vec<VectorCaseReport>,
    pub vector_sets: Vec<VectorSetSummary>,
    pub status: VectorSummary,
}

impl VectorRunReport {
    fn extend_vector_set(&mut self, vector_set_id: String, cases: Vec<VectorCaseReport>) {
        let mut status = VectorSummary::default();
        for case in &cases {
            status.record(case.status);
            self.status.record(case.status);
        }
        self.vector_sets.push(VectorSetSummary {
            vector_set_id,
            status,
        });
        self.cases.extend(cases);
    }

    pub fn has_failures(&self) -> bool {
        self.status.failed > 0
    }
}

#[derive(Debug)]
pub enum VectorRunnerError {
    Io {
        path: PathBuf,
        source: io::Error,
    },
    Json {
        path: PathBuf,
        source: serde_json::Error,
    },
    Schema(String),
}

impl fmt::Display for VectorRunnerError {
    fn fmt(&self, output: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            VectorRunnerError::Io { path, source } => {
                write!(output, "{}: {}", path.display(), source)
            }
            VectorRunnerError::Json { path, source } => {
                write!(output, "{}: malformed JSON: {}", path.display(), source)
            }
            VectorRunnerError::Schema(message) => output.write_str(message),
        }
    }
}

impl Error for VectorRunnerError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            VectorRunnerError::Io { source, .. } => Some(source),
            VectorRunnerError::Json { source, .. } => Some(source),
            VectorRunnerError::Schema(_) => None,
        }
    }
}

#[derive(Debug, Deserialize)]
struct VectorManifest {
    schema_version: u64,
    sources: Vec<VectorSource>,
    vector_sets: Vec<VectorSet>,
}

#[derive(Debug, Deserialize)]
struct VectorSource {
    id: String,
}

#[derive(Debug, Deserialize)]
struct VectorSet {
    id: String,
    kind: String,
    #[serde(default)]
    source_id: Option<String>,
    scheme: String,
    mode: String,
    #[allow(dead_code)]
    revision: String,
    parameter_sets: Vec<String>,
    status: String,
    #[serde(default)]
    pending_reason: Option<String>,
    #[serde(default)]
    files: Vec<VectorFile>,
    #[serde(default)]
    file: Option<String>,
}

#[derive(Debug, Deserialize)]
struct VectorFile {
    role: String,
    file: String,
}

#[derive(Debug, Deserialize)]
struct AcvpFile {
    algorithm: String,
    mode: String,
    revision: String,
    #[serde(rename = "testGroups")]
    test_groups: Vec<AcvpGroup>,
}

#[derive(Debug, Deserialize)]
struct AcvpGroup {
    #[serde(rename = "tgId")]
    tg_id: u64,
    #[serde(default, rename = "parameterSet")]
    parameter_set: Option<String>,
    tests: Vec<AcvpTest>,
}

#[derive(Debug, Deserialize)]
struct AcvpTest {
    #[serde(rename = "tcId")]
    tc_id: u64,
}

#[derive(Debug, Deserialize)]
struct PlaceholderVectors {
    cases: Vec<PlaceholderCase>,
}

#[derive(Debug, Deserialize)]
struct PlaceholderCase {
    id: String,
    scheme: String,
    operation: String,
    parameter_set: String,
    expected: String,
}

pub fn run_manifest(manifest_path: &Path) -> Result<VectorRunReport, VectorRunnerError> {
    let manifest: VectorManifest = load_json(manifest_path)?;
    if manifest.schema_version != 1 {
        return Err(VectorRunnerError::Schema(format!(
            "{}: schema_version: expected 1",
            manifest_path.display()
        )));
    }

    let source_ids = source_ids(&manifest)?;
    let repo_root = repo_root_for_manifest(manifest_path);
    let mut report = VectorRunReport::default();
    for vector_set in &manifest.vector_sets {
        validate_vector_set_basics(vector_set, &source_ids)?;
        let cases = match vector_set.kind.as_str() {
            "official-acvp" => run_official_vector_set(vector_set, &repo_root)?,
            "repository-placeholder" => run_placeholder_vector_set(vector_set, &repo_root)?,
            other => {
                return Err(VectorRunnerError::Schema(format!(
                    "vector_set {}: unsupported kind {}",
                    vector_set.id, other
                )));
            }
        };
        report.extend_vector_set(vector_set.id.clone(), cases);
    }
    Ok(report)
}

fn source_ids(manifest: &VectorManifest) -> Result<BTreeSet<String>, VectorRunnerError> {
    let mut ids = BTreeSet::new();
    for source in &manifest.sources {
        if source.id.is_empty() {
            return Err(VectorRunnerError::Schema(
                "sources: source id must not be empty".to_string(),
            ));
        }
        if !ids.insert(source.id.clone()) {
            return Err(VectorRunnerError::Schema(format!(
                "sources: duplicate source id {}",
                source.id
            )));
        }
    }
    if ids.is_empty() {
        return Err(VectorRunnerError::Schema(
            "sources: must contain at least one source".to_string(),
        ));
    }
    Ok(ids)
}

fn repo_root_for_manifest(manifest_path: &Path) -> PathBuf {
    let parent = manifest_path.parent().unwrap_or_else(|| Path::new("."));
    if parent.file_name().and_then(|name| name.to_str()) == Some("test-vectors") {
        parent
            .parent()
            .unwrap_or_else(|| Path::new("."))
            .to_path_buf()
    } else {
        PathBuf::from(".")
    }
}

fn load_json<T: for<'de> Deserialize<'de>>(path: &Path) -> Result<T, VectorRunnerError> {
    let text = fs::read_to_string(path).map_err(|source| VectorRunnerError::Io {
        path: path.to_path_buf(),
        source,
    })?;
    serde_json::from_str(&text).map_err(|source| VectorRunnerError::Json {
        path: path.to_path_buf(),
        source,
    })
}

fn validate_vector_set_basics(
    vector_set: &VectorSet,
    source_ids: &BTreeSet<String>,
) -> Result<(), VectorRunnerError> {
    if vector_set.id.is_empty() {
        return Err(VectorRunnerError::Schema(
            "vector_set id must not be empty".to_string(),
        ));
    }
    if vector_set.parameter_sets.is_empty() {
        return Err(VectorRunnerError::Schema(format!(
            "vector_set {}: parameter_sets must not be empty",
            vector_set.id
        )));
    }
    for parameter_set in &vector_set.parameter_sets {
        if !known_parameter_set(&vector_set.scheme, parameter_set) {
            return Err(VectorRunnerError::Schema(format!(
                "vector_set {}: unknown parameter set {}",
                vector_set.id, parameter_set
            )));
        }
    }
    if vector_set.kind == "official-acvp" {
        let source_id = vector_set.source_id.as_deref().ok_or_else(|| {
            VectorRunnerError::Schema(format!(
                "vector_set {}: source_id is missing",
                vector_set.id
            ))
        })?;
        if !source_ids.contains(source_id) {
            return Err(VectorRunnerError::Schema(format!(
                "vector_set {}: unknown source_id {}",
                vector_set.id, source_id
            )));
        }
    }
    Ok(())
}

pub fn known_parameter_set(scheme: &str, parameter_set: &str) -> bool {
    match scheme {
        "ML-KEM" => matches!(parameter_set, "ML-KEM-512" | "ML-KEM-768" | "ML-KEM-1024"),
        "ML-DSA" => matches!(parameter_set, "ML-DSA-44" | "ML-DSA-65" | "ML-DSA-87"),
        "mixed" => {
            known_parameter_set("ML-KEM", parameter_set)
                || known_parameter_set("ML-DSA", parameter_set)
        }
        _ => false,
    }
}

pub fn rust_algorithm_available(scheme: &str, mode: &str) -> bool {
    let _ = (scheme, mode);
    false
}

fn run_official_vector_set(
    vector_set: &VectorSet,
    repo_root: &Path,
) -> Result<Vec<VectorCaseReport>, VectorRunnerError> {
    match vector_set.status.as_str() {
        "pending" => Ok(pending_vector_set_reports(vector_set)),
        "vendored" => run_vendored_official_vector_set(vector_set, repo_root),
        other => Err(VectorRunnerError::Schema(format!(
            "vector_set {}: unsupported official status {}",
            vector_set.id, other
        ))),
    }
}

fn pending_vector_set_reports(vector_set: &VectorSet) -> Vec<VectorCaseReport> {
    let reason = vector_set
        .pending_reason
        .as_deref()
        .unwrap_or("official vector set is pending");
    vector_set
        .parameter_sets
        .iter()
        .map(|parameter_set| VectorCaseReport {
            status: VectorStatus::Pending,
            location: VectorCaseLocation {
                vector_set_id: vector_set.id.clone(),
                scheme: vector_set.scheme.clone(),
                mode: vector_set.mode.clone(),
                parameter_set: parameter_set.clone(),
                group_id: "pending".to_string(),
                test_case_id: "pending".to_string(),
            },
            reason: reason.to_string(),
        })
        .collect()
}

fn run_vendored_official_vector_set(
    vector_set: &VectorSet,
    repo_root: &Path,
) -> Result<Vec<VectorCaseReport>, VectorRunnerError> {
    let prompt_path = vector_file_path(vector_set, repo_root, "prompt")?;
    let expected_path = vector_file_path(vector_set, repo_root, "expectedResults")?;
    let prompt: AcvpFile = load_json(&prompt_path)?;
    let expected: AcvpFile = load_json(&expected_path)?;
    validate_acvp_header(vector_set, &prompt, &prompt_path)?;
    validate_acvp_header(vector_set, &expected, &expected_path)?;

    let expected_cases = expected_case_index(&expected);
    let mut reports = Vec::new();
    for group in &prompt.test_groups {
        let parameter_set = group.parameter_set.as_deref().ok_or_else(|| {
            VectorRunnerError::Schema(format!(
                "{}: testGroup {}: parameterSet is missing in prompt",
                prompt_path.display(),
                group.tg_id
            ))
        })?;
        if !known_parameter_set(&vector_set.scheme, parameter_set) {
            return Err(VectorRunnerError::Schema(format!(
                "{}: testGroup {}: unknown parameterSet {}",
                prompt_path.display(),
                group.tg_id,
                parameter_set
            )));
        }

        for test in &group.tests {
            let location = VectorCaseLocation {
                vector_set_id: vector_set.id.clone(),
                scheme: vector_set.scheme.clone(),
                mode: vector_set.mode.clone(),
                parameter_set: parameter_set.to_string(),
                group_id: group.tg_id.to_string(),
                test_case_id: test.tc_id.to_string(),
            };
            if !expected_cases.contains_key(&(group.tg_id, test.tc_id)) {
                reports.push(VectorCaseReport {
                    status: VectorStatus::Failed,
                    location,
                    reason: "expectedResults case is missing".to_string(),
                });
            } else if rust_algorithm_available(&vector_set.scheme, &vector_set.mode) {
                reports.push(VectorCaseReport {
                    status: VectorStatus::Skipped,
                    location,
                    reason: "Rust algorithm is marked available but comparison is not wired"
                        .to_string(),
                });
            } else {
                reports.push(VectorCaseReport {
                    status: VectorStatus::Pending,
                    location,
                    reason:
                        "Rust public algorithm unavailable; official expected output not compared"
                            .to_string(),
                });
            }
        }
    }
    Ok(reports)
}

fn vector_file_path(
    vector_set: &VectorSet,
    repo_root: &Path,
    role: &str,
) -> Result<PathBuf, VectorRunnerError> {
    let file = vector_set
        .files
        .iter()
        .find(|entry| entry.role == role)
        .ok_or_else(|| {
            VectorRunnerError::Schema(format!(
                "vector_set {}: {} file entry is missing",
                vector_set.id, role
            ))
        })?;
    Ok(repo_root.join(&file.file))
}

fn validate_acvp_header(
    vector_set: &VectorSet,
    file: &AcvpFile,
    path: &Path,
) -> Result<(), VectorRunnerError> {
    if file.algorithm != vector_set.scheme {
        return Err(VectorRunnerError::Schema(format!(
            "{}: algorithm: expected {}, got {}",
            path.display(),
            vector_set.scheme,
            file.algorithm
        )));
    }
    if file.mode != vector_set.mode {
        return Err(VectorRunnerError::Schema(format!(
            "{}: mode: expected {}, got {}",
            path.display(),
            vector_set.mode,
            file.mode
        )));
    }
    if file.revision != vector_set.revision {
        return Err(VectorRunnerError::Schema(format!(
            "{}: revision: expected {}, got {}",
            path.display(),
            vector_set.revision,
            file.revision
        )));
    }
    Ok(())
}

fn expected_case_index(file: &AcvpFile) -> BTreeMap<(u64, u64), ()> {
    let mut cases = BTreeMap::new();
    for group in &file.test_groups {
        for test in &group.tests {
            cases.insert((group.tg_id, test.tc_id), ());
        }
    }
    cases
}

fn run_placeholder_vector_set(
    vector_set: &VectorSet,
    repo_root: &Path,
) -> Result<Vec<VectorCaseReport>, VectorRunnerError> {
    if vector_set.status != "placeholder" {
        return Err(VectorRunnerError::Schema(format!(
            "vector_set {}: placeholder status expected",
            vector_set.id
        )));
    }
    let file = vector_set.file.as_deref().ok_or_else(|| {
        VectorRunnerError::Schema(format!("vector_set {}: file is missing", vector_set.id))
    })?;
    let placeholder: PlaceholderVectors = load_json(&repo_root.join(file))?;
    let mut reports = Vec::new();
    for case in placeholder.cases {
        let location = VectorCaseLocation {
            vector_set_id: vector_set.id.clone(),
            scheme: case.scheme.clone(),
            mode: case.operation.clone(),
            parameter_set: case.parameter_set.clone(),
            group_id: "placeholder".to_string(),
            test_case_id: case.id.clone(),
        };
        if case.expected != "not_implemented" {
            reports.push(VectorCaseReport {
                status: VectorStatus::Failed,
                location,
                reason: "placeholder expected value is unsupported".to_string(),
            });
        } else if !known_parameter_set(&case.scheme, &case.parameter_set) {
            reports.push(VectorCaseReport {
                status: VectorStatus::Failed,
                location,
                reason: "placeholder parameter set is unknown".to_string(),
            });
        } else {
            reports.push(VectorCaseReport {
                status: VectorStatus::Pending,
                location,
                reason: "Rust public algorithm unavailable; placeholder expected not_implemented"
                    .to_string(),
            });
        }
    }
    Ok(reports)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn repo_manifest_path() -> PathBuf {
        Path::new(env!("CARGO_MANIFEST_DIR"))
            .join("../..")
            .join("test-vectors/manifest.json")
    }

    fn write_temp_manifest(name: &str, contents: &str) -> PathBuf {
        let path = std::env::temp_dir().join(format!("pqcore-{name}-{}.json", std::process::id()));
        fs::write(&path, contents).expect("temp manifest should be writable");
        path
    }

    #[test]
    fn malformed_manifest_is_rejected() {
        let path = write_temp_manifest("malformed", "{not-json");
        let error = run_manifest(&path).expect_err("malformed manifest should fail");
        let _ = fs::remove_file(&path);
        assert!(error.to_string().contains("malformed JSON"));
    }

    #[test]
    fn duplicate_source_ids_are_rejected() {
        let path = write_temp_manifest(
            "duplicate-sources",
            r#"{
              "schema_version": 1,
              "sources": [{"id": "source"}, {"id": "source"}],
              "vector_sets": []
            }"#,
        );
        let error = run_manifest(&path).expect_err("duplicate source ids should fail");
        let _ = fs::remove_file(&path);
        assert!(error.to_string().contains("duplicate source id source"));
    }

    #[test]
    fn parameter_set_names_match_manifest_names() {
        assert!(known_parameter_set("ML-KEM", "ML-KEM-512"));
        assert!(known_parameter_set("ML-KEM", "ML-KEM-768"));
        assert!(known_parameter_set("ML-KEM", "ML-KEM-1024"));
        assert!(!known_parameter_set("ML-KEM", "Kyber512"));
        assert!(known_parameter_set("ML-DSA", "ML-DSA-44"));
        assert!(known_parameter_set("ML-DSA", "ML-DSA-65"));
        assert!(known_parameter_set("ML-DSA", "ML-DSA-87"));
        assert!(!known_parameter_set("ML-DSA", "Dilithium2"));
        assert!(known_parameter_set("mixed", "ML-KEM-512"));
        assert!(known_parameter_set("mixed", "ML-DSA-44"));
    }

    #[test]
    fn real_manifest_reports_unavailable_rust_algorithms_as_pending() {
        let report = run_manifest(&repo_manifest_path()).expect("manifest should parse");
        assert_eq!(report.vector_sets.len(), 6);
        assert_eq!(report.status.passed, 0);
        assert_eq!(report.status.pending, 859);
        assert_eq!(report.status.skipped, 0);
        assert_eq!(report.status.failed, 0);
        assert!(!report.has_failures());
        assert!(report.cases.iter().any(|case| {
            case.location.vector_set_id == "acvp-ml-kem-keygen-fips203"
                && case.location.parameter_set == "ML-KEM-512"
                && case.location.group_id == "1"
                && case.location.test_case_id == "1"
                && case.status == VectorStatus::Pending
        }));
        assert!(report.cases.iter().any(|case| {
            case.location.vector_set_id == "repo-placeholder-public-api-failclosed"
                && case.location.test_case_id == "placeholder-ml-kem-512-keygen"
                && case.status == VectorStatus::Pending
        }));
    }

    #[test]
    fn failure_output_includes_vector_location_metadata() {
        let failure = VectorCaseReport::failure(
            "acvp-ml-dsa-sigver-fips204",
            "ML-DSA",
            "sigVer",
            "ML-DSA-44",
            "7",
            "99",
            "expectedResults case is missing",
        );
        let text = failure.to_string();
        assert!(text.contains("FAILED"));
        assert!(text.contains("vector_set=acvp-ml-dsa-sigver-fips204"));
        assert!(text.contains("scheme=ML-DSA"));
        assert!(text.contains("mode=sigVer"));
        assert!(text.contains("parameter_set=ML-DSA-44"));
        assert!(text.contains("group_id=7"));
        assert!(text.contains("test_case_id=99"));
        assert!(text.contains("expectedResults case is missing"));
    }

    #[test]
    fn summary_tracks_all_report_statuses() {
        let mut summary = VectorSummary::default();
        summary.record(VectorStatus::Passed);
        summary.record(VectorStatus::Pending);
        summary.record(VectorStatus::Skipped);
        summary.record(VectorStatus::Failed);
        assert_eq!(summary.passed, 1);
        assert_eq!(summary.pending, 1);
        assert_eq!(summary.skipped, 1);
        assert_eq!(summary.failed, 1);
        assert_eq!(
            summary.to_string(),
            "summary passed=1 pending=1 skipped=1 failed=1"
        );
    }
}
