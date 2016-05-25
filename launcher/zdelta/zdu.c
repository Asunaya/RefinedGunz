/* file zdu.c; copyright Dimitre Trendafilov (2003)
 * command line frontend for decompressing zdelta difference
 * takes from 1 to 3 command line parameters
 * argv[1] - reference file 
 * argv[2] - delta file; optional - if not specified read from stdin
 * argv[3] - target file; optional - if not specified write to stdout
 * if K>3 arguments, the first K-2 are considered reference files
 */

#include <stdio.h>

#include "zd_mem.h"     /* provides dynamic memory allocation */
#include "zdlib.h" 

static const char usage_single[] = "usage: zdu Reference [Target [Delta]]";
static const char usage_multi[]  = "       zdu Reference1 Reference2";
static const char usage_multi_end[]  = "Target Delta";

#if REFNUM > 1
#define MainMulti(argc,argv) main_multi(argc,argv)
#else
#define MainMulti(argc, argv) 0
#endif

/*===========================================================================
 *  multiple reference files
 */
#if REFNUM>1
int main_multi(int argc, char *argv[]){

  FILE *fp_ref[REFNUM];
  zd_mem_buffer ref_buf[REFNUM];
  Bytef *ref[4];
  uLong ref_s[4];
  int   ref_n,i;

  zd_mem_buffer delta;
  Bytef *tar_buf = NULL;
  uLong t_size = 0;
  int   input, output;

  ref_n  = argc - 3; /* get the number of reference files */
  input  = argc - 2; /* get the input argument index */
  output = argc - 1; /* get the output argument index */

  /* open and check the input files */
  for(i=0;i<ref_n;++i){
    if( !(fp_ref[i] = fopen(argv[i+1],"rb")) ) { perror(argv[i+1]); exit(0); }
  }
  if(!freopen(argv[input],"rb",stdin)) { perror(argv[input]); exit(0); }

  /* copy input data to memory */
  for(i=0;i<ref_n;++i){
    if(dread_file(fp_ref[i],&ref_buf[i]) < 0) {perror(argv[i]); exit(0);}
    ref[i] = ref_buf[i].buffer; 
    ref_s[i] = ref_buf[i].pos - ref_buf[i].buffer; 
  }
  if(dread_file(stdin,&delta) < 0) perror(argv[input]);

  /* decompress the data */
  if(zd_uncompressN1((const Bytef**)ref, ref_s, ref_n,
		    &tar_buf, &t_size,
		    (const Bytef*) delta.buffer, delta.pos - delta.buffer) == ZD_OK){
    
    /* successfull compression write the delta to a file */
    if(argc>3 && (!freopen(argv[output],"wb",stdout))){
      perror(argv[output]);
      exit(0);
    }
    
    if(t_size !=  fwrite(tar_buf, sizeof(char),t_size ,stdout))
      perror("ouput");

    free(tar_buf);
  }
  
  /* release memory */
  zd_free(&delta);
  for(i=ref_n-1;i>=0;--i){
    zd_free(&ref_buf[i]); /* free  ref. buffers */
    fclose(fp_ref[i]);   /* close ref files */ 
  }

  return 0;
}
#endif /* REFNUM>1 */

/*===========================================================================
 *  single reference files
 */
int main_single(int argc, char *argv[]){
  FILE *fp_ref;

  zd_mem_buffer ref_buf;
  zd_mem_buffer delta;
  Bytef *tar_buf = NULL;
  uLong t_size = 0;

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
  if(dread_file(fp_ref,&ref_buf) < 0) {perror(argv[1]); exit(0);}
  if(dread_file(stdin,&delta) < 0) {perror(argv[2]); exit(0);}

  /* decompress the data */
  if(zd_uncompress1((const Bytef*)ref_buf.buffer, ref_buf.pos - ref_buf.buffer,
		    &tar_buf, &t_size,
		    (const Bytef*) delta.buffer, 
		    delta.pos - delta.buffer) == ZD_OK){
    
    /* successfull compression write the delta to a file */
    if(argc>3 && (!freopen(argv[3],"wb",stdout))){
      perror(argv[3]);
      exit(0);
    }
    if(t_size !=  fwrite(tar_buf, sizeof(char),t_size ,stdout))
      perror("ouput");
  }
  
  /* release memory */
  free(tar_buf);
  zd_free(&delta);
  zd_free(&ref_buf);
  
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


