#include "mkfs.h"

int interactive;
int write_immed;
int atari_format;
const char *program_name;

typedef struct _link
{
    void *data;
    struct _link *next;
} LINK;

void die(const char *msg, ...)
{
    va_list args;

    if (program_name)
    {
        fprintf(stderr, "%s: ", program_name);
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void pdie(const char *msg, ...)
{
    va_list args;

    if (program_name)
    {
        fprintf(stderr, "%s: ", program_name);
    }
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, ":%s\n", strerror(errno));
    exit(1);
}

void *alloc(int size)
{
    void *this;
    if ((this = malloc(size)))
        return this;
    pdie("malloc");
    return NULL;		/* for GCC */
}

void *qalloc(void **root, int size)
{
    LINK *link;

    link = alloc(sizeof(LINK));
    link->next = *root;
    *root = link;
    return link->data = alloc(size);
}

void qfree(void **root)
{
    LINK *this;

    while (*root)
    {
        this = (LINK *) * root;
        *root = this->next;
        free(this->data);
        free(this);
    }
}

int minimum(int a, int b)
{
    return a < b ? a : b;
}


#ifndef HAVE_VASPRINTF
int vasprintf(char **strp, const char *fmt, va_list va)
{
    int length;
    va_list vacopy;

    va_copy(vacopy, va);

    length = vsnprintf(NULL, 0, fmt, vacopy);
    if (length < 0)
        return length;

    *strp = malloc(length + 1);
    if (!*strp)
    {
        errno = ENOMEM;
        return -1;
    }

    return vsnprintf(*strp, length + 1, fmt, va);
}
#endif

int xasprintf(char **strp, const char *fmt, ...)
{
    va_list va;
    int retval;

    va_start(va, fmt);
    retval = vasprintf(strp, fmt, va);
    va_end(va);

    if (retval < 0)
        pdie("asprintf");

    return retval;
}


int get_choice(int noninteractive_result, const char *noninteractive_msg, int choices, ...)
{
    int choice_values[9];
    const char *choice_strings[9];
    int choice;
    int quit_choice;
    int print_choices, print_full_choices;
    va_list va;
    int i;
    static int inhibit_quit_choice;

    if (!interactive)
    {
        printf("%s\n", noninteractive_msg);
        return noninteractive_result;
    }

    if (choices < 2 || choices > 9)
        die("internal error: invalid number %u of choices in get_choice()",
            choices);

    va_start(va, choices);
    for (i = 0; i < choices; i++)
    {
        choice_values[i] = va_arg(va, int);
        choice_strings[i] = va_arg(va, const char *);
    }
    va_end(va);

    print_choices = 1;
    print_full_choices = 0;
    while (1)
    {
        if (print_choices)
        {
            print_choices = 0;
            for (i = 0; i < choices; i++)
                printf("%d) %s\n", i + 1, choice_strings[i]);

            if (print_full_choices)
            {
                printf("?) List all choices\n");
                printf("q) Quit fsck\n");
            }
        }

        printf("[%.*s?%s]? ", choices, "123456789", inhibit_quit_choice ? "" : "q");
        fflush(stdout);

        do
        {
            choice = getchar();
        }
        while (choice == '\n');    /* filter out enter presses */

        if (choice == EOF)
            exit(1);

        printf("%c\n", choice);

        if (choice > '0' && choice <= '0' + choices)
            break;

        if (choice == '?')
        {
            print_choices = 1;
            print_full_choices = 1;
        }

        if (!inhibit_quit_choice && (choice == 'q' || choice == 'Q'))
        {
            if (!write_immed)
                printf("No changes have been written to the filesystem yet. If you choose\n"
                       "to quit, it will be left in the same state it was in before you\n"
                       "started this program.\n");
            else
                printf("fsck is running in immediate write mode. All changes so far have\n"
                       "already been written and can not be undone now. If you choose to\n"
                       "quit now, these changes will stay in place.\n");

            inhibit_quit_choice = 1;
            quit_choice = get_choice(1, "This is never non-interactive.",
                                     2,
                                     1, "Quit now",
                                     2, "Continue");
            inhibit_quit_choice = 0;

            if (quit_choice == 1)
                exit(0);
        }
    }

    return choice_values[choice - '1'];
}


char *get_line(const char *prompt, char *dest, size_t length)
{
    /*
        struct termios tio, tio_orig;
        int tio_fail;
        char *retval;

        tio_fail = tcgetattr(0, &tio_orig);
        if (!tio_fail) {
    	tio = tio_orig;
    	tio.c_lflag |= ICANON | ECHO;
    	tcsetattr(0, TCSAFLUSH, &tio);
        }

        printf("%s: ", prompt);
        fflush(stdout);

        retval = fgets(dest, length, stdin);

        if (!tio_fail)
    	tcsetattr(0, TCSAFLUSH, &tio_orig);
        return retval;
    */
    return NULL;
}


/*
 * ++roman: On m68k, check if this is an Atari; if yes, turn on Atari variant
 * of MS-DOS filesystem by default.
 */
void check_atari(void)
{
#if defined(__mc68000__) && defined(__linux__) && defined(CONF_CHECK_ATARI)
    FILE *f;
    char line[128], *p;

    if (!(f = fopen("/proc/hardware", "r")))
    {
        perror("/proc/hardware");
        return;
    }

    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "Model:", 6) == 0)
        {
            p = line + 6;
            p += strspn(p, " \t");
            if (strncmp(p, "Atari ", 6) == 0)
                atari_format = 1;
            break;
        }
    }
    fclose(f);
