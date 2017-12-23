  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <omp.h>

  #include "graphio.h"
  #include "graph.h"

  char gfile[2048];

  void usage(){
    printf("./coloring <filename>\n");
    exit(0);
  }




  int * color;
  int * vertices;
  int * temp_array;
  int boolean = 1;
  int THREAD_NUMBER = 1;
  int coloring(etype * row_ptr, vtype * col_ind, int nov)
  {
    double start, end, diff;
    int i, j, x, y;
    int current_vertice, current_color, neighbour, neighbour_color;
    int vertices_size = nov;
    int d2_color;
    int vertice_color;
    while (boolean)
    {
      #pragma omp parallel for private(current_vertice, current_color, neighbour, neighbour_color) shared(color) num_threads(THREAD_NUMBER) schedule(dynamic, 1024)  // Coloring part
      for (i = 0; i < vertices_size; i++)
      {
        current_vertice = vertices[i];
        vertice_color = color[current_vertice];
        current_color = 0;
        int d2_neigh;

        for(j = row_ptr[current_vertice]; j < row_ptr[current_vertice+1]; j++)
        {
          neighbour = col_ind[j];
          for(int x = row_ptr[neighbour]; x < row_ptr[neighbour+1]; x++)
          {
            d2_neigh = col_ind[x];
            d2_color = color[d2_neigh];
            if(d2_color == current_color)
            {
              current_color++;
              j = row_ptr[current_vertice]-1;
              break;
            }
          }
        }
        color[current_vertice] = current_color;

      }

      memset(temp_array, 0, nov*sizeof(int));
      int current_index = 0;

      #pragma omp parallel for reduction(+:current_index) shared(color) num_threads(THREAD_NUMBER) schedule(dynamic, 1024) // ERROR CATCH PART
      for (y = 0; y < vertices_size ; y++)
      {
        int current_vertice = vertices[y];
        int current_color = color[current_vertice];
        int d2_neigh;
        for(j = row_ptr[current_vertice]; j < row_ptr[current_vertice+1]; j++)
        {
          int neighbour = col_ind[j];
          for(int x = row_ptr[neighbour]; x < row_ptr[neighbour+1]; x++)
          {
            d2_neigh = col_ind[x];
            d2_color = color[d2_neigh];
            if(d2_color == color[current_vertice] && d2_neigh != current_vertice)
            {
              if(current_vertice > d2_neigh)
              {
                  temp_array[current_vertice] = 1;
              }
              else
              {
                  temp_array[d2_neigh] = 1;
              }
              //printf("vertex %d conflicted with vertex %d\n", d2_neigh, current_vertice);
            }
          }
        }

        }

      free(vertices);
      int k;
      vertices_size = 0;
      for(k = 0; k < nov; k++)
      {
        if(temp_array[k] == 1)
        {
          vertices_size++;
        }
      }


      vertices = malloc((vertices_size)*sizeof(int)); // Creating a new vertices array of smaller size here
      current_index = 0;
      if(vertices_size == 0)
      {
        boolean = 0;
      }
      else
      {
        for(k = 0; k < nov; k++)
        {
          if(temp_array[k] == 1)
          {
            vertices[current_index] = k;
            current_index++;
          }
        }
        if(current_index < 100000)
        {
          THREAD_NUMBER = 1;
        }

      }

    }
      int max = 0;
      for (int z = 0; z < nov; z++)
      {
        if(color[z] > max)
        {
          max = color[z];
        }
      }
      return max+1;
  }

  int main(int argc, char *argv[]) {
    etype *row_ptr;
    vtype *col_ind;
    ewtype *ewghts;
    vwtype *vwghts;
    vtype nov;
    if(argc != 2)
      usage();

    const char* fname = argv[1];
    strcpy(gfile, fname);
    if(read_graph(gfile, &row_ptr, &col_ind, &ewghts, &vwghts, &nov, 0) == -1) {
      printf("error in graph read\n");
      exit(1);
    }



    color = malloc(nov * sizeof(int));

    for(int i = 0; i < nov; i++)
      {
        color[i] = 0;
      }

    vertices = malloc(nov * sizeof(int));

    for (int y = 0; y < nov; y++)
    {
      vertices[y] = y;
    }
    temp_array = malloc(nov*sizeof(int));
    double start_time = omp_get_wtime();
    int num_colors = coloring(row_ptr, col_ind, nov);
    double end_time = omp_get_wtime();
    double timetotal = end_time - start_time;
    printf("Number of colors is: %d\n", num_colors);
    printf("Total time spent is: %g\n", timetotal);
    // for(int i = 0; i < nov; i++)
    // {
    //   printf("Color of vertice: %d is %d\n", i, color[i]);
    // }
    free(row_ptr);
    free(col_ind);

    return 1;
  }
