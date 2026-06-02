#include "console.h"
#include "../DISPLAY/display.h"
#include "../FILESYSTEM/fs.h"
#include "../INTERRUPTS/kb.h"
#include "../LOG/klog.h"
#include "../WIDGETS/widgets.h"
#include "../GRAPHICS/gfx.h"

#define CONSOLE_MAX_COMMAND 128
#define CONSOLE_MAX_ARGS 4

static char command_buffer[CONSOLE_MAX_COMMAND];
static unsigned int command_length = 0;

static int console_streq(const char *a, const char *b) {
  unsigned int i = 0;
  while(a[i] != 0 && b[i] != 0) {
    if(a[i] != b[i]) return 0;
    i++;
  }
  return a[i] == b[i];
}

static void console_copy(char *dest, const char *src, unsigned int dest_size) {
  unsigned int i;
  if(dest_size == 0) return;
  for(i = 0; i + 1 < dest_size && src[i] != 0; i++) dest[i] = src[i];
  dest[i] = 0;
}

static int console_parse(char *command, char *argv[], int max_args) {
  int argc = 0;
  unsigned int i = 0;

  while(command[i] != 0 && argc < max_args) {
    while(command[i] == ' ') i++;
    if(command[i] == 0) break;
    argv[argc++] = &command[i];
    while(command[i] != 0 && command[i] != ' ') i++;
    if(command[i] == ' ') {
      command[i] = 0;
      i++;
    }
  }

  return argc;
}

static void console_write_uint(unsigned int value) {
  char digits[11];
  unsigned int i = 0;

  if(value == 0) {
    klog_write("0", 1);
    return;
  }

  while(value > 0 && i < sizeof(digits)) {
    digits[i++] = (char)('0' + (value % 10));
    value /= 10;
  }

  while(i > 0) {
    i--;
    klog_write(&digits[i], 1);
  }
}

static void console_print_status(void) {
  widget_draw_status(" x86-os console | help clear ls cat mkdir touch stat widgets gfx ", "COM1+VGA");
}

void console_print_prompt(void) {
  display_set_color(DISPLAY_COLOR_LIGHT_GREEN, DISPLAY_COLOR_BLACK);
  klog_write("x86-os> ", 8);
  display_set_color(DISPLAY_COLOR_LIGHT_GREY, DISPLAY_COLOR_BLACK);
}

static void console_cmd_help(void) {
  klog_write_string("commands:\n");
  klog_write_string("  help              show this message\n");
  klog_write_string("  clear             clear the screen\n");
  klog_write_string("  ls [path]         list a directory\n");
  klog_write_string("  cat <path>        print a file\n");
  klog_write_string("  mkdir <path>      create a directory\n");
  klog_write_string("  touch <path>      create an empty file\n");
  klog_write_string("  stat <path>       show type, size, mode and timestamps\n");
  klog_write_string("  widgets           draw text UI widgets demo\n");
  klog_write_string("  gfx               show framebuffer graphics info\n");
}

static void console_cmd_clear(void) {
  display_clear();
  console_print_status();
}

static void console_cmd_ls(int argc, char *argv[]) {
  struct fs_dir_entry entries[FS_MAX_LIST_ENTRIES];
  const char *path = "/";
  int count;
  int i;

  if(argc > 1) path = argv[1];

  count = vfs_list(path, entries, FS_MAX_LIST_ENTRIES);
  if(count < 0) {
    klog_write_string("ls: could not list directory\n");
    return;
  }

  if(count == 0) {
    klog_write_string("(empty)\n");
    return;
  }

  for(i = 0; i < count; i++) {
    klog_write_string(entries[i].name);
    if(entries[i].type == FS_NODE_DIRECTORY) klog_write("/", 1);
    klog_write("\n", 1);
  }
}

static void console_cmd_cat(int argc, char *argv[]) {
  unsigned char buffer[FS_MAX_DATA_SIZE + 1];
  int read;

  if(argc < 2) {
    klog_write_string("cat: usage: cat <path>\n");
    return;
  }

  read = vfs_read_file(argv[1], buffer, FS_MAX_DATA_SIZE);
  if(read < 0) {
    klog_write_string("cat: could not read file\n");
    return;
  }

  buffer[read] = 0;
  klog_write((const char *)buffer, (unsigned int)read);
  if(read == 0 || buffer[read - 1] != '\n') klog_write("\n", 1);
}

static void console_cmd_mkdir(int argc, char *argv[]) {
  if(argc < 2) {
    klog_write_string("mkdir: usage: mkdir <path>\n");
    return;
  }

  if(vfs_mkdir(argv[1]) < 0) {
    klog_write_string("mkdir: could not create directory\n");
    return;
  }

  klog_write_string("mkdir: created\n");
}

static void console_cmd_touch(int argc, char *argv[]) {
  if(argc < 2) {
    klog_write_string("touch: usage: touch <path>\n");
    return;
  }

  if(vfs_node_from_path(argv[1]) != FS_INVALID_NODE) {
    klog_write_string("touch: already exists\n");
    return;
  }

  if(vfs_create_file(argv[1]) < 0) {
    klog_write_string("touch: could not create file\n");
    return;
  }

  klog_write_string("touch: created\n");
}

