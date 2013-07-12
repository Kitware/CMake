/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id$
 ***************************************************************************/

/***


RECEIVING COOKIE INFORMATION
============================

struct CookieInfo *cookie_init(char *file);

        Inits a cookie struct to store data in a local file. This is always
        called before any cookies are set.

int cookies_set(struct CookieInfo *cookie, char *cookie_line);

        The 'cookie_line' parameter is a full "Set-cookie:" line as
        received from a server.

        The function need to replace previously stored lines that this new
        line superceeds.

        It may remove lines that are expired.

        It should return an indication of success/error.


SENDING COOKIE INFORMATION
==========================

struct Cookies *cookie_getlist(struct CookieInfo *cookie,
                               char *host, char *path, bool secure);

        For a given host and path, return a linked list of cookies that
        the client should send to the server if used now. The secure
        boolean informs the cookie if a secure connection is achieved or
        not.

        It shall only return cookies that haven't expired.


Example set of cookies:

    Set-cookie: PRODUCTINFO=webxpress; domain=.fidelity.com; path=/; secure
    Set-cookie: PERSONALIZE=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/ftgw; secure
    Set-cookie: FidHist=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: FidOrder=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: DisPend=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie: FidDis=none;expires=Monday, 13-Jun-1988 03:04:55 GMT;
    domain=.fidelity.com; path=/; secure
    Set-cookie:
    Session_Key@6791a9e0-901a-11d0-a1c8-9b012c88aa77=none;expires=Monday,
    13-Jun-1988 03:04:55 GMT; domain=.fidelity.com; path=/; secure
****/


#include "setup.h"

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)

#include <stdlib.h>
#include <string.h>

#define _MPRINTF_REPLACE /* without this on windows OS we get undefined reference to snprintf */
#include <curl/mprintf.h>

#include "urldata.h"
#include "cookie.h"
#include "strequal.h"
#include "strtok.h"
#include "sendf.h"
#include "memory.h"
#include "share.h"
#include "strtoofft.h"

/* The last #include file should be: */
#ifdef CURLDEBUG
#include "memdebug.h"
#endif

#define my_isspace(x) ((x == ' ') || (x == '\t'))

static void freecookie(struct Cookie *co)
{
  if(co->expirestr)
    free(co->expirestr);
  if(co->domain)
    free(co->domain);
  if(co->path)
    free(co->path);
  if(co->name)
    free(co->name);
  if(co->value)
    free(co->value);
  if(co->maxage)
    free(co->maxage);
  if(co->version)
    free(co->version);

  free(co);
}

static bool tailmatch(const char *little, const char *bigone)
{
  size_t littlelen = strlen(little);
  size_t biglen = strlen(bigone);

  if(littlelen > biglen)
    return FALSE;

  return (bool)strequal(little, bigone+biglen-littlelen);
}

/*
 * Load cookies from all given cookie files (CURLOPT_COOKIEFILE).
 */
void Curl_cookie_loadfiles(struct SessionHandle *data)
{
  struct curl_slist *list = data->change.cookielist;
  if(list) {
    Curl_share_lock(data, CURL_LOCK_DATA_COOKIE, CURL_LOCK_ACCESS_SINGLE);
    while(list) {
      data->cookies = Curl_cookie_init(data,
                                       list->data,
                                       data->cookies,
                                       data->set.cookiesession);
      list = list->next;
    }
    Curl_share_unlock(data, CURL_LOCK_DATA_COOKIE);
    curl_slist_free_all(data->change.cookielist); /* clean up list */
    data->change.cookielist = NULL; /* don't do this again! */
  }
}

/****************************************************************************
 *
 * Curl_cookie_add()
 *
 * Add a single cookie line to the cookie keeping object.
 *
 ***************************************************************************/

