#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p, target[MAXPATH];
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_NOFOLLOW)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(lstat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }

      if (st.type == T_SYMLINK) {
        if (readlink(buf, target) < 0) {
          fprintf(2, "ls: cannot readlink %s target\n", path);
          exit(-1);
        }
        printf("%s %d %d %d -> %s\n", fmtname(buf), st.type, st.ino, st.size, target);
      } else {
        printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      }  
    }
    break;

  case T_SYMLINK:
    if (readlink(path, target) < 0) {
      fprintf(2, "ls: cannot readlink %s\n", path);
      exit(-1);
    }
    printf("%s %d %d %d -> %s\n", fmtname(path), st.type, st.ino, st.size, fmtname(target));
    break;
  }
  
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
