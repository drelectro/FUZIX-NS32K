#ifndef __READLINE_H
#define __READLINE_H

#include <stddef.h>

/* Standard readline API */
extern char *readline(const char *__prompt);

/* Fuzix API's */
extern int rl_edit (int __fd, int __ofd, const char *__prompt, char *__input,
                    size_t __len);
extern void rl_hinit(char *__buffer, size_t __len);

/*
 * Optional tab completion hook.
 *
 * The callback is invoked when the user presses TAB.
 * - __buf is the current line buffer (not NUL-terminated).
 * - __len is the current line length.
 * - __cursor is the cursor position (0..__len).
 *
 * The callback should write a NUL-terminated string into __insert containing
 * characters to be inserted at the cursor. Return value:
 *   >0 : number of chars to insert from __insert
 *    0 : no completion available (readline will beep)
 *   <0 : reserved (treated as no completion)
 */
typedef int (*rl_complete_fn_t)(const char *__buf, size_t __len, size_t __cursor,
                                char *__insert, size_t __insert_len);
extern void rl_set_complete(rl_complete_fn_t __fn);

#endif