struct Cookie *
Curl_cookie_add(struct SessionHandle *data,
                /* The 'data' pointer here may be NULL at times, and thus
                   must only be used very carefully for things that can deal
                   with data being NULL. Such as infof() and similar */

                struct CookieInfo *c,
                bool httpheader, /* TRUE if HTTP header-style line */
                char *lineptr,   /* first character of the line */
                char *domain,    /* default domain */
                char *path)      /* full path used when this cookie is set,
                                    used to get default path for the cookie
                                    unless set */
{
  struct Cookie *clist;
  char *what;
  char name[MAX_NAME];
  char *ptr;
  char *semiptr;
  struct Cookie *co;
  struct Cookie *lastc=NULL;
  time_t now = time(NULL);
  bool replace_old = FALSE;
  bool badcookie = FALSE; /* cookies are good by default. mmmmm yummy */

  /* First, alloc and init a new struct for it */
  co = (struct Cookie *)calloc(sizeof(struct Cookie), 1);
  if(!co)
    return NULL; /* bail out if we're this low on memory */

  if(httpheader) {
    /* This line was read off a HTTP-header */
    char *sep;

    what = malloc(MAX_COOKIE_LINE);
    if(!what) {
      free(co);
      return NULL;
    }

    semiptr=strchr(lineptr, ';'); /* first, find a semicolon */

    while(*lineptr && my_isspace(*lineptr))
      lineptr++;

    ptr = lineptr;
    do {
      /* we have a <what>=<this> pair or a 'secure' word here */
      sep = strchr(ptr, '=');
      if(sep && (!semiptr || (semiptr>sep)) ) {
        /*
         * There is a = sign and if there was a semicolon too, which make sure
         * that the semicolon comes _after_ the equal sign.
         */

        name[0]=what[0]=0; /* init the buffers */
        if(1 <= sscanf(ptr, "%" MAX_NAME_TXT "[^;=]=%"
                       MAX_COOKIE_LINE_TXT "[^;\r\n]",
                       name, what)) {
          /* this is a <name>=<what> pair */

          char *whatptr;

          /* Strip off trailing whitespace from the 'what' */
          size_t len=strlen(what);
          while(len && my_isspace(what[len-1])) {
            what[len-1]=0;
            len--;
          }

          /* Skip leading whitespace from the 'what' */
          whatptr=what;
          while(my_isspace(*whatptr)) {
            whatptr++;
          }

          if(strequal("path", name)) {
            co->path=strdup(whatptr);
            if(!co->path) {
              badcookie = TRUE; /* out of memory bad */
              break;
            }
          }
          else if(strequal("domain", name)) {
            /* note that this name may or may not have a preceeding dot, but
               we don't care about that, we treat the names the same anyway */

            const char *domptr=whatptr;
            int dotcount=1;

            /* Count the dots, we need to make sure that there are enough
               of them. */

            if('.' == whatptr[0])
              /* don't count the initial dot, assume it */
              domptr++;

            do {
              domptr = strchr(domptr, '.');
              if(domptr) {
                domptr++;
                dotcount++;
              }
            } while(domptr);

            /* The original Netscape cookie spec defined that this domain name
               MUST have three dots (or two if one of the seven holy TLDs),
               but it seems that these kinds of cookies are in use "out there"
               so we cannot be that strict. I've therefore lowered the check
               to not allow less than two dots. */

            if(dotcount < 2) {
              /* Received and skipped a cookie with a domain using too few
                 dots. */
              badcookie=TRUE; /* mark this as a bad cookie */
              infof(data, "skipped cookie with illegal dotcount domain: %s\n",
                    whatptr);
            }
            else {
              /* Now, we make sure that our host is within the given domain,
                 or the given domain is not valid and thus cannot be set. */

              if('.' == whatptr[0])
                whatptr++; /* ignore preceeding dot */

              if(!domain || tailmatch(whatptr, domain)) {
                const char *tailptr=whatptr;
                if(tailptr[0] == '.')
                  tailptr++;
                co->domain=strdup(tailptr); /* don't prefix w/dots
                                               internally */
                if(!co->domain) {
                  badcookie = TRUE;
                  break;
                }
                co->tailmatch=TRUE; /* we always do that if the domain name was
                                       given */
              }
              else {
                /* we did not get a tailmatch and then the attempted set domain
                   is not a domain to which the current host belongs. Mark as
                   bad. */
                badcookie=TRUE;
                infof(data, "skipped cookie with bad tailmatch domain: %s\n",
                      whatptr);
              }
            }
          }
          else if(strequal("version", name)) {
            co->version=strdup(whatptr);
            if(!co->version) {
              badcookie = TRUE;
              break;
            }
          }
          else if(strequal("max-age", name)) {
            /* Defined in RFC2109:

               Optional.  The Max-Age attribute defines the lifetime of the
               cookie, in seconds.  The delta-seconds value is a decimal non-
               negative integer.  After delta-seconds seconds elapse, the
               client should discard the cookie.  A value of zero means the
               cookie should be discarded immediately.

             */
            co->maxage = strdup(whatptr);
            if(!co->maxage) {
              badcookie = TRUE;
              break;
            }
            co->expires =
              atoi((*co->maxage=='\"')?&co->maxage[1]:&co->maxage[0]) + (long)now;
          }
          else if(strequal("expires", name)) {
            co->expirestr=strdup(whatptr);
            if(!co->expirestr) {
              badcookie = TRUE;
              break;
            }
            co->expires = curl_getdate(what, &now);
          }
          else if(!co->name) {
            co->name = strdup(name);
            co->value = strdup(whatptr);
            if(!co->name || !co->value) {
              badcookie = TRUE;
              break;
            }
          }
          /*
            else this is the second (or more) name we don't know
            about! */
        }
        else {
          /* this is an "illegal" <what>=<this> pair */
        }
      }
      else {
        if(sscanf(ptr, "%" MAX_COOKIE_LINE_TXT "[^;\r\n]",
                  what)) {
          if(strequal("secure", what))
            co->secure = TRUE;
          /* else,
             unsupported keyword without assign! */

        }
      }
      if(!semiptr || !*semiptr) {
        /* we already know there are no more cookies */
        semiptr = NULL;
        continue;
      }

      ptr=semiptr+1;
      while(ptr && *ptr && my_isspace(*ptr))
        ptr++;
      semiptr=strchr(ptr, ';'); /* now, find the next semicolon */

      if(!semiptr && *ptr)
        /* There are no more semicolons, but there's a final name=value pair
           coming up */
        semiptr=strchr(ptr, '\0');
    } while(semiptr);

    if(!badcookie && !co->domain) {
      if(domain) {
        /* no domain was given in the header line, set the default */
        co->domain=strdup(domain);
        if(!co->domain)
          badcookie = TRUE;
      }
    }

    if(!badcookie && !co->path && path) {
      /* no path was given in the header line, set the default  */
      char *endslash = strrchr(path, '/');
      if(endslash) {
        size_t pathlen = endslash-path+1; /* include the ending slash */
        co->path=malloc(pathlen+1); /* one extra for the zero byte */
        if(co->path) {
          memcpy(co->path, path, pathlen);
          co->path[pathlen]=0; /* zero terminate */
        }
        else
          badcookie = TRUE;
      }
    }

    free(what);

    if(badcookie || !co->name) {
      /* we didn't get a cookie name or a bad one,
         this is an illegal line, bail out */
      freecookie(co);
      return NULL;
    }

  }
  else {
    /* This line is NOT a HTTP header style line, we do offer support for
       reading the odd netscape cookies-file format here */
    char *firstptr;
    char *tok_buf;
    int fields;

    if(lineptr[0]=='#') {
      /* don't even try the comments */
      free(co);
      return NULL;
    }
    /* strip off the possible end-of-line characters */
    ptr=strchr(lineptr, '\r');
    if(ptr)
      *ptr=0; /* clear it */
    ptr=strchr(lineptr, '\n');
    if(ptr)
      *ptr=0; /* clear it */

    firstptr=strtok_r(lineptr, "\t", &tok_buf); /* tokenize it on the TAB */

    /* Here's a quick check to eliminate normal HTTP-headers from this */
    if(!firstptr || strchr(firstptr, ':')) {
      free(co);
      return NULL;
    }

    /* Now loop through the fields and init the struct we already have
       allocated */
    for(ptr=firstptr, fields=0; ptr && !badcookie;
        ptr=strtok_r(NULL, "\t", &tok_buf), fields++) {
      switch(fields) {
      case 0:
        if(ptr[0]=='.') /* skip preceeding dots */
          ptr++;
        co->domain = strdup(ptr);
        if(!co->domain)
          badcookie = TRUE;
        break;
      case 1:
        /* This field got its explanation on the 23rd of May 2001 by
           Andr�s Garc�a:

           flag: A TRUE/FALSE value indicating if all machines within a given
           domain can access the variable. This value is set automatically by
           the browser, depending on the value you set for the domain.

           As far as I can see, it is set to true when the cookie says
           .domain.com and to false when the domain is complete www.domain.com
        */
        co->tailmatch=(bool)strequal(ptr, "TRUE"); /* store information */
        break;
      case 2:
        /* It turns out, that sometimes the file format allows the path
           field to remain not filled in, we try to detect this and work
           around it! Andr�s Garc�a made us aware of this... */
        if (strcmp("TRUE", ptr) && strcmp("FALSE", ptr)) {
          /* only if the path doesn't look like a boolean option! */
          co->path = strdup(ptr);
          if(!co->path)
            badcookie = TRUE;
          break;
        }
        /* this doesn't look like a path, make one up! */
        co->path = strdup("/");
        if(!co->path)
          badcookie = TRUE;
        fields++; /* add a field and fall down to secure */
        /* FALLTHROUGH */
      case 3:
        co->secure = (bool)strequal(ptr, "TRUE");
        break;
      case 4:
        co->expires = curlx_strtoofft(ptr, NULL, 10);
        break;
      case 5:
        co->name = strdup(ptr);
        if(!co->name)
          badcookie = TRUE;
        break;
      case 6:
        co->value = strdup(ptr);
        if(!co->value)
          badcookie = TRUE;
        break;
      }
    }
    if(6 == fields) {
      /* we got a cookie with blank contents, fix it */
      co->value = strdup("");
      if(!co->value)
        badcookie = TRUE;
      else
        fields++;
    }

    if(!badcookie && (7 != fields))
      /* we did not find the sufficient number of fields */
      badcookie = TRUE;

    if(badcookie) {
      freecookie(co);
      return NULL;
    }

  }

  if(!c->running &&    /* read from a file */
     c->newsession &&  /* clean session cookies */
     !co->expires) {   /* this is a session cookie since it doesn't expire! */
    freecookie(co);
    return NULL;
  }

  co->livecookie = c->running;

  /* now, we have parsed the incoming line, we must now check if this
     superceeds an already existing cookie, which it may if the previous have
     the same domain and path as this */

  clist = c->cookies;
  replace_old = FALSE;
  while(clist) {
    if(strequal(clist->name, co->name)) {
      /* the names are identical */

      if(clist->domain && co->domain) {
        if(strequal(clist->domain, co->domain))
          /* The domains are identical */
          replace_old=TRUE;
      }
      else if(!clist->domain && !co->domain)
        replace_old = TRUE;

      if(replace_old) {
        /* the domains were identical */

        if(clist->path && co->path) {
          if(strequal(clist->path, co->path)) {
            replace_old = TRUE;
          }
          else
            replace_old = FALSE;
        }
        else if(!clist->path && !co->path)
          replace_old = TRUE;
        else
          replace_old = FALSE;

      }

      if(replace_old && !co->livecookie && clist->livecookie) {
        /* Both cookies matched fine, except that the already present
           cookie is "live", which means it was set from a header, while
           the new one isn't "live" and thus only read from a file. We let
           live cookies stay alive */

        /* Free the newcomer and get out of here! */
        freecookie(co);
        return NULL;
      }

      if(replace_old) {
        co->next = clist->next; /* get the next-pointer first */

        /* then free all the old pointers */
        if(clist->name)
          free(clist->name);
        if(clist->value)
          free(clist->value);
        if(clist->domain)
          free(clist->domain);
        if(clist->path)
          free(clist->path);
        if(clist->expirestr)
          free(clist->expirestr);

        if(clist->version)
          free(clist->version);
        if(clist->maxage)
          free(clist->maxage);

        *clist = *co;  /* then store all the new data */

        free(co);   /* free the newly alloced memory */
        co = clist; /* point to the previous struct instead */

        /* We have replaced a cookie, now skip the rest of the list but
           make sure the 'lastc' pointer is properly set */
        do {
          lastc = clist;
          clist = clist->next;
        } while(clist);
        break;
      }
    }
    lastc = clist;
    clist = clist->next;
  }

  if(c->running)
    /* Only show this when NOT reading the cookies from a file */
    infof(data, "%s cookie %s=\"%s\" for domain %s, path %s, expire %d\n",
          replace_old?"Replaced":"Added", co->name, co->value,
          co->domain, co->path, co->expires);

  if(!replace_old) {
    /* then make the last item point on this new one */
    if(lastc)
      lastc->next = co;
    else
      c->cookies = co;
  }

  c->numcookies++; /* one more cookie in the jar */
  return co;
}