static void console_cmd_stat(int argc, char *argv[]) {
  struct fs_stat stat;

  if(argc < 2) {
    klog_write_string("stat: usage: stat <path>\n");
    return;
  }

  if(vfs_stat(argv[1], &stat) < 0) {
    klog_write_string("stat: could not stat path\n");
    return;
  }

  klog_write_string("type: ");
  if(stat.type == FS_NODE_DIRECTORY) klog_write_string("directory\n");
  else if(stat.type == FS_NODE_FILE) klog_write_string("file\n");
  else klog_write_string("unknown\n");

  klog_write_string("size: ");
  console_write_uint(stat.size);
  klog_write("\n", 1);

  klog_write_string("mode: ");
  console_write_uint(stat.mode);
  klog_write("\n", 1);

  klog_write_string("ctime: ");
  console_write_uint(stat.ctime);
  klog_write("\nmtime: ", 8);
  console_write_uint(stat.mtime);
  klog_write("\n", 1);
}


static void console_cmd_widgets(void) {
  static const char *menu_items[] = {
    "Filesystem browser",
    "Process monitor",
    "Settings",
    "Shutdown"
  };
  static const char *list_items[] = {
    "/",
    "/readme",
    "/docs",
    "Use ls/cat/stat commands"
  };
  struct text_menu menu;

  display_clear();
  console_print_status();
  widget_draw_box(1, 2, 36, 8, "Console Widget Demo");
  display_write_at(3, 5, "bordered boxes", 14, DISPLAY_COLOR_WHITE, DISPLAY_COLOR_BLACK);
  display_write_at(4, 5, "status bar", 10, DISPLAY_COLOR_LIGHT_GREEN, DISPLAY_COLOR_BLACK);
  display_write_at(5, 5, "menus + lists", 13, DISPLAY_COLOR_LIGHT_BROWN, DISPLAY_COLOR_BLACK);

  menu.title = "Main Menu";
  menu.items = menu_items;
  menu.count = 4;
  menu.selected = 1;
  widget_draw_menu(1, 42, 30, &menu);

  widget_draw_box(11, 2, 70, 8, "Selectable List");
  widget_draw_list(13, 4, 66, list_items, 4, 2);

  display_set_cursor(21, 0);
  klog_write_string("widgets: text UI demo drawn. Type clear to return to a clean console.\n");
}

static void console_cmd_gfx(void) {
  const struct gfx_state *state = gfx_get_state();

  if(!gfx_is_available()) {
    klog_write_string("gfx: RGB pixel framebuffer not available from multiboot info\n");
    klog_write_string("gfx: text VGA console remains active at 0xb8000\n");
    return;
  }

  klog_write_string("gfx: framebuffer available\nwidth: ");
  console_write_uint(state->width);
  klog_write_string("\nheight: ");
  console_write_uint(state->height);
  klog_write_string("\npitch: ");
  console_write_uint(state->pitch);
  klog_write_string("\nbpp: ");
  console_write_uint(state->bpp);
  klog_write_string("\n");
  gfx_demo();
}

static void console_execute(char *command) {
  char local[CONSOLE_MAX_COMMAND];
  char *argv[CONSOLE_MAX_ARGS];
  int argc;

  console_copy(local, command, sizeof(local));
  argc = console_parse(local, argv, CONSOLE_MAX_ARGS);
  if(argc == 0) return;

  if(console_streq(argv[0], "help")) console_cmd_help();
  else if(console_streq(argv[0], "clear")) console_cmd_clear();
  else if(console_streq(argv[0], "ls")) console_cmd_ls(argc, argv);
  else if(console_streq(argv[0], "cat")) console_cmd_cat(argc, argv);
  else if(console_streq(argv[0], "mkdir")) console_cmd_mkdir(argc, argv);
  else if(console_streq(argv[0], "touch")) console_cmd_touch(argc, argv);
  else if(console_streq(argv[0], "stat")) console_cmd_stat(argc, argv);
  else if(console_streq(argv[0], "widgets")) console_cmd_widgets();
  else if(console_streq(argv[0], "gfx")) console_cmd_gfx();
  else klog_write_string("unknown command; try help\n");
}

static void console_accept_char(char c) {
  if(c == '\n') {
    command_buffer[command_length] = 0;
    klog_write("\n", 1);
    console_execute(command_buffer);
    command_length = 0;
    console_print_prompt();
    return;
  }

  if(command_length + 1 >= CONSOLE_MAX_COMMAND) return;

  command_buffer[command_length++] = c;
  klog_write(&c, 1);
}

static void console_backspace(void) {
  if(command_length == 0) return;
  command_length--;
  display_backspace();
  klog_disable_sink(KLOG_SINK_DISPLAY);
  klog_write("\b", 1);
  klog_enable_sink(KLOG_SINK_DISPLAY);
}

static void console_seed_files(void) {
  const unsigned char readme[] = "Welcome to x86-os. Try: help, ls, cat /readme, stat /readme, mkdir /tmp, touch /tmp/file\n";

  if(vfs_node_from_path("/readme") == FS_INVALID_NODE) {
    int node = vfs_create_file("/readme");
    if(node != FS_INVALID_NODE) vfs_write_file("/readme", readme, sizeof(readme) - 1);
  }

  if(vfs_node_from_path("/docs") == FS_INVALID_NODE) {
    vfs_mkdir("/docs");
  }
}

void console_init(void) {
  command_length = 0;
  console_seed_files();
  console_print_status();
  klog_write_string("Welcome to x86-os text console UI\n");
  klog_write_string("Type 'help' to see commands.\n");
  console_print_prompt();
}

void console_run(void) {
  for(;;) {
    if(keyboard_has_data()) {
      unsigned char scan_code = keyboard_read_scan_code();
      char c;

      if(scan_code & 0x80) continue;

      if(scan_code == KEYBOARD_BACKSPACE_SCAN_CODE) {
        console_backspace();
        continue;
      }

      if(scan_code > KEYBOARD_MAX_ASCII) continue;
      c = (char)keyboard_scan_code_to_ascii(scan_code);
      if(c == 0) continue;
      console_accept_char(c);
    }
  }
}
