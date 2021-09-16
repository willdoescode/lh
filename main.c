#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "style.h"

struct owner_info {
  char letter;
  char* color;
  int is_dir;
};

int
is_directory(const char* path) {
  DIR* directory = opendir(path);
  if (directory != NULL) {
    closedir(directory);
    return 1;
  }

  if (errno == ENOTDIR) {
    closedir(directory);
    return 0;
  }

  closedir(directory);
  return -1;
}

char*
file_perm_str(struct stat f_stat) {
  char* x = malloc(8 * sizeof(char));
  x[0] = (f_stat.st_mode & S_IRUSR) ? 'r' : '-';
  x[1] = (f_stat.st_mode & S_IWUSR) ? 'w' : '-';
  x[2] = (f_stat.st_mode & S_IXUSR) ? 'x' : '-';
  x[3] = (f_stat.st_mode & S_IRGRP) ? 'r' : '-';
  x[4] = (f_stat.st_mode & S_IWGRP) ? 'w' : '-';
  x[5] = (f_stat.st_mode & S_IXGRP) ? 'x' : '-';
  x[6] = (f_stat.st_mode & S_IROTH) ? 'r' : '-';
  x[7] = (f_stat.st_mode & S_IWOTH) ? 'w' : '-';
  x[8] = (f_stat.st_mode & S_IXOTH) ? 'x' : '-';
  return x;
}

void
f_type(mode_t mode, struct owner_info* owner) {
  switch (mode & S_IFMT) {
  case S_IFBLK:
    owner->letter = 'b';
    owner->color = light_green;
    owner->is_dir = 0;
    break;
  case S_IFCHR:
    owner->letter = 'c';
    owner->color = light_yellow;
    owner->is_dir = 0;
    break;
  case S_IFDIR:
    owner->letter = 'd';
    owner->color = light_blue;
    owner->is_dir = 1;
    break;
  case S_IFIFO:
    owner->letter = '|';
    owner->color = light_yellow;
    owner->is_dir = 0;
    break;
  case S_IFLNK:
    owner->letter = 'l';
    owner->color = light_magenta;
    owner->is_dir = 0;
    break;
  case S_IFREG:
    owner->letter = '.';
    owner->color = end;
    owner->is_dir = 0;
    break;
  case S_IFSOCK:
    owner->letter = 's';
    owner->color = light_red;
    owner->is_dir = 0;
    break;
  default:
    owner->letter = '.';
    owner->color = end;
    owner->is_dir = 0;
    break;
  }
}

void
iterate_dir(const char* path) {
  struct dirent* entry;

  /* Path has already been validated */
  DIR* directory = opendir(path);

  while ((entry = readdir(directory))) {
    char buf[PATH_MAX + 1];
    realpath(entry->d_name, buf);

    struct stat stat_res;
    stat(buf, &stat_res);

    struct owner_info f_type_res;
    f_type(stat_res.st_mode, &f_type_res);

    char* perms = file_perm_str(stat_res);

    printf("%s %s", perms, f_type_res.color);
    printf("%s: %s\n", buf, entry->d_name);

    free(perms);
  }

  closedir(directory);
}

int
main(int argc, char* argv[]) {
  if (argc <= 1) {
    iterate_dir("./");
    return 0;
  }

  for (size_t i = 1; i < argc; i++)
    if (is_directory(argv[i]))
      iterate_dir(argv[i]);

  return 0;
}