/*****************************************************************************
 *
 * Curl_cookie_init()
 *
 * Inits a cookie struct to read data from a local file. This is always
 * called before any cookies are set. File may be NULL.
 *
 * If 'newsession' is TRUE, discard all "session cookies" on read from file.
 *
 ****************************************************************************/
struct CookieInfo *Curl_cookie_init(struct SessionHandle *data,
                                    char *file,
                                    struct CookieInfo *inc,
                                    bool newsession)
{
  struct CookieInfo *c;
  FILE *fp;
  bool fromfile=TRUE;

  if(NULL == inc) {
    /* we didn't get a struct, create one */
    c = (struct CookieInfo *)calloc(1, sizeof(struct CookieInfo));
    if(!c)
      return NULL; /* failed to get memory */
    c->filename = strdup(file?file:"none"); /* copy the name just in case */
  }
  else {
    /* we got an already existing one, use that */
    c = inc;
  }
  c->running = FALSE; /* this is not running, this is init */

  if(file && strequal(file, "-")) {
    fp = stdin;
    fromfile=FALSE;
  }
  else if(file && !*file) {
    /* points to a "" string */
    fp = NULL;
  }
  else
    fp = file?fopen(file, "r"):NULL;

  c->newsession = newsession; /* new session? */

  if(fp) {
    char *lineptr;
    bool headerline;

    char *line = (char *)malloc(MAX_COOKIE_LINE);
    if(line) {
      while(fgets(line, MAX_COOKIE_LINE, fp)) {
        if(checkprefix("Set-Cookie:", line)) {
          /* This is a cookie line, get it! */
          lineptr=&line[11];
          headerline=TRUE;
        }
        else {
          lineptr=line;
          headerline=FALSE;
        }
        while(*lineptr && my_isspace(*lineptr))
          lineptr++;

        Curl_cookie_add(data, c, headerline, lineptr, NULL, NULL);
      }
      free(line); /* free the line buffer */
    }
    if(fromfile)
      fclose(fp);
  }

  c->running = TRUE;          /* now, we're running */

  return c;
}

