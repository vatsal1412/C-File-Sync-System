#include <stdio.h>
#include <string.h>
// Add your system includes here.
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libgen.h>
#include "hash.h"

char* x_name(const char *path)
{

  int i = strlen(path) - 1;
  char *fname = malloc(sizeof(path) + sizeof(char));
  if (strlen(path) == 1)
  {
    strcpy(fname, path);
  }
  else
  {
    while (path[i] != '/' || path[i] == 0)
    {
      strcpy(fname, &path[i]);
      i--;
    }
  }
  return fname;
}

void copy(const char* src, const char* des)
{
  FILE *fpr;
  FILE *fpw;
  char buf[100];
  size_t bytes;

  if ((fpr = fopen(src,"r")) == NULL)
  {
    perror("error reading file");
  }
  if ((fpw = fopen(des,"w")) == NULL)
  {
    perror("error writing file");
  }

  while((bytes = fread(buf,1,100,fpr))!= 0)
  {
    fwrite(buf, 1, bytes, fpw);
  }
  fclose(fpr);
  fclose(fpw);
}

const char*gen_path( const char* path, const char* sub_dir)
{
  char* buf = malloc(sizeof(char*)*strlen(path));
  strcpy(buf,path);
  strcat(buf, "/");
  strcat(buf, sub_dir);
  const char* full_path = (const char*)buf;
  return full_path;
}

int copy_ftree(const char *src, const char *des)
{
  //if the src path is relative
  if(strstr(src,"/") == NULL)
  {
    //make the path absolute
      src = gen_path("./",src);
  }
  //if the des path is relative
  if(strstr(des,"/") == NULL)
  {
    //make the path absolute
      des = gen_path("./",des);
  }
  int num_proc = 0;
  //struct to hold information about source and destination file
  struct stat src_inf;
  struct stat des_inf;
  //get the information by calling lstat on both source and destination
  lstat(src, &src_inf);
  lstat(des, &des_inf);

  if (S_ISDIR(des_inf.st_mode))
  {
    //opent the dest
    // BASE CASE if the file you are copying is Regular File
    if (S_ISREG(src_inf.st_mode))
    {
      // there is a file by the same name
      if (access(gen_path(des,x_name(src)), F_OK) == 0)
      {
        //if the sizes are the same
        if(src_inf.st_size == des_inf.st_size )
        {
          // if the hash are diffrent
          FILE *src_ptr;
          FILE *des_ptr;
          src_ptr = fopen(src, "r");
          des_ptr = fopen(des, "r");
          //if the hash is diffrent
          if (hash(src_ptr) != hash(des_ptr)) {
            //copy
            copy(src,gen_path(des,x_name(src)));
          }
          else
          {
            // if the permissions of the src or des are diffrent
            if ((src_inf.st_mode & 0777) != (des_inf.st_mode & 0777))
            {
              //update the destination's permissions to src's permissions
              chmod(gen_path(des,x_name(src)),src_inf.st_mode);
            }
          }
          fclose(des_ptr);
          fclose(src_ptr);
        }

        //else copy normally
        else
        {
          copy(src,gen_path(des,x_name(src)));
        }
      }
      //else copy normally
      else
      {
         copy(src,gen_path(des,x_name(src)));
      }
    }

    else if (S_ISDIR(src_inf.st_mode))
    {
      //check if the directory already exists in the destination
      if ((access(gen_path(des,x_name(src)), F_OK) != 0))
      {
        mkdir(gen_path(des,x_name(src)),src_inf.st_mode);
      }
      //open and read the
      DIR *dir;
      struct dirent *inf;
      dir = opendir(src);
      // loop through the whole directory
      while ((inf = readdir(dir)) != NULL)
      {
        //if the files are not it self or its parent
        if ((inf->d_name)[0] != '.')
        {
          //struct to hold information about content files
          struct stat con_inf;
          // get the information about the content file
          lstat(gen_path(src,inf->d_name), &con_inf);
          // if that file is a regular file
          if (S_ISREG(con_inf.st_mode))
          {
            // recursively call copy_ftree on that file to the destination
            copy_ftree(gen_path(src,inf->d_name),gen_path(des,x_name(src)));
          }
          // else if the file is a directory
          else if (S_ISDIR(con_inf.st_mode))
          {
            //fork and recursively call copy_ftree on the sub dir
            int pid = fork();
            if ( pid == 0 )
            {
              num_proc += copy_ftree(gen_path(src,inf->d_name),gen_path(des,x_name(src)));
              exit(1+num_proc);
            }
            else
            {
              int status;
              if(wait(&status) != -1)
              {
                  if (WIFEXITED(status))
                  {
                    num_proc += WEXITSTATUS(status);
                  }
              }
            }
          }
        }
      }
    }
    else
    {
      exit(-1);
    }
  }
return num_proc++;
}
