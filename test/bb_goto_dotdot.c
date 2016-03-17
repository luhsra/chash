extern char *bltinlookup(char *);
extern int LONE_DASH(const char *);
#define CD_PRINT 1
extern char * path_advance(const char **, const char *);
extern int cdopt(void);
extern char **argptr;
extern char *nullstr;
extern void ash_msg_and_raise_error(const char *, const char *);
extern void out1fmt(const char *, const char *);

extern char * curdir;
extern int docd(const char *, int);

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int
cdcmd(int argc, char **argv)
{
        const char *dest;
        const char *path;
        const char *p;
        char c;
        struct stat statb;
        int flags;

        flags = cdopt();
        dest = *argptr;
        if (!dest)
                dest = bltinlookup("HOME");
        else if (LONE_DASH(dest)) {
                dest = bltinlookup("OLDPWD");
                flags |= CD_PRINT;
        }
        if (!dest)
                dest = nullstr;
        if (*dest == '/')
                goto step7;
        if (*dest == '.') {
                c = dest[1];
{{A}} dotdot:
                switch (c) {
                case '\0':
                case '/':
                        goto step6;
                case '.':
                        c = dest[2];
                        if (c != '.')
				goto dotdot;
                }
        }
        if (!*dest)
                dest = ".";
        path = bltinlookup("CDPATH");
        if (!path) {
 step6:
 step7:
 {{B}} dotdot:

            p = dest;
                goto docd;
        }
        do {
                c = *path;
                p = path_advance(&path, dest);
                if (stat(p, &statb) >= 0 && S_ISDIR(statb.st_mode)) {
                        if (c && c != ':')
                                flags |= CD_PRINT;
 docd:
                        if (!docd(p, flags))
                                goto out;
                        break;
                }
        } while (path);
        ash_msg_and_raise_error("can't cd to %s", dest);
        /* NOTREACHED */
 out:
        if (flags & CD_PRINT)
                out1fmt("%s\n", curdir);
        return 0;
}
/*
 * check-name: Forward and backward gotos
 * assert-obj: A != B
 */
