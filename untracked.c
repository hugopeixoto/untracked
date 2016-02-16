#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
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

char* execvp_stdout(char *argv[]) {
  int pfds[2];
  pid_t cpid;

  pipe(pfds);
  if ((cpid = fork())== 0) {
    close(pfds[0]);
    dup2(pfds[1], 1);
    close(pfds[1]);
    execvp(argv[0], argv);
    exit(127);
  } else {
    char buf[BUFSIZ];
    int s = 0;
    char* output = strdup("");
    close(pfds[1]);
    for (;;) {
      int n = read(pfds[0], buf, BUFSIZ);
      if (n <= 0) {
        return output;
      }

      output = realloc(output, s + n);
      memcpy(output + s, buf, n);
      s += n;
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
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISDIR(path_stat.st_mode);
}

int is_git(const char *path)
{
  char* headdir = join_paths(path, ".git/HEAD");
  char* objsdir = join_paths(path, ".git/objects");
  char* refsdir = join_paths(path, ".git/refs");

  int rv = is_file(headdir) && is_dir(objsdir) && is_dir(refsdir);

  free(headdir);
  free(objsdir);
  free(refsdir);
  return rv;
}

Status git_status(char *path) {
  char* status = execvp_stdout(
      (char*[]){"git", "-C", path, "status", "--porcelain", NULL});
  int clean_gwd = !strcmp(status, "");
  free(status);

  if (!clean_gwd) {
    return MIXED;
  }

  return TRACKED;
}

Status getdirinfo(char *path) {
  Status s = UNKNOWN;
  struct dirent* dp = NULL;
  DIR* fd = NULL;

  if (!is_dir(path)) {
    return UNTRACKED;
  }

  if (is_git(path)) {
    return git_status(path);
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
    if (getdirinfo(cwd) != TRACKED) {
      puts(cwd);
    }
    free(cwd);
  } else {
    for (int i = 1; i < argc; i++) {
      if (getdirinfo(argv[i]) != TRACKED) {
        puts(argv[i]);
      }
    }
  }

  return 0;
}
