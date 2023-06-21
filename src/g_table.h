#ifndef G_TABLE_DEF_H
#define G_TABLE_DEF_H

enum { ACCEPT = 0,
       REJECT,
       NEWLINE,
       UPPER,
       LOWER,
       WANTED,
       WANTED_UPPER,
       FILLED,
       END_OF_FILE };

extern unsigned char g_table[256];

#endif /* G_TABLE_DEF_H */
