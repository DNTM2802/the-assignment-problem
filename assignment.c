////////////////////////////////////////////////////////////////////////////////////////////////////
//
// AED, 2019/2020
//
// DINIS CRUZ 92080
// DUARTE MORTÁGUA 92963
// TIAGO OLIVEIRA 93456
//
// Brute-force solution of the assignment problem
// (https://en.wikipedia.org/wiki/Assignment_problem)
//
// Compile with "cc -Wall -O2 assignment.c -lm" or equivalent
//
// In the assignment problem we will solve here we have n agents and n tasks;
// assigning agent
//   a
// to task
//   t
// costs
//   cost[a][t]
// The goal of the problem is to assign one agent to each task such that the
// total cost is minimized
// The total cost is the sum of the costs

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define NDEBUG  // uncomment to skip disable asserts (makes the code slightly
// faster)
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// problem data
//
// max_n ........ maximum problem size
// cost[a][t] ... cost of assigning agent a to task t
//
//
// if your compiler complains about srandom() and random(), replace #if 0 by #if
// 1
//
#if 0
#define srandom srand
#define random rand
#endif

#define max_n 32 // do not change this (maximum number of agents, and tasks)
#define range                                                                  \
  20 // do not change this (for the pseudo-random generation of costs)
#define t_range                                                                \
  (3 * range) // do not change this (maximum cost of an assignment)

static int cost[max_n][max_n];
static int seed; // place a student number here!

static void init_costs(int n) {
  if (n == -3) { // special case (example for n=3)

    cost[0][0] = 3;   cost[0][1] = 8;   cost[0][2] = 6;
    cost[1][0] = 4;   cost[1][1] = 7;   cost[1][2] = 5;
    cost[2][0] = 5;   cost[2][1] = 7;   cost[2][2] = 5;

    return;
  }
  if (n == -5) { // special case (example for n=5)

    cost[0][0] = 27;    cost[0][1] = 27;    cost[0][2] = 25;    cost[0][3] = 41;    cost[0][4] = 24;
    cost[1][0] = 28;    cost[1][1] = 26;    cost[1][2] = 47;    cost[1][3] = 38;    cost[1][4] = 21;
    cost[2][0] = 22;    cost[2][1] = 48;    cost[2][2] = 26;    cost[2][3] = 14;    cost[2][4] = 24;
    cost[3][0] = 32;    cost[3][1] = 31;    cost[3][2] = 9;     cost[3][3] = 41;    cost[3][4] = 36;
    cost[4][0] = 24;    cost[4][1] = 34;    cost[4][2] = 30;    cost[4][3] = 35;    cost[4][4] = 45;

    return;
  }
  assert(n >= 1 && n <= max_n);
  srandom((unsigned int)seed * (unsigned int)max_n + (unsigned int)n);
  for (int a = 0; a < n; a++)
    for (int t = 0; t < n; t++)
      cost[a][t] = 3 + (random() % range) + (random() % range) + (random() % range); // [3,3*range]
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// code to measure the elapsed time used by a program fragment (an almost copy
// of elapsed_time.h)
//
// use as follows:
//
//   (void)elapsed_time();
//   // put your code to be time measured here
//   dt = elapsed_time();
//   // put morecode to be time measured here
//   dt = elapsed_time();
//
// elapsed_time() measures the CPU time between consecutive calls
//

#if defined(__linux__) || defined(__APPLE__)

//
// GNU/Linux and MacOS code to measure elapsed time
//

#include <time.h>

static double elapsed_time(void) {
  static struct timespec last_time, current_time;

  last_time = current_time;
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &current_time) != 0)
    return -1.0; // clock_gettime() failed!!!
  return ((double)current_time.tv_sec - (double)last_time.tv_sec) + 1.0e-9 * ((double)current_time.tv_nsec - (double)last_time.tv_nsec);
}

#endif

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)

//
// Microsoft Windows code to measure elapsed time
//

#include <windows.h>

