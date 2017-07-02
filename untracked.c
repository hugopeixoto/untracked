#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>

typedef enum {
  UNKNOWN = 0,
  UNTRACKED = 1,
  TRACKED = 2,
  MIXED = 3
} Status;

char* join_paths(const char* base, const char* name) {
  int len = strlen(base) + strlen(name) + 2;
  char* path = (char*)malloc(len);
  snprintf(path, len, "%s/%s", base, name);
  return path;
}

Status exec_status(const char* cmd, const char* path) {
  if (fork() == 0) {
    execlp(cmd, cmd, path, NULL);
    exit(127);
  } else {
    int code;
    wait(&code);

    if (code == 0) {
      return TRACKED;
    } else {
      return UNTRACKED;
    }
  }
}

char* append_line(char* base, const char* name) {
  int blen = strlen(base);
  int nlen = strlen(name);

  base = (char*)realloc(base, blen + nlen + 2);
  strncpy(base + blen, name, nlen);
  base[blen + nlen] = '\n';
  base[blen + nlen+1] = '\0';

  return base;
}

int is_file(const char *path) {
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path) {
  struct stat path_stat = {0};
  stat(path, &path_stat);
  return S_ISDIR(path_stat.st_mode);
}

int is_git(const char *path)
{
  char* gitdir = join_paths(path, ".git");
  int rv = is_dir(gitdir);
  free(gitdir);
  return rv;
}

int is_svn(const char *path) {
  char* svndir = join_paths(path, ".svn");
  int rv = is_dir(svndir);
  free(svndir);
  return rv;
}

Status getdirinfo(char *path) {
  Status s = UNKNOWN;
  struct dirent* dp = NULL;
  DIR* fd = NULL;

  if (!is_dir(path)) {
    return UNTRACKED;
  }

  if (is_git(path)) {
    return exec_status("untracked-git", path);
  }

  if (is_svn(path)) {
    return exec_status("untracked-svn", path);
  }

  if (!(fd = opendir(path))) {
    return UNTRACKED;
  }

  char* backlog = strdup("");
  while((dp = readdir(fd)) != NULL) {
    char* child;
    Status c;

    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
        continue;
    }

    child = join_paths(path, dp->d_name);
    c = getdirinfo(child);
    s |= c;

    if (c == UNTRACKED) {
      backlog = append_line(backlog, child);
    }

    free(child);
  }
  closedir(fd);

  if (s == UNKNOWN) {
    s = UNTRACKED;
  }

  if (s == MIXED) {
    fputs(backlog, stdout);
  }
  free(backlog);

  return s;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    char* cwd = getcwd(NULL, 0);
    if (getdirinfo(cwd) == UNTRACKED) {
      puts(cwd);
    }
    free(cwd);
  } else {
    for (int i = 1; i < argc; i++) {
      if (getdirinfo(argv[i]) == UNTRACKED) {
        puts(argv[i]);
      }
    }
  }

  return 0;
}
