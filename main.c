#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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
  static char perms_array[9 * (MAX_COLOR_LEN + 1)];
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
mode_info(mode_t mode, struct owner_info* owner) {
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
  char* f_size;
  struct owner_info info;
};

void
handle_individual_file(struct file f, int longest_size, int longest_group,
                       int longest_owner) {
  printf("%s%c%s %s%s ", f.info.color, f.info.letter, f.perms, dark_yellow,
         f.group);

  for (int i = 0; i < (longest_group - strlen(f.group)); i++)
    printf(" ");

  printf("%s%s ", f.info.is_dir ? light_black : light_green, f.f_size);

  for (int i = 0; i < (longest_size - strlen(f.f_size)); i++)
    printf(" ");

  printf("%s%s ", dark_yellow, f.owner);

  for (int i = 0; i < (longest_owner - strlen(f.owner)); i++)
    printf(" ");

  printf("%s%s %s%s%s\n", light_blue, f.modified_time, f.info.color, f.filename,
         end);

  free(f.group);
  free(f.owner);
  free(f.filename);
  if (f.info.is_dir)
    free(f.f_size);
}

int
round_size(int n) {
  float d = n * 100.0;
  int i = d + 0.5;
  d = i / 100;
  return d;
}

char*
size_to_str(uintmax_t size) {
  static const char* sizes[] = {"B", "KB", "MB", "GB"};

  int div = 0;
  size_t rem = 0;

  while (size >= 1024 && div < 4) {
    rem = (size % 1024);
    div++;
    size /= 1024;
  }

  float size_d = size + rem / 1024.;
  static char f_size[10];
  sprintf(f_size, "%d %s", round_size(size_d), sizes[div]);

  return f_size;
}

struct files {
  struct file* files;
  int longest_size;
  int longest_group;
  int longest_owner;
  int length;
};

struct files
iterate_dir(const char* path) {
  struct dirent* entry;

  /* Path has already been validated */
  DIR* directory = opendir(path);

  /* Skip . and .. */
  readdir(directory);
  readdir(directory);

  int size = 10;
  int position = 0;
  struct file* files = malloc(10 * sizeof(struct file));

  int longest_size = 0, longest_group = 0, longest_owner = 0;

  while ((entry = readdir(directory))) {
    char buf[PATH_MAX + 1];
    realpath(entry->d_name, buf);

    struct stat stat_res;
    stat(buf, &stat_res);

    struct owner_info f_type_res;
    mode_info(stat_res.st_mode, &f_type_res);

    char* perms = file_perm_str(stat_res);

    struct passwd* pw = getpwuid(stat_res.st_uid);
    struct group* gr = getgrgid(stat_res.st_gid);

    char* dir_size = malloc(sizeof(char) * 1);
    strcpy(dir_size, "-");

    char* f_size;

    if (f_type_res.is_dir)
      f_size = dir_size;
    else {
      f_size = size_to_str(stat_res.st_size);
      free(dir_size);
    }

    if (strlen(gr->gr_name) > longest_group)
      longest_group = strlen(gr->gr_name);

    if (strlen(pw->pw_name) > longest_owner)
      longest_owner = strlen(pw->pw_name);

    if (strlen(f_size) > longest_size)
      longest_size = strlen(f_size);

    if (position == size) {
      size += 5;
      struct file* realloc_files = realloc(files, (size * sizeof(struct file)));
      if (realloc_files)
        files = realloc_files;
      else
        printf("%sError: failed to extend array.", light_red);
    }

    char modified_time[20];
    if (!strftime(modified_time, 20, "%d %b %H:%M %y",
                  localtime(&stat_res.st_mtime)))
      modified_time[0] = ' ';

    char* filename = malloc(entry->d_namlen * sizeof(char));
    strcpy(filename, entry->d_name);

    char* o_name = malloc(strlen(pw->pw_name) * sizeof(char));
    strcpy(o_name, pw->pw_name);

    char* g_name = malloc(strlen(gr->gr_name) * sizeof(char));
    strcpy(g_name, gr->gr_name);

    files[position++] = (struct file){.owner = o_name,
                                      .group = g_name,
                                      .info = f_type_res,
                                      .perms = perms,
                                      .f_size = f_size,
                                      .modified_time = modified_time,
                                      .filename = filename};
  }

  closedir(directory);

  return (struct files){.files = files,
                        .length = position,
                        .longest_size = longest_size,
                        .longest_group = longest_group,
                        .longest_owner = longest_owner};
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

void
handle_files(struct files f) {
  for (int i = 0; i < f.length; i++) {
    handle_individual_file(f.files[i], f.longest_size, f.longest_group,
                           f.longest_owner);
  }

  free(f.files);
}

int
main(int argc, char** argv) {
  if (argc <= 1) {
    handle_files(iterate_dir("./"));
    return 0;
  }

  for (size_t i = 1; i < argc; i++) {
    int is_dir = is_directory(argv[i]);
    validate_path(argv[i], is_dir);
    if (is_dir) {
      handle_files(iterate_dir(argv[i]));

      continue;
    }
  }

  return 0;
}
