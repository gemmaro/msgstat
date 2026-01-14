/* Copyright (C) 2025  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gettext-po.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: more helpful error handling */
void msgstat_xerror(int severity, po_message_t message, const char *filename,
                    size_t lineno, size_t column, int multiline_p,
                    const char *message_text) {
  fputs("* unexpected error", stderr);
  exit(1);
}
void msgstat_xerror2(int severity, po_message_t message1, const char *filename1,
                     size_t lineno1, size_t column1, int multiline_p1,
                     const char *message_text1, po_message_t message2,
                     const char *filename2, size_t lineno2, size_t column2,
                     int multiline_p2, const char *message_text2) {
  fputs("* unexpected error", stderr);
  exit(1);
}

typedef struct msgstat_stat {
  unsigned int untranslated;
  unsigned int translated;
  unsigned int fuzzy;
} msgstat_stat;

#define MSGSTAT_STAT_CREATE(stat)                                              \
  stat = malloc(sizeof(msgstat_stat));                                         \
  stat->untranslated = 0;                                                      \
  stat->translated = 0;                                                        \
  stat->fuzzy = 0;

#define MSGSTAT_STAT_FREE(stat) free(stat);

void msgstat_stat_add(msgstat_stat *const stat,
                      const msgstat_stat *const delta) {
  stat->untranslated += delta->untranslated;
  stat->translated += delta->translated;
  stat->fuzzy += delta->fuzzy;
}

#define MSGSTAT_STAT_ALL(stat)                                                 \
  (stat->untranslated + stat->translated + stat->fuzzy)
#define MSGSTAT_STAT_PERCENT(stat, field)                                      \
  ((double)stat->field / MSGSTAT_STAT_ALL(stat)) * 100

#define MSGSTAT_STAT_PRINT(stat)                                               \
  stat->translated == MSGSTAT_STAT_ALL(stat)                                   \
      ? printf("complete✔️")                                               \
      : printf("u: %05.1f\tf: %05.1f\tt: %05.1f\trest: %d",                    \
               MSGSTAT_STAT_PERCENT(stat, untranslated),                       \
               MSGSTAT_STAT_PERCENT(stat, fuzzy),                              \
               MSGSTAT_STAT_PERCENT(stat, translated),                         \
               stat->untranslated + stat->fuzzy);

#define MSGSTAT_STAT_FROM_MESSAGE(message, stat)                               \
  if (strlen(po_message_msgstr(message))) {                                    \
    if (po_message_is_fuzzy(message))                                          \
      stat->fuzzy++;                                                           \
    else                                                                       \
      stat->translated++;                                                      \
  } else                                                                       \
    stat->untranslated++;

typedef struct msgstat_stats {
  const char **filenames; /* NULL means no file */
  msgstat_stat **stats;
  size_t size;
} msgstat_stats;

#define MSGSTAT_STATS_CREATE(stats)                                            \
  stats = malloc(sizeof(msgstat_stats));                                       \
  stats->filenames = NULL;                                                     \
  stats->stats = NULL;                                                         \
  stats->size = 0;

#define MSGSTAT_STATS_FREE(stats)                                              \
  for (size_t index = 0; index < stats->size; index++)                         \
    MSGSTAT_STAT_FREE(stats->stats[index]);                                    \
  free(stats->filenames);                                                      \
  free(stats->stats);                                                          \
  free(stats);

#define MSGSTAT_STATS_PRINT(stats)                                             \
  if (stats->size) {                                                           \
    msgstat_stat *sum;                                                         \
    MSGSTAT_STAT_CREATE(sum);                                                  \
    for (size_t index = 0; index < stats->size; index++) {                     \
      printf("%s:\t", stats->filenames[index]);                                \
      MSGSTAT_STAT_PRINT(stats->stats[index]);                                 \
      putchar('\n');                                                           \
      msgstat_stat_add(sum, stats->stats[index]);                              \
    }                                                                          \
    printf("all:\t");                                                          \
    MSGSTAT_STAT_PRINT(sum);                                                   \
    putchar('\n');                                                             \
    MSGSTAT_STAT_FREE(sum);                                                    \
  } else {                                                                     \
    fputs("no data", stderr);                                                  \
  }

void msgstat_stats_add_from_message_for_filename(const po_message_t message,
                                                 const char *const filename,
                                                 msgstat_stats *const stats) {
  if (stats->size) {
    for (size_t index = 0; index < stats->size; index++) {
      const char *const filename_in_stats = stats->filenames[index];
      if (!strcmp(filename_in_stats, filename)) {
        MSGSTAT_STAT_FROM_MESSAGE(message, stats->stats[index]);
        return;
      }
    }
  }

  stats->size++;

  const char **const new_filenames =
      realloc(stats->filenames, sizeof(char *) * (stats->size));
  if (!new_filenames) {
    fputs("no memory\n", stderr);
    exit(1);
  }
  stats->filenames = new_filenames;
  stats->filenames[stats->size - 1] = filename;

  msgstat_stat **const new_stats =
      realloc(stats->stats, sizeof(msgstat_stat *) * (stats->size));
  if (!new_stats) {
    fputs("no memory", stderr);
    exit(1);
  }
  stats->stats = new_stats;
  msgstat_stat *stat;
  MSGSTAT_STAT_CREATE(stat);
  MSGSTAT_STAT_FROM_MESSAGE(message, stat);
  stats->stats[stats->size - 1] = stat;
}

void msgstat_stats_add_from_message(const po_message_t message,
                                    msgstat_stats *const stats) {
  po_filepos_t filepos;
  for (int file_position_index = 0;
       (filepos = po_message_filepos(message, file_position_index));
       file_position_index++) {
    const char *const filename = po_filepos_file(filepos);
    msgstat_stats_add_from_message_for_filename(message, filename, stats);
  }
}

void msgstat_print_for_domain(const po_file_t file,
                              const char *const *domainp) {
  const po_message_iterator_t iterator = po_message_iterator(file, *domainp);
  msgstat_stats *stats;
  MSGSTAT_STATS_CREATE(stats);
  po_message_t message;
  while ((message = po_next_message(iterator)))
    if (!po_message_is_obsolete(message))
      msgstat_stats_add_from_message(message, stats);
  MSGSTAT_STATS_PRINT(stats);
  MSGSTAT_STATS_FREE(stats);
  po_message_iterator_free(iterator);
}

int main(const int argc, const char *const argv[]) {
  /* const bool debug = getenv("MSGSTAT_DEBUG"); */
  if (argc != 2) {
    fputs("wrong number of arguments\n", stderr);
    exit(1);
  }
  const char *const filename = argv[1];
  const struct po_xerror_handler handler = {
      .xerror = msgstat_xerror,
      .xerror2 = msgstat_xerror2,
  };
  const po_file_t file = po_file_read(filename, &handler);
  for (const char *const *domainp = po_file_domains(file); *domainp; domainp++)
    msgstat_print_for_domain(file, domainp);
  po_file_free(file);
  return 0;
}
