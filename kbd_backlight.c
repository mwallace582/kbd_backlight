/*
 * Copyright (c) 2016 Jeff Spaulding <sarnet@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>

typedef enum {
     ACTION_NONE = 0,
     ACTION_UP,
     ACTION_DOWN,
     ACTION_SET,
     ACTION_ZERO,
     ACTION_MAX,
} Action_t;

typedef struct {
     Action_t action;
     union {
          int level;
          int increment;
     };
} Arguments_t;

void print_help(char* program, int max_level)
{
     printf("This program alters the key backlight illumination value.\n\n");

     printf("Usage: %s [Options] \n", program);
     printf("Options:\n");
     printf("  -u <increment> Increase brightness by increment.\n");
     printf("  -d <increment> Decrease brightness by increment.\n");
     printf("  -s <level> Set brightness to level between 0 and %d\n\n", max_level);
     printf("  -m Set brightness to maximum value (%d)\n", max_level);
     printf("  -o Set brightness to 0.\n");
     printf("  -h Help. This message\n\n" );

     printf("Examples:\n");
     printf("$ %s -u 5\n", program);
     printf("$ %s -d 10\n", program);
     printf("$ %s -s 100\n", program);
     printf("$ %s -m\n", program);
     printf("$ %s -o\n", program);
     printf("$ %s -m\n\n", program);
}

int get_args(int argc, char* argv[],
             int max_level,
             Arguments_t* args)
{
     int ch;
     while ((ch = getopt(argc, argv, "u:d:s:moh")) != -1) {
          switch (ch) {
          case 'u':
               args->action = ACTION_UP;
               args->increment = atoi(optarg);
               if (args->increment < 0 || args->increment > max_level) {
                    print_help(argv[0], max_level);
                    return EXIT_FAILURE;
               }
               break;
          case 'd':
               args->action = ACTION_DOWN;
               args->increment = atoi(optarg);
               if (args->increment < 0 || args->increment > max_level) {
                    print_help(argv[0], max_level);
                    return EXIT_FAILURE;
               }
               break;
          case 's':
               args->action = ACTION_SET;
               args->level = atoi(optarg);
               if (args->level < 0 || args->level > max_level) {
                    print_help(argv[0], max_level);
                    return EXIT_FAILURE;
               }
               break;
          case 'm':
               args->action = ACTION_MAX;
               break;
          case 'o':
               args->action = ACTION_ZERO;
               break;
          case '?':
          case 'h':
          default:
               print_help(argv[0], max_level);
               return EXIT_FAILURE;
          }

          /* Only process the first argument */
          if (args->action != ACTION_NONE) {
               break;
          }
     }

     return EXIT_SUCCESS;
}

void get_max_level(int* max_level)
{
     const char* max_file = "/sys/class/leds/smc::kbd_backlight/max_brightness";
     FILE* fp_max_level = NULL;

     *max_level = 100; /* Default value */

     fp_max_level = fopen(max_file, "r");
     if (fp_max_level == NULL) {
          fprintf(stderr, "Unable to obtain maximum level from %s", max_file);
          goto clean_exit;
     }

     fscanf(fp_max_level, "%d", max_level);

clean_exit:
     fclose(fp_max_level);
}

int change_level(Arguments_t args,
                 int max_level)
{
     int rc = EXIT_SUCCESS;

     const char* kbd_file = "/sys/class/leds/smc::kbd_backlight/brightness";
     FILE *fp_level = NULL;

     fp_level = fopen(kbd_file, "w+");
     if (fp_level == NULL) {
          err(1, "%s", kbd_file);
          rc = EXIT_FAILURE;
          goto clean_exit;
     }

     int curr_level;
     fscanf(fp_level, "%d", &curr_level);

     int new_level = 0;
     switch (args.action) {
     case ACTION_NONE:
          rc = EXIT_FAILURE;
          goto clean_exit;
     case ACTION_UP:
          new_level = curr_level + args.increment;
          break;
     case ACTION_DOWN:
          new_level = curr_level - args.increment;
          break;
     case ACTION_SET:
          new_level = args.level;
          break;
     case ACTION_ZERO:
          new_level = 0;
          break;
     case ACTION_MAX:
          new_level = max_level;
          break;
     }

     if (new_level > max_level) {
          new_level = max_level;
     }
     else if (new_level < 0) {
          new_level = 0;
     }

     fprintf(fp_level, "%d", new_level);

     printf("Changed level from %d to %d\n",
             curr_level, new_level);

clean_exit:
     fclose(fp_level);
     return rc;
}

int main(int argc, char* argv[])
{
     Arguments_t args = {0};
     int max_level = 0;

     /* Get the maximum brightness level from the system */
     get_max_level(&max_level);

     /* Parse input arguments */
     int rc = get_args(argc, argv, max_level, &args);
     if (rc) {
          return rc;
     }

     /* Change the level as specified in arguments */
     rc = change_level(args, max_level);
     if (rc) {
          return rc;
     }

     return EXIT_SUCCESS;
}