/*****************************************************************************
 *
 * Curl_cookie_getlist()
 *
 * For a given host and path, return a linked list of cookies that the
 * client should send to the server if used now. The secure boolean informs
 * the cookie if a secure connection is achieved or not.
 *
 * It shall only return cookies that haven't expired.
 *
 ****************************************************************************/

struct Cookie *Curl_cookie_getlist(struct CookieInfo *c,
                                   char *host, char *path, bool secure)
{
  struct Cookie *newco;
  struct Cookie *co;
  time_t now = time(NULL);
  struct Cookie *mainco=NULL;

  if(!c || !c->cookies)
    return NULL; /* no cookie struct or no cookies in the struct */

  co = c->cookies;

  while(co) {
    /* only process this cookie if it is not expired or had no expire
       date AND that if the cookie requires we're secure we must only
       continue if we are! */
    if( (co->expires<=0 || (co->expires> now)) &&
        (co->secure?secure:TRUE) ) {

      /* now check if the domain is correct */
      if(!co->domain ||
         (co->tailmatch && tailmatch(co->domain, host)) ||
         (!co->tailmatch && strequal(host, co->domain)) ) {
        /* the right part of the host matches the domain stuff in the
           cookie data */

        /* now check the left part of the path with the cookies path
           requirement */
        if(!co->path ||
           /* not using checkprefix() because matching should be
              case-sensitive */
           !strncmp(co->path, path, strlen(co->path)) ) {

          /* and now, we know this is a match and we should create an
             entry for the return-linked-list */

          newco = (struct Cookie *)malloc(sizeof(struct Cookie));
          if(newco) {
            /* first, copy the whole source cookie: */
            memcpy(newco, co, sizeof(struct Cookie));

            /* then modify our next */
            newco->next = mainco;

            /* point the main to us */
            mainco = newco;
          }
          else {
            /* failure, clear up the allocated chain and return NULL */
            while(mainco) {
              co = mainco->next;
              free(mainco);
              mainco = co;
            }

            return NULL;
          }
        }
      }
    }
    co = co->next;
  }

  return mainco; /* return the new list */
}

