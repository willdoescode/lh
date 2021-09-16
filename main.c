#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
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
  static char perms_array[(8 + (10 * sizeof(light_white)) * sizeof(char))];
  sprintf(perms_array, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", light_green,
          (f_stat.st_mode & S_IRUSR) ? "r" : "-", light_yellow,
          (f_stat.st_mode & S_IWUSR) ? "w" : "-", light_red,
          (f_stat.st_mode & S_IXUSR) ? "x" : "-", light_green,
          (f_stat.st_mode & S_IRGRP) ? "r" : "-", light_yellow,
          (f_stat.st_mode & S_IWGRP) ? "w" : "-", light_red,
          (f_stat.st_mode & S_IXGRP) ? "x" : "-", light_green,
          (f_stat.st_mode & S_IROTH) ? "r" : "-", light_yellow,
          (f_stat.st_mode & S_IWOTH) ? "w" : "-", light_red,
          (f_stat.st_mode & S_IXOTH) ? "x" : "-", end);

  return perms_array;
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

struct file {
  char* owner;
  char* group;
  char* filename;
  char* perms;
  char* modified_time;
  struct owner_info info;
};

void
handle_individual_file(struct file f, int longest_group, int longest_owner) {
  printf("%s%c%s %s%s ", f.info.color, f.info.letter, f.perms, dark_yellow,
         f.group);
  for (int i = 0; i < (longest_group - strlen(f.group)); i++)
    printf(" ");
  printf("%s ", f.owner);
  for (int i = 0; i < (longest_owner - strlen(f.owner)); i++)
    printf(" ");

  printf("%s%s %s%s\n", light_blue, f.modified_time, f.info.color, f.filename);
}

void
iterate_dir(const char* path) {
  struct dirent* entry;

  /* Path has already been validated */
  DIR* directory = opendir(path);

  struct file* files = malloc(10 * sizeof(struct file));
  int len = 10, pos = 0;

  int longest_group = 0, longest_owner = 0;

  while ((entry = readdir(directory))) {
    char buf[PATH_MAX + 1];
    realpath(entry->d_name, buf);

    struct stat stat_res;
    stat(buf, &stat_res);

    struct owner_info f_type_res;
    f_type(stat_res.st_mode, &f_type_res);

    char* perms = file_perm_str(stat_res);

    struct passwd* pw = getpwuid(stat_res.st_uid);
    struct group* gr = getgrgid(stat_res.st_gid);

    if (strlen(gr->gr_name) > longest_group)
      longest_group = strlen(gr->gr_name);

    if (strlen(pw->pw_name) > longest_owner)
      longest_owner = strlen(pw->pw_name);

    if (pos >= len)
      files = realloc(files,
                      (len * sizeof(struct file)) + (5 * sizeof(struct file)));

    char modified_time[20];
    strftime(modified_time, 20, "%d %b %H:%M %y",
             localtime(&stat_res.st_mtime));

    files[pos++] = (struct file){.owner = pw->pw_name,
                                 .group = gr->gr_name,
                                 .info = f_type_res,
                                 .perms = perms,
                                 .modified_time = modified_time,
                                 .filename = entry->d_name};
    len++;
  }

  for (int i = 0; i < pos; i++)
    handle_individual_file(files[i], longest_group, longest_owner);

  closedir(directory);

  free(files);
}

int
file_exists(char* filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void
validate_path(char* path, int is_dir) {
  if (file_exists(path) && !is_dir) {
    printf("%sError: \"%s\" does not exist.\n", light_red, path);
    exit(1);
  }
}

int
main(int argc, char* argv[]) {
  if (argc <= 1) {
    iterate_dir("./");
    return 0;
  }

  for (size_t i = 1; i < argc; i++) {
    int is_dir = is_directory(argv[i]);
    validate_path(argv[i], is_dir);
    if (is_dir) {
      iterate_dir(argv[i]);
      continue;
    }

    printf("%s", argv[i]);
  }

  return 0;
}
