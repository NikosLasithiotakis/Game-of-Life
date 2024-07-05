#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

char** world;
char** next_generation;
int rows, columns;
int num_threads;

void read_file(char* input_filename){
    int i, j;
    char cell;
    int column_counter = 0;
    FILE* input_file = fopen(input_filename, "r");
    if(input_file == NULL){
        printf("Error: Failed to open input file.\n");
        exit(1);
    }
    fscanf(input_file, "%d %d", &rows, &columns);
    world = (char**)malloc(rows * sizeof(char*));
    next_generation = (char**)malloc(rows * sizeof(char*));
    for(i = 0; i < rows; i++){
        world[i] = (char*)malloc(columns * sizeof(char));
        next_generation[i] = (char*)malloc(columns * sizeof(char));
    }
    i = 0;
    j = 0;
    while(fscanf(input_file, "%c", &cell) == 1){
        if(cell == 10 || cell == 124){
            continue;
        }
        world[i][j++] = cell;
        column_counter++;
        if(column_counter == columns){
            i++;
            j = 0;
            column_counter = 0;
        } 
    }
    fclose(input_file);
}

void update_world(){
#pragma omp parallel for
    for(int i = 0 ; i < rows ; i++){
        for(int j = 0 ; j < columns ; j++){
            world[i][j] = next_generation[i][j];
        }
    }
}

int is_alive_neighbor(int neighbor_row, int neighbor_col){
    if(neighbor_row >= 0 && neighbor_row < rows && neighbor_col >= 0 && neighbor_col < columns){
        if(world[neighbor_row][neighbor_col] == '*'){
            return 1;
        }
    }
    return 0;
}

int calc_neighbors(int i, int j){
    int alive_neighbors = 0;
    /* Top-left neighbor */
    alive_neighbors += is_alive_neighbor(i - 1, j - 1);
    /* Top neighbor */
    alive_neighbors += is_alive_neighbor(i - 1, j);
    /* Top-right neighbor */
    alive_neighbors += is_alive_neighbor(i - 1, j + 1);
    /* Left neighbor */
    alive_neighbors += is_alive_neighbor(i, j - 1);
    /* Right neighbor */
    alive_neighbors += is_alive_neighbor(i, j + 1);
    /* Bottom-left neighbor */
    alive_neighbors += is_alive_neighbor(i + 1, j - 1);
    /* Bottom neighbor */
    alive_neighbors += is_alive_neighbor(i + 1, j);
    /* Bottom-right neighbor */
    alive_neighbors += is_alive_neighbor(i + 1, j + 1);
    return alive_neighbors;
}

void calc_next_generation(){
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic, num_threads)
        for(int i = 0 ; i < rows ; i++){
            for(int j = 0 ; j < columns ; j++){
                int alive_neighbours = calc_neighbors(i, j);
                if(world[i][j] == '*'){
                    if(alive_neighbours < 2 || alive_neighbours > 3){
                        next_generation[i][j] = ' ';
                    }
                    else{
                        next_generation[i][j] = '*';
                    }
                } 
                else{
                    if(alive_neighbours == 3){
                        next_generation[i][j] = '*';
                    } 
                    else{
                        next_generation[i][j] = ' ';
                    }
                }
            }
        }
        #pragma omp single
        {
            update_world();
        }
    }
}

void print_to_file(char* filename){
    int i, j;
    FILE* output_file = fopen(filename, "w");
    if(output_file == NULL){
        printf("Error: Failed to open output file.\n");
        return;
    }
    fprintf(output_file, "%d %d\n", rows, columns);
    for(i = 0 ; i < rows ; i++){
        for(j = 0 ; j < columns ; j++){
            fprintf(output_file, "|%c", world[i][j]);
        }
        fprintf(output_file, "|\n");
    }
    fclose(output_file);
}

void free_func(){
    int i;
    for(i = 0 ; i < rows ; i++){
        free(world[i]);
        free(next_generation[i]);
    }
    free(world);
    free(next_generation);
}

double get_time(){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec + now.tv_nsec * 1e-9;
}

int main(int argc, char* argv[]){
    int i;
    char* input_filename;
    char* output_filename;
    int generation_number;
    double time;
    if(argc != 4){
        printf("Usage: %s <input_file> <generation_number> <output_file>\n", argv[0]);
        return 1;
    }
    printf("Enter the number of threads (1, 2, or 4): ");
    scanf("%d", &num_threads);
    time = get_time();
    if(num_threads != 1 && num_threads != 2 && num_threads != 4){
        printf("Invalid number of threads. Please enter 1, 2, or 4.\n");
        return 1;
    }
    omp_set_num_threads(num_threads);
    input_filename = argv[1];
    generation_number = atoi(argv[2]);
    output_filename = argv[3];
    read_file(input_filename);
    for(i = 0; i < generation_number; i++){
        calc_next_generation();
    }
    print_to_file(output_filename);
    free_func();
    printf("Execution Time: %.3f seconds\n", get_time() - time);
    return 0;
}
