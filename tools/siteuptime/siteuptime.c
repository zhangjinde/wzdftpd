#ifdef __CYGWIN__
#include <w32api/windows.h>
#else /* __CYGWIN__ */
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
#endif /* __CYGWIN__ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wzd.h>
/*#include <wzd_shm.h>*/

#define	SHM_KEY	0x1331c0d3

unsigned long key;


char *time_to_str(time_t time)
{ /* This support functionw as written by George Shearer (Dr_Delete) */

  static char workstr[100];
  unsigned short int days=(time/86400),hours,mins,secs;
  hours=((time-(days*86400))/3600);
  mins=((time-(days*86400)-(hours*3600))/60);
  secs=(time-(days*86400)-(hours*3600)-(mins*60));

  workstr[0]=(char)0;
  if(days)
    sprintf(workstr,"%dd",days);
  if(hours)
    sprintf(workstr,"%s%s%dh",workstr,(workstr[0])?", ":"",hours);
  if(mins)
    sprintf(workstr,"%s%s%dm",workstr,(workstr[0])?", ":"",mins);
  if(secs)
    sprintf(workstr,"%s%s%ds",workstr,(workstr[0])?", ":"",secs);
  if (!days && !hours && !mins && !secs)
    sprintf(workstr,"0 seconds");

  return(workstr);
}

void usage(const char *progname)
{
  fprintf(stderr,"Usage: %s [-k shm_key]\r\n",progname);
}

int parse_args(int argc, char **argv)
{
  int opt;
  unsigned long l;
  char *ptr;

   /* please keep options ordered ! */
  while ((opt=getopt(argc, argv, "hk:")) != -1) {
    switch (opt) {
    case 'h':
      usage(argv[0]);
      return 1;
    case 'k':
      l = strtoul(optarg,&ptr,0);
      if (*ptr != '\0') {
	usage(argv[0]);
	return 1;
      }
      key = l;
      break;
    }
  }
  
  return 0;
}

int main(int argc, char *argv[])
{
  int shmid;
  wzd_config_t * config;
  time_t t;
#ifdef __CYGWIN__
  void * handle;
  char name[256];
#endif

  /* default values */
  key = SHM_KEY;

  if (parse_args(argc,argv)) {
    usage(argv[0]);
    exit(1);
  }

#ifdef __CYGWIN__
  sprintf(name,"%lu",key-1);
  handle = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,name);
  if (handle == NULL)
#else
  shmid = shmget(key-1,0,0400);
  if (shmid == -1)
#endif
  {
    fprintf(stderr,"shmget failed\n");
    fprintf(stderr,"This is probably due to\n");
    fprintf(stderr,"\t* server not started\n");
    fprintf(stderr,"\t* wrong key\n");
    return -1;
  }

#ifdef __CYGWIN__
  config = MapViewOfFile(handle,FILE_MAP_ALL_ACCESS,0, 0, 0);
  if (config == NULL)
#else
  config = shmat(shmid,NULL,SHM_RDONLY);
  if (config == (void*)-1)
#endif
  {
    fprintf(stderr,"shmat failed\n");
    return -1;
  }

  time(&t);
  t = t - config->server_start;
  printf("Uptime: %s\n",time_to_str(t));

#ifdef  __CYGWIN__
  CloseHandle(handle);
#else
  shmdt(config);
#endif
  return 0;
}