/*****************************************************************************
 *
 * Curl_cookie_clearall()
 *
 * Clear all existing cookies and reset the counter.
 *
 ****************************************************************************/
void Curl_cookie_clearall(struct CookieInfo *cookies)
{
  if(cookies) {
    Curl_cookie_freelist(cookies->cookies);
    cookies->cookies = NULL;
    cookies->numcookies = 0;
  }
}

/*****************************************************************************
 *
 * Curl_cookie_freelist()
 *
 * Free a list of cookies previously returned by Curl_cookie_getlist();
 *
 ****************************************************************************/

void Curl_cookie_freelist(struct Cookie *co)
{
  struct Cookie *next;
  if(co) {
    while(co) {
      next = co->next;
      free(co); /* we only free the struct since the "members" are all
                      just copied! */
      co = next;
    }
  }
}


/*****************************************************************************
 *
 * Curl_cookie_clearsess()
 *
 * Free all session cookies in the cookies list.
 *
 ****************************************************************************/
void Curl_cookie_clearsess(struct CookieInfo *cookies)
{
  struct Cookie *first, *curr, *next, *prev = NULL;

  if(!cookies->cookies)
    return;

  first = curr = prev = cookies->cookies;

  for(; curr; curr = next) {
    next = curr->next;
    if(!curr->expires) {
      if(first == curr)
        first = next;

      if(prev == curr)
        prev = next;
      else
        prev->next = next;

      free(curr);
      cookies->numcookies--;
    }
    else
      prev = curr;
  }

  cookies->cookies = first;
}