static double elapsed_time(void) {
  static LARGE_INTEGER frequency, last_time, current_time;
  static int first_time = 1;

  if (first_time != 0) {
    QueryPerformanceFrequency(&frequency);
    first_time = 0;
  }
  last_time = current_time;
  QueryPerformanceCounter(&current_time);
  return (double)(current_time.QuadPart - last_time.QuadPart) /
         (double)frequency.QuadPart;
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// function to generate a pseudo-random permutation
//

void random_permutation(int n, int t[n]) {
  assert(n >= 1 && n <= 1000000);
  for (int i = 0; i < n; i++)
    t[i] = i;
  for (int i = n - 1; i > 0; i--) {
    int j = (int)floor((double)(i + 1) * (double)random() / (1.0 + (double)RAND_MAX)); // range 0..i
    assert(j >= 0 && j <= i);
    int k = t[i];
    t[i] = t[j];
    t[j] = k;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// place to store best and worst solutions (also code to print them)
//
static int min_cost, min_cost_assignment[max_n]; // smallest cost information
static int max_cost, max_cost_assignment[max_n]; // largest cost information
long histo[60 * max_n + 1];
FILE *f_custos; // CODE FOR EXPORTING HISTOGRAMS AND EXECUTION TIME GRAPHS
FILE *f_reps;   // CODE FOR EXPORTING HISTOGRAMS AND EXECUTION TIME GRAPHS
FILE *n_tested; // CODE FOR EXPORTING HISTOGRAMS AND EXECUTION TIME GRAPHS
FILE *n_time;   // CODE FOR EXPORTING HISTOGRAMS AND EXECUTION TIME GRAPHS
static long n_visited; // number of permutations visited (examined)
// place your histogram global variable here
static double cpu_time;

#define minus_inf -1000000000 // a very small integer
#define plus_inf +1000000000  // a very large integer

static void reset_solutions(void) {
  min_cost = plus_inf;
  max_cost = minus_inf;
  n_visited = 0l;
  memset(histo, 0, sizeof histo);
  cpu_time = 0.0;
}

#define show_info_1 (1 << 0)
#define show_info_2 (1 << 1)
#define show_costs (1 << 2)
#define show_min_solution (1 << 3)
#define show_max_solution (1 << 4)
#define show_histogram (1 << 5)
#define show_all (0xFFFF)

static void show_solutions(int n, char *header, int what_to_show) {

  printf("%s\n", header);
  if ((what_to_show & show_info_1) != 0) {
    printf("  seed .......... %d\n", seed);
    printf("  n ............. %d\n", n);
  }
  if ((what_to_show & show_info_2) != 0) {
    printf("  visited ....... %ld\n", n_visited);
    printf("  cpu time ...... %.3fs\n", cpu_time);
    // n_time = fopen("n_time", "a");
    // n_tested = fopen("n_tested", "a");
    // if (n_time == NULL || n_tested == NULL) {
    //   printf("Error opening file!");
    //   exit(1);
    // } else {
    //   fprintf(n_tested, "%d\n", n);
    //   fprintf(n_time, "%f\n", cpu_time);
    //   fclose(n_time);
    //   fclose(n_tested);
    // }
  }
  if ((what_to_show & show_costs) != 0) {
    printf("  costs ......... (ommited)\n");
    // for (int a = 0; a < n; a++) {
    //   for (int t = 0; t < n; t++)
    //     printf(" %2d", cost[a][t]);
    //   printf("\n%s", (a < n - 1) ? "                 " : "");
    // }
  }
  if ((what_to_show & show_min_solution) != 0) {
    printf("  min cost ...... %d\n", min_cost);
    if (min_cost != plus_inf) {
      printf("  assignement ...");
      for (int i = 0; i < n; i++)
        printf(" %d", min_cost_assignment[i]);
      printf("\n");
    }
  }
  if ((what_to_show & show_max_solution) != 0) {
    printf("  max cost ...... %d\n", max_cost);
    if (max_cost != minus_inf) {
      printf("  assignement ...");
      for (int i = 0; i < n; i++)
        printf(" %d", max_cost_assignment[i]);
      printf("\n");
    }
  }
  if ((what_to_show & show_histogram) != 0) {

    // char str_custos[12];          // WE DECIDED TO EXPORT ALL THE
    // char str_reps[12];            // NEEDED DATA TO SEPARATE FILES AND
    // sprintf(str_custos, "%d", n); // THEN LOAD IT IN OCTAVE. ALL THE
    // sprintf(str_reps, "%d", n);   // FIGURES IN THE REPORT WERE PRODUCED
    // strcat(str_custos, "_costs"); // WITH OCTAVE
    // strcat(str_reps, "_reps");
    // f_custos = fopen(str_custos, "w+");
    // f_reps = fopen(str_reps, "w+");
    // printf("%d %d ", min_cost, max_cost);
    // if (f_custos == NULL || f_reps == NULL) {
    //   printf("Error opening file!");
    //   exit(1);
    // } else {
    //   printf("%d %d", min_cost, max_cost);
    //   for (int i = 42; i <= 840; i++) {
    //     fprintf(f_custos, "%d\n", i);
    //     fprintf(f_reps, "%ld\n", histo[i]);
    //   }
    //   fclose(f_custos);
    //   fclose(f_reps);
    // }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// code used to generate all permutations of n objects
//
// n ........ number of objects
// m ........ index where changes occur (a[0], ..., a[m-1] will not be changed)
// a[idx] ... the number of the object placed in position idx
//
// TODO: modify the following function to solve the assignment problem
//

static void generate_all_permutations(int n, int m, int a[n]) {
  if (m < n - 1) {
    for (int i = m; i < n; i++) {
#define swap(i, j)                                                             \
  do {                                                                         \
    int t = a[i];                                                              \
    a[i] = a[j];                                                               \
    a[j] = t;                                                                  \
  } while (0)
      swap(i, m);                             // exchange a[i] with a[m]
      generate_all_permutations(n, m + 1, a); // recurse
      swap(i, m);                             // undo the exchange of a[i] with a[m]
#undef swap
    }
  }
  else {
    int temp_cost = 0;
    n_visited++;
    for (int x = 0; x < n; x++) {     // GET THE TOTAL COST OF A SINGLE PERMUTATION
      temp_cost += cost[x][a[x]];     // GET THE TOTAL COST OF A SINGLE PERMUTATION
    }
    // histo[temp_cost] = histo[temp_cost] + 1;  // UPDATE HISTOGRAM
    if (temp_cost < min_cost) {               // CHECK IF THIS PERMUTATION IS THE CHEAPEST SO FAR
      min_cost = temp_cost;                   // IF IT IS, UPDATE THE CHEAPEST PERMUTATION TO THIS ONE
      for (int y = 0; y < n; y++) {           // UPDATE THE ASSIGNMENT
        min_cost_assignment[y] = a[y];
      }
    } else {                                  // IF IT'S NOT THE CHEAPEST, IT MAY BE THE MORE EXPENSIVE ONE
      if (temp_cost > max_cost) {             // CHECK THAT
        max_cost = temp_cost;                 // IF IT IS, UPDATE THE MORE EXPENSIVE PERMUTATION TO THIS ONE
        for (int t = 0; t < n; t++) {         // UPDATE THE ASSIGNMENT
          max_cost_assignment[t] = a[t];
        }
      }
    }
  }
}

static void generate_all_permutations_branch_and_bound_min(int n, int m, int a[n], int medium_cost) {
  if (m < n - 1) {
    if (min_cost < medium_cost + 3 * (n - m)) { // CHECK IF, SO FAR, THIS PERMUTATION IS ALREADY
                                                // CONDEMNED// TO BE MORE EXPENSIVE THAN THE CHEAPEST
      return;                                   // SO FAR, AND IF IT IS, DISCARD IT.
    }
    for (int i = m; i < n; i++) {
#define swap(i, j)                                                             \
  do {                                                                         \
    int t = a[i];                                                              \
    a[i] = a[j];                                                               \
    a[j] = t;                                                                  \
  } while (0)
      swap(i, m);                       // exchange a[i] with a[m]
      generate_all_permutations_branch_and_bound_min(n, m + 1, a, medium_cost + cost[m][a[m]]); // UPDATE PERMUTATION COST SO FAR
      swap(i, m);                       // undo the exchange of a[i] with a[m]
#undef swap
    }
    return;
  }

  else {
    int total_cost = medium_cost + cost[m][a[m]]; // UPDATE PERMUTATION COST
    n_visited++;
    // histo[total_cost] = histo[total_cost] + 1;    // UPDATE HISTOGRAM
    if (total_cost <
        min_cost) {                               // CHECK IF THIS PERMUTATION IS THE CHEAPEST SO FAR
      min_cost = total_cost;
      for (int y = 0; y < n; y++) {               // UPDATE THE ASSIGNMENT
        min_cost_assignment[y] = a[y];
      }
    }
  }
}

static void generate_all_permutations_branch_and_bound_max(int n, int m, int a[n], int medium_cost) {
  if (m < n - 1) {
    if (max_cost > medium_cost + 60 * (n - m)) { // CHECK IF, SO FAR, THIS PERMUTATION IS ALREADY
                                                 // CONDEMNEDO BE LESS EXPENSIVE THAN THE MOST EXPENSIVE
      return;                                    // SO FAR, AND IF IT IS, DISCARD IT.
    }
    for (int i = m; i < n; i++) {
#define swap(i, j)                                                             \
  do {                                                                         \
    int t = a[i];                                                              \
    a[i] = a[j];                                                               \
    a[j] = t;                                                                  \
  } while (0)
      swap(i, m); // exchange a[i] with a[m]
      generate_all_permutations_branch_and_bound_max(
          n, m + 1, a,
          medium_cost + cost[m][a[m]]); // UPDATE PERMUTATION COST SO FAR
      swap(i, m);                       // undo the exchange of a[i] with a[m]
#undef swap
    }
    return;
  }

  else {
    int total_cost = medium_cost + cost[m][a[m]]; // UPDATE PERMUTATION COST
    // histo[total_cost] = histo[total_cost] + 1;    // UPDATE HISTOGRAM
    n_visited++;
    if (total_cost >  max_cost) {     // CHECK IF THIS PERMUTATION IS THE MOST EXPENSIVE SO FAR
      max_cost = total_cost;
      for (int y = 0; y < n; y++) {   // UPDATE THE ASSIGNMENT
        max_cost_assignment[y] = a[y];
      }
    }
  }
}

static void generate_random_permutations(int n) {
  int arr_perm[n];
  for (int count = 0; count < 1000000; count++) {
    random_permutation(n, arr_perm);  // UMA RANDOM PERMUTATION
    int temp_cost = 0;
    for (int x = 0; x < n; x++) {
      temp_cost += cost[x][arr_perm[x]];  // CUSTO DA PERMUTAÇÃO
    }
    // histo[n_visited] = min_cost; // UPDATE HISTOGRAM
    n_visited++;
    if (temp_cost < min_cost) {  // VER SE É A PERMUTAÇÃO COM CUSTO MÍNIMO
      min_cost = temp_cost;
      for (int y = 0; y < n; y++) {
        min_cost_assignment[y] = arr_perm[y];
      }
    }
    if (temp_cost > max_cost) {
      max_cost = temp_cost;
      for (int t = 0; t < n; t++) {
        max_cost_assignment[t] = arr_perm[t];
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// main program
//

int main(int argc, char **argv) {
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'e') {
    seed = 0;
    {
      memset(histo, 0, sizeof(histo));
      int n = 3;
      init_costs(-3); // costs for the example with n = 3
      int a[n];
      for (int i = 0; i < n; i++)
        a[i] = i;
      reset_solutions();
      (void)elapsed_time();
      generate_all_permutations(n, 0, a);
      cpu_time = elapsed_time();
      show_solutions(n, "Example for n=3", show_all);
      printf("\n");
    }
    {
      memset(histo, 0, sizeof(histo));
      int n = 5;
      init_costs(-5); // costs for the example with n = 5
      int a[n];
      for (int i = 0; i < n; i++)
        a[i] = i;
      reset_solutions();
      (void)elapsed_time();
      generate_all_permutations(n, 0, a);
      cpu_time = elapsed_time();
      show_solutions(n, "Example for n=5", show_all);
      return 0;
    }
  }
  if (argc == 2) {
    seed = atoi(argv[1]); // seed = student number
    if (seed >= 0 && seed <= 1000000) {
      for (int n = 1; n <= max_n; n++) {
        memset(histo, 0, sizeof(histo));
        init_costs(n);
        show_solutions(n, "Problem statement", show_info_1 | show_costs);

        if (n < 14)
        {
          int a[n];
          for(int i = 0;i < n;i++)
          a[i] = i; // initial permutation
          reset_solutions();
          (void)elapsed_time();
          generate_all_permutations(n,0,a);      // DONE WITH BRUTE FORCE SOLUTION
          cpu_time = elapsed_time();
          show_solutions(n,"Brute force", show_info_2 | show_min_solution | show_max_solution);
          reset_solutions();
          (void)elapsed_time();
          generate_random_permutations(n);   // DONE WITH RANDOM PERMUTATIONS
          cpu_time = elapsed_time();
          show_solutions(n,"Random Permutations", show_info_2 | show_min_solution | show_max_solution);
        }

        if (n < 16)
        {
          int a[n];
          for (int i = 0; i < n; i++)
            a[i] = i; // initial permutation
          reset_solutions();
          (void)elapsed_time();
          generate_all_permutations_branch_and_bound_max(n, 0, a, 0); // DONE WITH BRUTE FORCE BRANCH-AND-BOUND SOLUTION
          cpu_time = elapsed_time();
          show_solutions(n, "Brute force with branch-and-bound max",show_max_solution | show_info_2);
          reset_solutions();
          (void)elapsed_time();
          generate_all_permutations_branch_and_bound_min(n, 0, a, 0); // DONE WITH BRUTE FORCE BRANCH-AND-BOUND SOLUTION
          cpu_time = elapsed_time();
          show_solutions(n, "Brute force with branch-and-bound min",show_min_solution | show_info_2);
        }
        printf("\n");
      }
      return 0;
    }
  }
  fprintf(stderr, "usage: %s -e              # for the examples\n", argv[0]);
  fprintf(stderr, "usage: %s student_number\n", argv[0]);
  return 1;
}
