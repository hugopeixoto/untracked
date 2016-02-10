package main

import "os"
import "fmt"
import "sort"
import "flag"
import "path"

func main() {
  flag.Parse()

  if len(flag.Args()) == 0 {
	  dir, _ := os.Getwd()
    Print(GetGitInfo(dir))
  } else {
    for _, dir := range flag.Args() {
      Print(GetGitInfo(dir))
    }
  }
}

func IsDir(path string) bool {
  f, err := os.Stat(path)

  return err == nil && f.Mode().IsDir()
}

func UnderGit(path string) bool {
	f, err := os.Stat(path + "/.git")
	if err != nil {
		return false
	}

	return f.Mode().IsDir() && IsBareGit(path + "/.git")
}

func IsBareGit(path string) bool {
	_, err1 := os.Stat(path + "/objects")
	_, err2 := os.Stat(path + "/refs")
	_, err3 := os.Stat(path + "/HEAD")

	return err1 == nil && err2 == nil && err3 == nil
}

func ReadDirNames(dirname string) []string {
	f, err := os.Open(dirname)
	if err != nil {
		return []string{}
	}

	names, err := f.Readdirnames(-1)
	f.Close()

	if err != nil {
		return []string{}
	}

	sort.Strings(names)
	return names
}

type GitInfo struct {
  Path string
  AllGitted bool
  NoneGitted bool
  Children []GitInfo
}

func GetGitInfo(path string) GitInfo {
  info := GitInfo{ path, false, true, []GitInfo{} }

	info.AllGitted = UnderGit(path) || IsBareGit(path)

  if info.AllGitted {
    info.NoneGitted = false
    return info
  }

  names := ReadDirNames(path)
  if len(names) == 0 {
    return info
  }

  info.AllGitted = true
  info.NoneGitted = true
  for _, child := range names {
    cinfo := GetGitInfo(path + "/" + child)

    info.Children = append(info.Children, cinfo)

    info.AllGitted = info.AllGitted && cinfo.AllGitted
    info.NoneGitted = info.NoneGitted && cinfo.NoneGitted
  }

  return info
}

func Print(info GitInfo) {
  if info.AllGitted {
    return
  }

  if info.NoneGitted {
    fmt.Println(path.Clean(info.Path))
    return
  }

  for _, c := range info.Children {
    Print(c)
  }
}
