#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>

const char *homedir;
void help();

int main(int argc, char *argv[]) {
  char c = 0;
  bool skip = false;
  bool do_clear = false;
  bool do_help = false;
  bool do_list = false;
  while((c = getopt(argc, argv, "s:clh")) != -1) {
    switch(c) {
      case 's':
        if (skip) {
          break;
        }

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
    }
  }
  if (do_help) {
    help();
    exit(0);
  }

  // https://stackoverflow.com/a/26696759/6103202
  if ((homedir = getenv("HOME")) == NULL) {
    homedir = getpwuid(getuid())->pw_dir;
  }

  if (do_list) {
    // read goto list and print out
    // goto list is in ~/.gotolist
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