#endif
}


uint32_t generate_volume_id(void)
{
    struct timeval now;

    if (gettimeofday(&now, NULL) != 0 || now.tv_sec == (time_t)-1 || now.tv_sec < 0)
    {
        srand(getpid());
        /* rand() returns int from [0,RAND_MAX], therefore only 31 bits */
        return (((uint32_t)(rand() & 0xFFFF)) << 16) | ((uint32_t)(rand() & 0xFFFF));
    }

    /* volume ID = current time, fudged for more uniqueness */
    return ((uint32_t)now.tv_sec << 20) | (uint32_t)now.tv_usec;
}

/*
 * Validate volume label
 *
 * @param[in]   doslabel   Label stored according to current DOS codepage
 *
 * @return   bitmask of errors
 *           0x01 - lowercase character
 *           0x02 - character below 0x20
 *           0x04 - character in disallowed set
 *           0x08 - empty or space-only label
 *           0x10 - space at beginning
 */
int validate_volume_label(char *doslabel)
{
    int i;
    int ret = 0;
    wchar_t wlabel[12];

    if (dos_string_to_wchar_string(wlabel, doslabel, sizeof(wlabel)))
    {
        for (i = 0; wlabel[i]; i++)
        {
            /* FAT specification: Lower case characters are not allowed in DIR_Name
                                  (what these characters are is country specific)
               Original label is stored in DOS OEM code page, so islower() function
               cannot be used. Therefore convert original label to locale independent
               wchar_t* and then use iswlower() function for it.
            */
            if (iswlower(wlabel[i]))
            {
                ret |= 0x01;
                break;
            }
        }
    }

    /* According to FAT specification those bytes (after conversion to DOS OEM
       code page) are not allowed.
     */
    for (i = 0; i < 11; i++)
    {
        if (doslabel[i] < 0x20)
            ret |= 0x02;
        if (doslabel[i] == 0x22 ||
                (doslabel[i] >= 0x2A && doslabel[i] <= 0x2C) ||
                doslabel[i] == 0x2E ||
                doslabel[i] == 0x2F ||
                (doslabel[i] >= 0x3A && doslabel[i] <= 0x3F) ||
                (doslabel[i] >= 0x5B && doslabel[i] <= 0x5D) ||
                doslabel[i] == 0x7C)
            ret |= 0x04;
    }

    if (memcmp(doslabel, "           ", 11) == 0)
        ret |= 0x08;

    if (doslabel[0] == ' ')
        ret |= 0x10;

    return ret;
}