/*****************************************************************************
 *
 * Curl_cookie_cleanup()
 *
 * Free a "cookie object" previous created with cookie_init().
 *
 ****************************************************************************/
void Curl_cookie_cleanup(struct CookieInfo *c)
{
  struct Cookie *co;
  struct Cookie *next;
  if(c) {
    if(c->filename)
      free(c->filename);
    co = c->cookies;

    while(co) {
      next = co->next;
      freecookie(co);
      co = next;
    }
    free(c); /* free the base struct as well */
  }
}

/* get_netscape_format()
 *
 * Formats a string for Netscape output file, w/o a newline at the end.
 *
 * Function returns a char * to a formatted line. Has to be free()d
*/
static char *get_netscape_format(const struct Cookie *co)
{
  return aprintf(
    "%s%s\t" /* domain */
    "%s\t"   /* tailmatch */
    "%s\t"   /* path */
    "%s\t"   /* secure */
    "%" FORMAT_OFF_T "\t"   /* expires */
    "%s\t"   /* name */
    "%s",    /* value */
    /* Make sure all domains are prefixed with a dot if they allow
       tailmatching. This is Mozilla-style. */
    (co->tailmatch && co->domain && co->domain[0] != '.')? ".":"",
    co->domain?co->domain:"unknown",
    co->tailmatch?"TRUE":"FALSE",
    co->path?co->path:"/",
    co->secure?"TRUE":"FALSE",
    co->expires,
    co->name,
    co->value?co->value:"");
}

/*
 * Curl_cookie_output()
 *
 * Writes all internally known cookies to the specified file. Specify
 * "-" as file name to write to stdout.
 *
 * The function returns non-zero on write failure.
 */
int Curl_cookie_output(struct CookieInfo *c, char *dumphere)
{
  struct Cookie *co;
  FILE *out;
  bool use_stdout=FALSE;

  if((NULL == c) || (0 == c->numcookies))
    /* If there are no known cookies, we don't write or even create any
       destination file */
    return 0;

  if(strequal("-", dumphere)) {
    /* use stdout */
    out = stdout;
    use_stdout=TRUE;
  }
  else {
    out = fopen(dumphere, "w");
    if(!out)
      return 1; /* failure */
  }

  if(c) {
    char *format_ptr;

    fputs("# Netscape HTTP Cookie File\n"
          "# http://curlm.haxx.se/rfc/cookie_spec.html\n"
          "# This file was generated by libcurl! Edit at your own risk.\n\n",
          out);
    co = c->cookies;

    while(co) {
      format_ptr = get_netscape_format(co);
      if (format_ptr == NULL) {
        fprintf(out, "#\n# Fatal libcurl error\n");
        if(!use_stdout)
          fclose(out);
        return 1;
      }
      fprintf(out, "%s\n", format_ptr);
      free(format_ptr);
      co=co->next;
    }
  }

  if(!use_stdout)
    fclose(out);

  return 0;
}

struct curl_slist *Curl_cookie_list(struct SessionHandle *data)
{
  struct curl_slist *list = NULL;
  struct curl_slist *beg;
  struct Cookie *c;
  char *line;

  if ((data->cookies == NULL) ||
      (data->cookies->numcookies == 0))
    return NULL;

  c = data->cookies->cookies;

  beg = list;
  while (c) {
    /* fill the list with _all_ the cookies we know */
    line = get_netscape_format(c);
    if (line == NULL) {
      /* get_netscape_format returns null only if we run out of memory */

      curl_slist_free_all(beg); /* free some memory */
      return NULL;
    }
    list = curl_slist_append(list, line);
    free(line);
    c = c->next;
  }

  return list;
}

#endif /* CURL_DISABLE_HTTP || CURL_DISABLE_COOKIES */
