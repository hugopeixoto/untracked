#![feature(exact_size_is_empty)]

#[derive(Debug)]
enum RepoStatus {
    Clean,
    Dirty,
}

#[derive(Debug)]
enum Status {
    Untracked,
    Tracked(RepoStatus),
}

// SVN: svn status "$1"

trait GitDirectory {
    fn is_git(&self) -> bool;
    fn git_status(&self) -> RepoStatus;
}

impl GitDirectory for std::path::PathBuf {
    fn is_git(&self) -> bool {
        self.join(".git").is_dir()
    }

    fn git_status(&self) -> RepoStatus {
        let git_status = std::process::Command::new("git")
            .args(["-C", &self.to_string_lossy(), "status", "--porcelain"])
            .output();

        match git_status {
            Ok(output) => {
                if output.stdout.is_empty() && output.stderr.is_empty() {
                    RepoStatus::Clean
                } else {
                    RepoStatus::Dirty
                }
            }
            Err(_) => RepoStatus::Dirty
        }
    }
}

fn track_status(path: &std::path::PathBuf) -> Status {
    if !path.is_dir() {
        return Status::Untracked;
    }

    if path.is_git() {
        return Status::Tracked(path.git_status());
    }

    let mut any_repositories = false;
    let mut reportable = vec![];

    let contents = path.read_dir();
    if contents.is_err() {
        eprintln!("! unable to read directory {}", path.to_string_lossy());

        return Status::Untracked;
    }

    for subdir in contents.unwrap() {
        let subdir = subdir.unwrap();

        match track_status(&subdir.path()) {
            Status::Tracked(RepoStatus::Clean) => {
                any_repositories = true;
            },
            Status::Tracked(RepoStatus::Dirty) => {
                any_repositories = true;
                reportable.push(('m', subdir.path()));
            },
            Status::Untracked => {
                reportable.push(('u', subdir.path()));
            },
        }
    }

    if any_repositories {
        for (status, subdir) in reportable {
            println!("{} {}", status, subdir.to_string_lossy());
        }

        Status::Tracked(RepoStatus::Clean)
    } else {
        Status::Untracked
    }
}

fn check_untracked(path: &std::path::PathBuf) -> bool {
    let path = path.canonicalize().unwrap();

    match track_status(&path) {
        Status::Untracked => {
            println!("u {}", path.to_string_lossy());
            false
        },
        Status::Tracked(RepoStatus::Dirty) => {
            println!("m {}", path.to_string_lossy());
            false
        },
        _ => true,
    }
}

fn main() {
    let directories = if std::env::args().len() == 1 {
        vec![std::env::current_dir().unwrap()]
    } else {
        std::env::args().skip(1).map(std::path::PathBuf::from).collect()
    };

    let mut ok = true;
    for directory in directories {
        ok &= check_untracked(&directory);
    }

    if !ok {
        std::process::exit(1);
    }
}
