Detect files that are currently not under version control

This tool helps me figure out which projects are not tracked by a git
repository. Running this on my `$HOME/work` directory outputs the following:

```
hugopeixoto@hylia$ untracked ~/work/personal
m /home/hugopeixoto/work/personal/challenges/aoc2021
m /home/hugopeixoto/work/personal/challenges/aoc2020
m /home/hugopeixoto/work/personal/dotfiles
m /home/hugopeixoto/work/personal/tico
u /home/hugopeixoto/work/personal/talks
m /home/hugopeixoto/work/personal/netdiff
m /home/hugopeixoto/work/personal/dockerfiles
m /home/hugopeixoto/work/personal/infrastructure
m /home/hugopeixoto/work/personal/notes
u /home/hugopeixoto/work/personal/weekly2items
u /home/hugopeixoto/work/personal/dedup
```

The first character represents the status of the file/directory, where:
- `m` means modified: the directory is a git repository but with uncommited changes
- `u` means untracked: the directory/file is not tracked in git repository
