#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define IS_NULLGOTO(list) (list->alias == NULL)

typedef struct _gotolist {
  char *alias;
  char *path;
} gotolist;

const char *g_homedir;
void help();
gotolist **read_gotolist();
void save_gotolist();

gotolist **g_gotolist;
unsigned int g_gotolist_size;
char *g_gotolist_path = NULL;
int main(int argc, char *argv[]) {
  char c = 0;
  char *alias = NULL;
  bool skip = false;
  bool do_set = false;
  bool do_clear = false;
  bool do_help = false;
  bool do_list = false;

  char *new_directory = NULL; /* used in set(-s) */
  while((c = getopt(argc, argv, "s:clh")) != -1) {
    switch(c) {
      case 's':
        if (skip) {
          break;
        }
        new_directory = strdup(optarg);
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
  if (do_help || (!do_list && alias == NULL)) {
    help();
    exit(0);
  }

  bool alias_is_num = true;
  if (!do_list) {
    for(int i=0; alias[i]; ++i) {
      if(!isdigit(alias[i])) { // if alias is all-digit, it's okay
        if (!(alias[0] >= 0x41 && alias[0] <= 0x5a
              || alias[0] >= 0x61 && alias[0] <= 0x7a)) {
          fprintf(stderr, "alias must start with alphabet: %s", alias);
          exit(-1);
        }
        alias_is_num = false;
      }
    }
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
  // g_gotolist_size is set in read_gotolist()
  g_gotolist = read_gotolist(&g_gotolist_size);

  if (do_list) {
    // read goto list and print out
    // goto list is in ~/.gotolist
    for (int i=0; !IS_NULLGOTO(g_gotolist[i]); ++i) {
      gotolist *cur_goto = g_gotolist[i];
      printf("\t%-6d %-16s\t%s\n", i, cur_goto->alias, cur_goto->path);
    }
    exit(0);
  }

  if (do_set) {
    // if already in gotolist, just change it

    // find lastidx first
    int lastidx = 0;
    for (; !IS_NULLGOTO(g_gotolist[lastidx]); ++lastidx) {}

    bool changed = false;
    int idx = 0;
    if (alias_is_num) {
      unsigned int alias_num = atoi(alias);
      if (alias_num > lastidx) {
        fprintf(stderr, "alias #%d doesn't exist\n", alias_num);
        exit(-1);
      }
      gotolist *cur_goto = g_gotolist[alias_num];
      free(cur_goto->path);
      cur_goto->path = new_directory;
      changed = true;
    }
    else {
      for (; !IS_NULLGOTO(g_gotolist[idx]); ++idx) {
        gotolist *cur_goto = g_gotolist[idx];
        if (strcmp(cur_goto->alias, alias) == 0) {
          free(cur_goto->path);
          cur_goto->path = new_directory;
          changed = true;
          break;
        }
      } // idx will be index of nullgoto in g_gotolist if changed != true
    }
    if (!changed) {
      // if not, add new alias & directory
      gotolist *new_goto = (gotolist *)malloc(sizeof(gotolist));
      new_goto->alias = alias;
      new_goto->path = new_directory;
      gotolist *null_goto = g_gotolist[idx];

      g_gotolist = realloc(g_gotolist, g_gotolist_size + strlen(alias) 
                            + strlen(new_directory) + 2/* space & \n */);
      g_gotolist[idx++] = new_goto;
      g_gotolist[idx] = null_goto;
    }
    
    // flush g_gotolist to file
    save_gotolist();
    exit(0);
  }

  if (do_clear) {
    // find index of nullgoto
    int lastidx = 0;
    for (; !IS_NULLGOTO(g_gotolist[lastidx]); ++lastidx) {}

    if (alias_is_num) {
      unsigned int alias_num = atoi(alias);
      if (alias_num > lastidx) {
        fprintf(stderr, "alias #%d doesn't exist\n", alias_num);
        exit(-1);
      }
      memcpy(&g_gotolist[alias_num], &g_gotolist[alias_num + 1], (lastidx - alias_num) * sizeof(gotolist *));
    }
    else {
      for (int i=0; !IS_NULLGOTO(g_gotolist[i]); ++i) {
        if (strcmp(g_gotolist[i]->alias, alias) == 0) {
          memcpy(&g_gotolist[i], &g_gotolist[i + 1], (lastidx - i) * sizeof(gotolist *));
        }
      }
    }

    save_gotolist();
    exit(0);
  }
  
  // print directory for alias for shellscript
  int lastidx = 0;
  for (; !IS_NULLGOTO(g_gotolist[lastidx]); ++lastidx) {}

  if (alias_is_num) {
    unsigned int alias_num = atoi(alias);
    if (alias_num > lastidx) {
      fprintf(stderr, "alias #%d doesn't exist\n", alias_num);
      exit(-1);
    }
    puts(g_gotolist[alias_num]->path);
    exit(1);
  }
  else {
    for (int i=0; !IS_NULLGOTO(g_gotolist[i]); ++i) {
      if (strcmp(g_gotolist[i]->alias, alias) == 0) {
        puts(g_gotolist[i]->path);
        exit(1); // 1 for goto return
      }
    }
  }
  exit(-1);
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
  for (int i=0; i<sizeof(usage)/sizeof(struct command_usage); ++i) {
    fprintf(stderr, "\t%-16s\t%s\n", usage[i].option, usage[i].usage);
  }
}

gotolist **read_gotolist(size_t *list_size) {
  char *line = NULL;
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

  g_gotolist_path = (char *)malloc(gotolist_path_length + 1);
  memset(g_gotolist_path, 0, gotolist_path_length + 1);
  snprintf(g_gotolist_path, gotolist_path_length + 1, "%s/.gotolist", g_homedir);

  fp = fopen(g_gotolist_path, "r");
  if (fp == NULL) {
    perror("fopen");
    free(g_gotolist_path);
    g_gotolist_path = NULL;
    printf("returning NULL\n");
    return NULL;
  }

  // get file size first
  // parsed_list = malloc(file_size+8(NULL)); will be safe enough for gotolist
  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  parsed_list = (gotolist **)malloc(file_size);
  *list_size = file_size;

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

void save_gotolist() {
  FILE *fp = NULL;
  fp = fopen(g_gotolist_path, "w+");
  for (int i=0; !IS_NULLGOTO(g_gotolist[i]); ++i) {
    fprintf(fp, "%s %s\n", g_gotolist[i]->alias, g_gotolist[i]->path);
  }
  fclose(fp);
}
