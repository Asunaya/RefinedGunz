/* file _zdc.c; copyright Dimitre Trendafilov (2003)
 * command line frontend for computing zdelta difference for files up to 1MB
 * takes from 1 to 6 command line parameters
 * argv[1] - reference file 
 * argv[2] - target file; optional - if not specified read from stdin
 * argv[3] - delta file; optional - if not specified write to stdout
 * if K>3 arguments, the first K-2 are considered reference files
 */

#include <stdio.h>
#include "zd_mem.h"     /* dynamic memory allocation */
#include "zdlib.h"      /* zdelta interface          */

static const char usage_single[] = "usage: _zdc Reference [Target [Delta]]";
static const char usage_multi[]  = "       _zdc Reference1 Reference2";
static const char usage_multi_end[]  = "Target Delta";

#define BUF_SIZE  1024*1024

#if REFNUM > 1
#define MainMulti(argc,argv) main_multi(argc,argv)
#else
#define MainMulti(argc, argv) 0
#endif

/*===========================================================================
 *  multiple refernce files
 */

#if REFNUM > 1
int main_multi(int argc, char *argv[]){
  Bytef        *tar_buf;                 /* target memory buffer     */
  FILE         *fp_ref[REFNUM];          /* reference file handles   */
  Bytef        *ref_buf[REFNUM];         /* reference target buffers */
  uLong         ref_s[REFNUM], tar_s;
  int           ref_n, i;
  int           input=2, output=3;

  Bytef         *delta = NULL;
  uLong         d_size = 0;

  ref_n  = argc - 3; /* get the number of reference files */
  input  = argc - 2; /* get the input argument index      */
  output = argc - 1; /* get the output argument index     */

  /* open and check the input files */
  for(i=0;i<ref_n;++i){
    if( !(fp_ref[i] = fopen(argv[i+1],"rb")) ){ perror(argv[i+1]); exit(0); }
  }
  if(!freopen(argv[input],"rb",stdin)){ perror(argv[input]); exit(0); }

  /* copy input data to memory */
  for(i=0;i<ref_n;++i){
    ref_buf[i] = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
    if(ref_buf[i] == NULL) 
    {
      fprintf(stderr,"%s\n","insufficient memory"); 
      exit(0);
    }
    ref_s[i] = fread((void *) ref_buf[i], sizeof(Byte), 
		     (size_t) BUF_SIZE, fp_ref[i]);
  }
  tar_buf = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
  if(tar_buf == NULL) 
  {
    fprintf(stderr, "%s\n", "insufficient memory"); 
    exit(0);
  }
  tar_s = fread((void *) tar_buf, sizeof(Byte), (size_t) BUF_SIZE, stdin);

  delta = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
  if(delta == NULL) 
  {
    fprintf(stderr, "%s\n", "insufficient memory"); 
    exit(0);
  }
  d_size = BUF_SIZE;

  /* compress the data */
  if( zd_compressN(ref_buf,ref_s,ref_n,tar_buf,tar_s,delta,&d_size) == ZD_OK){

    /* successfull compression write the delta to a file */
    if(!freopen(argv[output],"wb",stdout)){
      perror(argv[4]);
      exit(0);
    }

    if(d_size !=  fwrite(delta,sizeof(char),d_size ,stdout)) perror("ouput");
  }

  /* release memory */
  free(delta);
  free(&tar_buf);
  for(i=ref_n-1;i>=0;--i){
    free(&ref_buf[i]); /* free  ref. buffers */
    fclose(fp_ref[i]); /* close ref files    */ 
  }

  return 0;
}
#endif /* REFNUM */

/*===========================================================================
 *  single reference file
 */

int main_single(int argc, char *argv[]){
  FILE *fp_ref;
  Bytef *tar_buf;
  Bytef *ref_buf;
  Bytef *delta = NULL;
  uLong d_size = 0, tar_s, ref_s;

  /* open and check the input files */
  if( !(fp_ref = fopen(argv[1],"rb")) )
  {
    perror(argv[1]);
    exit(0);
  }

  if(argc>2 && (!freopen(argv[2],"rb",stdin)))
  {
    perror(argv[2]); 
    exit(0);
  }

  /* copy input data to memory */
  ref_buf = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
  if(ref_buf == NULL) 
  {
    fprintf(stderr,"%s\n","insufficient memory"); 
    exit(0);
  }
  ref_s = fread((void *) ref_buf, sizeof(Byte), (size_t) BUF_SIZE, fp_ref);

  tar_buf = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
  if(tar_buf == NULL) 
  {
    fprintf(stderr, "%s\n", "insufficient memory"); 
    exit(0);
  }
  tar_s = fread((void *) tar_buf, sizeof(Byte), (size_t) BUF_SIZE, stdin);

  delta = (Bytef*) malloc(BUF_SIZE*sizeof(Byte));
  if(delta == NULL) 
  {
    fprintf(stderr, "%s\n", "insufficient memory"); 
    exit(0);
  }

  d_size = BUF_SIZE;

  /* compress the data */
  if(zd_compress(ref_buf, ref_s, tar_buf, tar_s, delta, &d_size) == ZD_OK){

    /* successfull compression write the delta to a file */
    if(argc>3 && (!freopen(argv[3],"wb",stdout))){
      perror(argv[3]);
      exit(0);
    }
    if(d_size != fwrite(delta, sizeof(char), d_size, stdout)) perror("ouput");
  }
  
  /* release memory */
  free(delta);
  free(tar_buf);
  free(ref_buf);

  /* close files */
  fclose(fp_ref);

  return 0;
}

/*===========================================================================
 *  main function
 */

int main(int argc, char *argv[]){
  int i;

  if(argc<2 || argc>(REFNUM+3))   /* check command line parameters */
  {
    fprintf(stderr,"%s",usage_single);
    if(REFNUM>1){
      fprintf(stderr," |\n%s",usage_multi);
      for(i=3;i<=REFNUM;++i) fprintf (stderr," %s%d", "[Reference",i);
      for(i=3;i<=REFNUM;++i) fprintf (stderr,"%s", "]");
      fprintf(stderr," %s\n",usage_multi_end);
    }
    else{
      fprintf(stderr,"\n");
    }
    exit(0);
  }

  if(argc <= 4){
    return main_single(argc,argv);
  }
  else{
    return MainMulti(argc, argv);
  }
}

