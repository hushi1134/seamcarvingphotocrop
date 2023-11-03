#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "c_img.h"


void calc_energy(struct rgb_img *im, struct rgb_img **grad){
  create_img(grad, im->height, im->width);
  int rx, gx, bx, ry, by, gy, energy, dgenergy;
  
  
  for (int i = 0; i < im->height; i++){
    for(int j = 0; j < im->width; j++){

      if (i ==0){
      ry = (int) get_pixel(im, i+1, j, 0) - (int) get_pixel(im, im->height -1, j, 0);
      gy = (int) get_pixel(im, i+1, j, 1) - (int) get_pixel(im, im->height -1, j, 1);
      by = (int) get_pixel(im, i+1, j, 2) - (int) get_pixel(im, im->height -1, j, 2);
      }else if (i +1 == im->height){
        ry = (int) get_pixel(im, 0, j, 0) - (int) get_pixel(im, i-1, j, 0);
      gy = (int) get_pixel(im, 0, j, 1) - (int) get_pixel(im, i-1, j, 1);
      by = (int) get_pixel(im, 0, j, 2) - (int) get_pixel(im, i-1, j, 2);
      }else{
        ry = (int) get_pixel(im, i+1, j, 0) - (int) get_pixel(im, i-1, j, 0);
      gy = (int) get_pixel(im, i+1, j, 1) - (int) get_pixel(im, i-1, j, 1);
      by = (int) get_pixel(im, i+1, j, 2) - (int) get_pixel(im, i-1, j, 2);
      }

    if(j == 0){
      rx = (int) get_pixel(im, i, j+1, 0) - (int) get_pixel(im, i, im->width-1, 0);
      gx = (int) get_pixel(im, i,j+1, 1) - (int) get_pixel(im, i, im->width-1, 1);
      bx = (int) get_pixel(im, i,j+1, 2) - (int) get_pixel(im, i, im->width-1, 2);
    }else if (j +1 == im -> width){
      rx = (int) get_pixel(im, i, 0, 0) - (int) get_pixel(im, i, j-1, 0);
      gx = (int) get_pixel(im, i,0, 1) - (int) get_pixel(im, i, j-1, 1);
      bx = (int) get_pixel(im, i,0, 2) - (int) get_pixel(im, i, j-1, 2);
    }else{
      rx = (int) get_pixel(im, i, j+1, 0) - (int) get_pixel(im, i, j-1, 0);
      gx = (int) get_pixel(im, i,j+1, 1) - (int) get_pixel(im, i, j-1, 1);
      bx = (int) get_pixel(im, i,j+1, 2) - (int) get_pixel(im, i, j-1, 2);
    }
      
      
      energy = sqrt(rx*rx + gx*gx+bx*bx + ry*ry + gy*gy+by*by);
      dgenergy = (uint8_t)(energy/10);

      set_pixel(*grad, i, j, dgenergy, dgenergy, dgenergy);


        }
  }
};

void dynamic_seam(struct rgb_img *grad, double **best_arr){
  *best_arr = (double *)malloc(grad->height*grad->width*sizeof(double));
  for (int i = 0; i < grad->height; i++){
    for(int j = 0; j < grad->width; j++){
      double a,b,c;
      if (i == 0){
        (*best_arr)[j] = (double)get_pixel(grad, i, j, 0);
      }else if(j == 0){
        b = (*best_arr)[(i-1)*grad->width+j];
        c = (*best_arr)[(i-1)*grad->width+j+1];
        if (c<b){
          b = c;
        }
        (*best_arr)[i*grad->width+j] = b + (double)get_pixel(grad, i, j, 0);
      }
      else if(j == grad->width-1){
        b = (*best_arr)[(i-1)*grad->width+j];
        c = (*best_arr)[(i-1)*grad->width+j-1];
        if (c<b){
          b = c;
        }
        (*best_arr)[i*grad->width+j] = b + (double)get_pixel(grad, i, j, 0);
      

    }else{
        a = (*best_arr)[(i-1)*grad->width+j-1];
        b = (*best_arr)[(i-1)*grad->width+j];
        c = (*best_arr)[(i-1)*grad->width+j+1];
        if (c<b){
          b = c;
        } if (a<b){
          b = a;
        }
        (*best_arr)[i*grad->width+j] = b + (double)get_pixel(grad, i, j, 0);
      
    }
    }
}
}


void recover_path(double *best, int height, int width, int **path){
  (*path) = (int *)malloc(height*sizeof(int));
  double a;
  double min = best[(height-1)*width];
  int j1 = 0;
  for(int j = 1; j < width; j++){
    a = best[(height-1)*width+j]; 
    if (a < min){
      min = a; 
      j1 = j;
  }
  }
  (*path)[height-1] = j1;
  double b, c;
  
  for(int i = height-2; i>=0; i--){
    if (j1 - 1 <0){
      a = best[i*width+j1];
    } else {
    a = best[i*width+j1-1];
    }
    b = best[i*width+j1];
    if (j1 + 1 == width){
      c = best[i*width+j1];
    }else{
    c = best[i*width+j1+1];
    }
    int j2 = j1;
    if (c<b){
      b = c;
      j2 = j1+1;
    } if (a<b){
      b = a;
      j2 = j1-1;
    }
    j1 = j2;
    (*path)[i] = j1;
  }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
  create_img(dest, src->height, src->width -1);
  int r,g,b;
  for (int i = 0; i < src->height; i++){
    for(int j = 0; j < src->width; j++){
      r = get_pixel(src, i, j, 0);
      g = get_pixel(src, i, j, 1);
      b = get_pixel(src, i, j, 2);
      if( j < path[i]){
        set_pixel(*dest, i, j, r, g, b);
      } else if (j > path[i]){
        set_pixel(*dest, i, j-1, r, g, b);
      }
}
}
}








