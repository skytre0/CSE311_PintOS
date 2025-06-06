#include <list.h>

struct frame {
    void *address;
    uint8_t *page;           /* Saved page directory. */
    int tid;
    struct list_elem frame_elem;   /* List element for the frame list. */
};

//  contains a pointer to the page,

struct list frame_table;