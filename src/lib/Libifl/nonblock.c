/*
 * Defns of nonblocking read,write.
 * Headers redefine read/write to name these instead, before inclusion
 * of stdio.h, so system declaration is used.
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <pbs_config.h>

/* LF: undefine write() to prevent loop-forever issue as write is redefined in pbs_config.h */

#undef write


/* LF: undefine write() to prevent loop-forever issue as write is redefined in pbs_config.h */
#undef read

ssize_t write_nonblocking_socket(

  int     fd,
  void   *buf,
  ssize_t count)

  {
  ssize_t i;
  time_t  start, now;
  int errval; 

  /* FIXME: LF: this was hardcoded here for a long time, */
  /*         shouldn't it be tcp_timeout from server/mom configuration? */
  int timeout = 30;

  /* NOTE:  under some circumstances, a blocking fd will be passed */

  #ifdef HAVE_SO_SNDTIMEO
  /* LF: if blocking fd is passed - in some cases it may cause server to hang for a long time.
     to fix this we introduce socket timeout */

  struct timeval send_timeout, send_timeout_orig;

  int tv_size; 
  int tm_set = 0; // timeout modification flag; false by default


  send_timeout.tv_sec = timeout;
  send_timeout.tv_usec = 0;

  /* try to store socket timeout value and set new timeout if possible */

  tv_size = sizeof(send_timeout_orig); // must contain proper size for getsockopt to success

  if (getsockopt(fd,SOL_SOCKET, SO_SNDTIMEO,(void *) &send_timeout_orig, (socklen_t *) &tv_size) == 0)
  {
     if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void *) &send_timeout, sizeof(send_timeout)) == 0)
       tm_set = 1; // socket timeout modified
  }
  #endif

  /* Set a timer to prevent an infinite loop here. */
  time(&now);
  start = now;

  for (;;)
    {
    i = write(fd, buf, count);
    /* save errno for write */
    errval = errno;

    if (i >= 0)
      {
      /* successfully wrote 'i' bytes */
      break;
      }

    if (errno != EAGAIN)
      {
      /* write failed */
      break;
      }


    time(&now);
    if ((now - start) >= timeout)
      {
      /* timed out */
      break;
      }
    }    /* END for () */


  /* cleaup and return */
  #ifdef HAVE_SO_SNDTIMEO
  /* restore send timeout */
    if(tm_set)
      setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (void *) &send_timeout_orig, sizeof(send_timeout_orig));
  #endif

  /* set errno properly and return i from write() */
  errno = errval;
  return(i);

  /*NOTREACHED*/

  return(0);
  }  /* END write_nonblocking_socket() */



ssize_t read_nonblocking_socket(

  int     fd,
  void   *buf,
  ssize_t count)

  {
  int     flags;
  ssize_t i;
  time_t  start, now;

  /* verify socket is non-blocking */

  /* NOTE:  under some circumstances, a blocking fd will be passed */

  if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
    return(-1);
    }

#if defined(FNDELAY) && !defined(__hpux)
  if (flags & FNDELAY)
#else
  if (flags & O_NONBLOCK)
#endif
    {
    /* flag already set */

    /* NO-OP */
    }
  else
    {
    /* set no delay */

#if defined(FNDELAY) && !defined(__hpux)
    flags |= FNDELAY;
#else
    flags |= O_NONBLOCK;
#endif

    /* NOTE:  the pbs scheduling API passes in a blocking socket which
              should be a non-blocking socket in pbs_disconnect.  Also,
              qsub passes in a blocking socket which must remain
              non-blocking */

    /* the below non-blocking socket flag check should be rolled into
       pbs_disconnect and removed from here (NYI) */

    /*
    if (fcntl(fd,F_SETFL,flags) == -1)
      {
      return(-1);
      }
    */
    }    /* END else (flags & BLOCK) */

  /* Set a timer to prevent an infinite loop here. */

  start = -1;

  for (;;)
    {
    i = read(fd, buf, count);

    if (i >= 0)
      {
      return(i);
      }

    if (errno != EAGAIN)
      {
      return(i);
      }

    time(&now);

    if (start == -1)
      {
      start = now;
      }
    else if ((now - start) > 30)
      {
      return(i);
      }
    }    /* END for () */

  /*NOTREACHED*/

  return(0);
  }  /* END read_nonblocking_socket() */





/*
 * Call the real read, for things that want to block.
 */

ssize_t read_blocking_socket(

  int      fd,
  void    *buf,
  ssize_t  count)

  {
  return(read(fd, buf, count));
  }

