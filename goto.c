#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

typedef struct _gotolist {
  char *alias;
  char *path;
} gotolist;

const char *g_homedir;
void help();
gotolist **read_gotolist();

gotolist **g_gotolist;
int main(int argc, char *argv[]) {
  char c = 0;
  const char *alias = NULL;
  bool skip = false;
  bool do_set = false;
  bool do_clear = false;
  bool do_help = false;
  bool do_list = false;
  while((c = getopt(argc, argv, "s:clh")) != -1) {
    switch(c) {
      case 's':
        if (skip) {
          break;
        }
        do_set = true;
        skip = true;
        break;
      case 'c':
        if (skip) {
          break;
        }
        do_clear = true;
        skip = true;
        break;
      case 'l':
        do_list = true;
        break;
      case 'h':
        do_help = true;
        break;
      default:
        break;
    }
  }
  alias = argv[optind];
  if (do_help || (!do_list && alias ==NULL)) {
    help();
    exit(0);
  }

  // https://stackoverflow.com/a/26696759/6103202
  if ((g_homedir = getenv("HOME")) == NULL) {
    g_homedir = getpwuid(getuid())->pw_dir;
    if (g_homedir == NULL) {
      fprintf(stderr, "Failed to get $HOME\n");
      exit(-1);
    }
  }

  // read or create .gotolist
  g_gotolist = read_gotolist();

  if (do_list) {
    // read goto list and print out
    // goto list is in ~/.gotolist
    for(int i=0; g_gotolist[i]->alias != NULL; ++i) {
      gotolist *cur_goto = g_gotolist[i];
      printf("\t%-6d %-16s\t%s\n", i, cur_goto->alias, cur_goto->path);
    }
    exit(0);
  }

  if (do_set) {

  }

}

void help() {
  struct command_usage {
    const char *option;
    const char *usage;
  } usage[] = {
    {"-h", "Print this message"},
    {"-l", "List aliases"},
    {"-s <directory>", "Set directory to alias"},
    {"-c <alias>", "Clear alias"}
  };

  fprintf(stderr, "usage: goto [OPTION] [ALIAS]\n\n");
  for(int i=0; i<sizeof(usage)/sizeof(struct command_usage); ++i) {
    fprintf(stderr, "\t%-16s\t%-128s\n", usage[i].option, usage[i].usage);
  }
}

gotolist **read_gotolist() {

  char *line = NULL;
  char *gotolist_path = NULL;
  FILE *fp = NULL;
  size_t len = 0;
  size_t file_size = 0;
  size_t parsed_count = 0;
  unsigned int gotolist_path_length = 0;

  gotolist **parsed_list;

  gotolist_path_length = strlen(g_homedir) + strlen("/.gotolist");
  if (gotolist_path_length > 4096) {
    fprintf(stderr, "$HOME is too long!\n");
    exit(-1);
  }

  gotolist_path = (char *)malloc(gotolist_path_length + 1);
  memset(gotolist_path, 0, gotolist_path_length + 1);
  snprintf(gotolist_path, gotolist_path_length + 1, "%s/.gotolist", g_homedir);

  printf("opening: %s\n", gotolist_path);
  fp = fopen(gotolist_path, "r");
  if (fp == NULL) {
    perror("fopen");
    free(gotolist_path);
    gotolist_path = NULL;
    printf("returning NULL\n");
    return NULL;
  }

  // get file size first
  // parsed_list = malloc(file_size+8(NULL)); will be safe enough for gotolist
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  parsed_list = (gotolist **)malloc(file_size);

  while(getline(&line, &len, fp) != -1) {
    size_t alias_len = 0;
    size_t path_len = 0;
    gotolist *cur_goto = (gotolist *)malloc(sizeof(gotolist));
    char *first_space = strchr(line, ' ');
    char *end_of_line = strchr(line, '\n');

    alias_len = first_space - line;
    cur_goto->alias = malloc(alias_len + 1);
    memset(cur_goto->alias, 0, alias_len + 1);
    snprintf(cur_goto->alias, alias_len + 1, "%s", line);

    path_len = end_of_line - first_space;
    cur_goto->path = malloc(path_len + 1);
    memset(cur_goto->path, 0, path_len + 1);
    snprintf(cur_goto->path, path_len, "%s", first_space + 1);

    parsed_list[parsed_count++] = cur_goto;
  }

  gotolist *nullgoto = malloc(sizeof(gotolist));
  nullgoto->alias = NULL;
  nullgoto->path = NULL;
  parsed_list[parsed_count] = nullgoto;

  free(line);
  line = NULL;
  fclose(fp);
  return parsed_list;
}
