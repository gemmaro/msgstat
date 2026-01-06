#include <gettext-po.h>
#include <stdio.h>

void xerror(int severity, po_message_t message, const char *filename,
            size_t lineno, size_t column, int multiline_p,
            const char *message_text) {}

void xerror2(int severity, po_message_t message1, const char *filename1,
             size_t lineno1, size_t column1, int multiline_p1,
             const char *message_text1, po_message_t message2,
             const char *filename2, size_t lineno2, size_t column2,
             int multiline_p2, const char *message_text2) {}

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

  struct po_xerror_handler handler = {
      .xerror = xerror,
      .xerror2 = xerror2,
  };

  po_file_t file = po_file_read(argv[1], &handler);
  po_file_free(file);

  return 0;
}
