// 331CC - Poalelungi Gabriel - Tema3APD - 17 ianuarie 2022
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
struct clstr {
    int coord;
    int no_workers;
    int workers[100];
};

struct clstr topology[3];

int min(double n1, double n2) {
    if (n1 <= n2)
        return n1;
    else return n2;
}

int main(int argc, char **argv) {

// Initial stage;
    int N = atoi(argv[1]);
    
    int numtasks, rank, coord_rank;

    int initial_array[N];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Workers for each cluster: cluster0 = c0, cluster1 = c1, cluster2 = c2;
    int c0[numtasks], c1[numtasks], c2[numtasks];
    // Number of workers in each array c0, c1, c2
    int nc0, nc1, nc2;

// Stage where each coordinator find who their workers are and send each worker who their 
// coordinator is. Also, the other coordinators find the other local topologies;
    if (rank == 0) {

        // Open file cluster0 for coordinator with rank 0;
        FILE *cluster0 = fopen("./cluster0.txt", "r");
        fscanf(cluster0, "%d", &nc0);

        // Save the rank of every worker in array c0 and send to each one their coordinator's rank
        for(int i = 1; i <= nc0; i++) {
            int current_worker;
            fscanf(cluster0, "%d", &current_worker);

            c0[i] = current_worker;

            MPI_Send(&rank, 1, MPI_INT, current_worker, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,current_worker);
        }

        // Coordinator with rank 0 knows his cluster's topology
        topology[0].coord = 0;
        topology[0].no_workers = nc0;
        for(int j = 1; j <= nc0; j++) {
            topology[0].workers[j] = c0[j];
        }

        // Send cluster0 topology to the other coordinators with rank 1 and 2
        for(int j = 1; j <= 2; j++) {
            MPI_Send(&topology[0].coord, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            MPI_Send(&topology[0].no_workers, 1, MPI_INT, j, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            for(int i = 1; i <= topology[0].no_workers; i++) {
                MPI_Send(&topology[0].workers[i], 1, MPI_INT, j, 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,j);
            }
        }
    } else if (rank == 1) {

        // Open file cluster1 for coordinator with rank 1;
        FILE *cluster1 = fopen("./cluster1.txt", "r");
        fscanf(cluster1, "%d", &nc1);

        // Save the rank of every worker in array c1 and send to each one their coordinator's rank
        for(int i = 1; i <= nc1; i++) {
            int current_worker;
            fscanf(cluster1, "%d", &current_worker);

            c1[i] = current_worker;

            MPI_Send(&rank, 1, MPI_INT, current_worker, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,current_worker);
        }

        // Coordinator with rank 1 knows his cluster's topology
        topology[1].coord = 1;
        topology[1].no_workers = nc1;
        for(int j = 1; j <= nc1; j++) {
            topology[1].workers[j] = c1[j];
        }

        // Send cluster1 topology to the other coordinators with rank 0 and 2
        for(int j = 0; j <= 2; j = j + 2) {
            MPI_Send(&topology[1].coord, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            MPI_Send(&topology[1].no_workers, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            for(int i = 1; i <= topology[1].no_workers; i++) {
                MPI_Send(&topology[1].workers[i], 1, MPI_INT, j, 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,j);
            }
        }
    } else if (rank == 2) {

        // Open file cluster2 for coordinator with rank 2;
        FILE *cluster2 = fopen("./cluster2.txt", "r");
        fscanf(cluster2, "%d", &nc2);

        // Save the rank of every worker in array c2 and send to each one their coordinator's rank
        for(int i = 1; i <= nc2; i++) {
            int current_worker;
            fscanf(cluster2, "%d", &current_worker);

            c2[i] = current_worker;

            MPI_Send(&rank, 1, MPI_INT, current_worker, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,current_worker);
        }

        // Coordinator with rank 2 knows his cluster's topology
        topology[2].coord = 2;
        topology[2].no_workers = nc2;
        for(int j = 1; j <= nc2; j++) {
            topology[2].workers[j] = c2[j];
        }

        // Send cluster1 topology to the other coordinators with rank 0 and 1
        for(int j = 0; j <= 1; j++) {
            MPI_Send(&topology[2].coord, 1, MPI_INT, j, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            MPI_Send(&topology[2].no_workers, 1, MPI_INT, j, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,j);

            for(int i = 1; i <= topology[2].no_workers; i++) {
                MPI_Send(&topology[2].workers[i], 1, MPI_INT, j, 2, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,j);
            }
        }

    } else {
        // Let workers know who their coordinator is
        MPI_Status status;
        MPI_Recv(&coord_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

// Stage where everybody finds what's the topology of the system
    if (rank == 0) {
        // Receive the topology of the clusters from each coordinator, 1 and 2;
        for(int i = 1; i <= 2; i++) {
            MPI_Status status;

            MPI_Recv(&topology[i].coord, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);

            MPI_Recv(&topology[i].no_workers, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);

            for(int j = 1; j <= topology[i].no_workers; j++) {
                MPI_Recv(&topology[i].workers[j], 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            }
        }
        // Print the entire topology known by coordinator with rank 0;
        printf("0 ->");
        for(int i = 0; i <= 2; i++) {
            printf(" %d:", topology[i].coord);
            for(int j = 1; j < topology[i].no_workers; j++) {
                printf("%d,", topology[i].workers[j]);
            }
            printf("%d", topology[i].workers[topology[i].no_workers]);
        }
        printf("\n");

        // Send the entire topology to the subordinate workers of coordinator with rank 0;
        // For each worker subordinate to coordinator 0;
        for(int i = 1; i <= nc0; i++) {
            // Send it the topology of each cluster;
            for(int j = 0; j <= 2; j++) {
                MPI_Send(&topology[j].coord, 1, MPI_INT, c0[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c0[i]);

                MPI_Send(&topology[j].no_workers, 1, MPI_INT, c0[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c0[i]);

                for(int k = 1; k <= topology[j].no_workers; k++) {
                    MPI_Send(&topology[j].workers[k], 1, MPI_INT, c0[i], j, MPI_COMM_WORLD);
                    printf("M(%d,%d)\n",rank,c0[i]);
                }
            }
        }
    } else if (rank == 1) {
        // Receive the topology of the clusters from each coordinator, 0 and 2;
        for(int i = 0; i <= 2; i = i + 2) {
            MPI_Status status;

            MPI_Recv(&topology[i].coord, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);

            MPI_Recv(&topology[i].no_workers, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);

            for(int j = 1; j <= topology[i].no_workers; j++) {
                MPI_Recv(&topology[i].workers[j], 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            }
        }

        // Print the entire topology known by coordinator with rank 1;
        printf("1 ->");
        for(int i = 0; i <= 2; i++) {
            printf(" %d:", topology[i].coord);
            for(int j = 1; j < topology[i].no_workers; j++) {
                printf("%d,", topology[i].workers[j]);
            }
            printf("%d", topology[i].workers[topology[i].no_workers]);
        }
        printf("\n");

        // Send the entire topology to the subordinate workers of coordinator with rank 1;
        // For each worker subordinate to coordinator 1:
        for(int i = 1; i <= nc1; i++) {
            // Send it the topology of each cluster;
            for(int j = 0; j <= 2; j++) {
                MPI_Send(&topology[j].coord, 1, MPI_INT, c1[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c1[i]);
                MPI_Send(&topology[j].no_workers, 1, MPI_INT, c1[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c1[i]);
                for(int k = 1; k <= topology[j].no_workers; k++) {
                    MPI_Send(&topology[j].workers[k], 1, MPI_INT, c1[i], j, MPI_COMM_WORLD);
                    printf("M(%d,%d)\n",rank,c1[i]);
                }
            }
        }
    } else if (rank == 2) {
        // Receive the topology of the clusters of each coordinator, 0 and 1;
        for(int i = 0; i <= 1; i++) {
            MPI_Status status;
            MPI_Recv(&topology[i].coord, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            MPI_Recv(&topology[i].no_workers, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);

            for(int j = 1; j <= topology[i].no_workers; j++) {
                MPI_Recv(&topology[i].workers[j], 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            }
        }

        // Print the entire topology known by coordinator with rank 1;
        printf("2 ->");
        for(int i = 0; i <= 2; i++) {
            printf(" %d:", topology[i].coord);
            for(int j = 1; j < topology[i].no_workers; j++) {
                printf("%d,", topology[i].workers[j]);
            }
            printf("%d", topology[i].workers[topology[i].no_workers]);
        }
        printf("\n");


        // Send the entire topology to the subordinate workers of coordinator with rank 2;
        // For each worker subordinate to coordinator 2:
        for(int i = 1; i <= nc2; i++) {
            // Send it the topology of each cluster;
            for(int j = 0; j <= 2; j++) {
                MPI_Send(&topology[j].coord, 1, MPI_INT, c2[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c2[i]);
                MPI_Send(&topology[j].no_workers, 1, MPI_INT, c2[i], j, MPI_COMM_WORLD);
                printf("M(%d,%d)\n",rank,c2[i]);
                for(int k = 1; k <= topology[j].no_workers; k++) {
                    MPI_Send(&topology[j].workers[k], 1, MPI_INT, c2[i], j, MPI_COMM_WORLD);
                    printf("M(%d,%d)\n",rank,c2[i]);
                }
            }
        }
    } else {
        // This worker receives the topology of each cluster and saves the entire topology of the system
        for(int i = 0; i <= 2; i++) {
            MPI_Status status;
            MPI_Recv(&topology[i].coord, 1, MPI_INT, coord_rank, i, MPI_COMM_WORLD, &status);
            MPI_Recv(&topology[i].no_workers, 1, MPI_INT, coord_rank, i, MPI_COMM_WORLD, &status);

            for(int j = 1; j <= topology[i].no_workers; j++) {
                MPI_Recv(&topology[i].workers[j], 1, MPI_INT, coord_rank, i, MPI_COMM_WORLD, &status);
            }
        }

        // Print the entire topology known by this worker;
        printf("%d ->", rank);
        for(int i = 0; i <= 2; i++) {
            printf(" %d:", topology[i].coord);
            for(int j = 1; j < topology[i].no_workers; j++) {
                printf("%d,", topology[i].workers[j]);
            }
            printf("%d", topology[i].workers[topology[i].no_workers]);
        }
        printf("\n");
    }

/* 
Stage where coordinator 0:
 - initializes the array
 - decides in how many parts the array is shared
 - decides how many parts each cluster receives in such way that every worker from every cluster
   has the same amount of work
 - send the assigned parts to coordinators 1 and 2
 - share its part among its workers
Coordinator 1 and 2 share the received part to each worker;
The workers do their assigned job; 
*/
    if (rank == 0) {
        //Generate the array with i from 0 to N-1, v[i] = i;
        for(int i = 0; i < N; i++) {
            initial_array[i] = i;
        }
        
        // Number of total workers of the system;
        int total_workers = topology[0].no_workers + topology[1].no_workers + topology[2].no_workers;

        // What part of the entire array each cluster receives(cluster0, cluster1 and cluster2);
        int n_partial_array0 = topology[0].no_workers * ceil((double)N / total_workers);
        int n_partial_array1 = topology[1].no_workers * ceil((double)N / total_workers);
        int n_partial_array2 = topology[2].no_workers * ceil((double)N / total_workers);

        // Start and end indexes for cluster0's part of the array;
        int start0 = 0;
        int end0 = n_partial_array0;

        // Start and end indexes for cluster1's part of the array;
        int start1 = end0;
        int end1 = end0 + n_partial_array1;

        // Start and end indexes for cluster2's part of the array;
        int start2 = end1;
        int end2 = end1 + n_partial_array2;

        // Send the dimension of the assigned array to cluster0's coordinator;
        MPI_Send(&n_partial_array1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 0, 1);   

        // Send the dimension of the assigned array to cluster1's coordinator;
        MPI_Send(&n_partial_array2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 0, 2);

        // Send the assigned array to the cluster1's coordinator;
        for(int i = start1; i <= end1; i++) {
            MPI_Send(&initial_array[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 0, 1);
        }

        // Send the assigned array to the cluster2's coordinator;
        for(int i = start2; i <= end2; i++) {
            MPI_Send(&initial_array[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 0, 2);
        }

        // Share the assigned part of the array to every worker of cluster0;
        for(int i = 1; i <= topology[0].no_workers; i++) {
            // Start and stop indexes for the assigned part of the array to this worker;
            int start = (i-1) * (double) n_partial_array0 / topology[0].no_workers;
            int stop = min(n_partial_array0, i * (double) n_partial_array0 / topology[0].no_workers);

            // Dimension of the assigned part of the array to this worker;
            int n = stop - start;

            // Send the dimension to this worker;
            MPI_Send(&n, 1, MPI_INT, topology[0].workers[i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 0, topology[0].workers[i]);

            // Send the assigned array to this worker;
            for(int j = start; j < stop; j++) {
                MPI_Send(&initial_array[j], 1, MPI_INT, topology[0].workers[i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", 0, topology[0].workers[i]);
            }
        }
    } else if (rank == 1) {
        int partial_array1[N];
        int n1;

        // Receive the dimension of the assigned array for cluster1;
        MPI_Status status;
        MPI_Recv(&n1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // Receive the assigned array for cluster1 from coordinator 0;
        for(int i = 0; i < n1; i++) {
            MPI_Recv(&partial_array1[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }

        // Share the assigned part of the array to every worker of cluster1;
        for(int i = 1; i <= topology[1].no_workers; i++) {
            // Start and stop indexes for the assigned part of the array to this worker;
            int start = (i-1) * (double) n1 / topology[1].no_workers;
            int stop = min(n1, i * (double) n1 / topology[1].no_workers);

            // Dimension of the assigned part of the array to this worker;
            int n = stop - start;

            // Send the dimension to this worker;
            MPI_Send(&n, 1, MPI_INT, topology[1].workers[i], 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 1, topology[1].workers[i]);

            // Send the assigned array to this worker;
            for(int j = start; j < stop; j++) {
                MPI_Send(&partial_array1[j], 1, MPI_INT, topology[1].workers[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", 1, topology[1].workers[i]);
            }
        }

    } else if (rank == 2) {
        int partial_array2[N];
        int n2;

        // Receive the dimension of the assigned array for cluster2;
        MPI_Status status;
        MPI_Recv(&n2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // Receive the assigned array for cluster2 from coordinator 0;
        for(int i = 0; i < n2; i++) {
            MPI_Recv(&partial_array2[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }

        // Share the assigned part of the array to every worker of cluster2;
        for(int i = 1; i <= topology[2].no_workers; i++) {
            // Start and stop indexes for the assigned part of the array to this worker;
            int start = (i-1) * (double) n2 / topology[2].no_workers;
            int stop = min(n2, i * (double) n2 / topology[2].no_workers);

            // Dimension of the assigned part of the array to this worker;
            int n = stop - start;

            // Send the dimension to this worker;
            MPI_Send(&n, 1, MPI_INT, topology[2].workers[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 2, topology[2].workers[i]);

            // Send the assigned array to this worker;
            for(int j = start; j < stop; j++) {
                MPI_Send(&partial_array2[j], 1, MPI_INT, topology[2].workers[i], 2, MPI_COMM_WORLD);
               printf("M(%d,%d)\n", 2, topology[2].workers[i]);
            }
        }
    } else {
        int partial_array[N];
        int n;

        // Receive from this worker's coordinator the dimension and the content 
        // of the assigned part of the array; 
        MPI_Status status;
        MPI_Recv(&n, 1, MPI_INT, coord_rank, coord_rank, MPI_COMM_WORLD, &status);
        for(int i = 0; i < n; i++) {
            MPI_Recv(&partial_array[i], 1, MPI_INT, coord_rank, coord_rank, MPI_COMM_WORLD, &status);
            // Double each element of the array;
            partial_array[i] = partial_array[i] * 2;
        }

        // Send back to the coordinator the dimension and the changed content of the
        // assigned part of the array;
        MPI_Send(&n, 1, MPI_INT, coord_rank, rank, MPI_COMM_WORLD);
        printf("M(%d,%d)\n",rank,coord_rank);
        for(int i = 0; i < n; i++) {
            MPI_Send(&partial_array[i], 1, MPI_INT, coord_rank, rank, MPI_COMM_WORLD);
            printf("M(%d,%d)\n",rank,coord_rank);
        }
    }

// Stage where each coordinator receives the results of jobs of each worker they coordinate;
// Also, they send the reassemebled part to the coordinator 0;
    if(rank == 0) {
        int partial_array[N];
        int n = 0;

        // Receive the changed parts from each worker of cluster0 and reassemble them;
        MPI_Status status;
        for (int i = 1; i <= topology[0].no_workers; i++) {
            int n_i;

            MPI_Recv(&n_i, 1, MPI_INT, topology[0].workers[i], topology[0].workers[i], MPI_COMM_WORLD, &status);
            for(int j = n; j < n_i + n; j++) {
                MPI_Recv(&partial_array[j], 1, MPI_INT, topology[0].workers[i], topology[0].workers[i], MPI_COMM_WORLD, &status);
            }

            n += n_i;
        }

        // Send the dimension and the reassembled array to the coordinator 0;
        // (In this case, it sends it to himself, but it's necessary for the next part)
        // (Here it will not print to stdout that a message was sent)
        MPI_Send(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        for(int i = 0; i < n; i++) {
            MPI_Send(&partial_array[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }

    } else if(rank == 1) {
        int partial_array[N];
        int n = 0;

        // Receive the changed parts from each worker of cluster1 and reassemble them;
        MPI_Status status;
        for (int i = 1; i <= topology[1].no_workers; i++) {
            int n_i;

            MPI_Recv(&n_i, 1, MPI_INT, topology[1].workers[i], topology[1].workers[i], MPI_COMM_WORLD, &status);
            for(int j = n; j < n + n_i; j++) {
                MPI_Recv(&partial_array[j], 1, MPI_INT, topology[1].workers[i], topology[1].workers[i], MPI_COMM_WORLD, &status);
            }

            n += n_i;
        }

        // Send the dimension and the reassembled array to the coordinator 0;
        MPI_Send(&n, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 1, 0);

        for(int i = 0; i < n; i++) {
            MPI_Send(&partial_array[i], 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 1, 0);
        }
    } else if(rank == 2) {
        int partial_array[N];
        int n = 0;

        // Receive the changed parts from each worker of cluster2 and reassemble them;
        MPI_Status status;
        for (int i = 1; i <= topology[2].no_workers; i++) {
            int n_i;

            MPI_Recv(&n_i, 1, MPI_INT, topology[2].workers[i], topology[2].workers[i], MPI_COMM_WORLD, &status);
            for(int j = n; j < n + n_i; j++) {
                MPI_Recv(&partial_array[j], 1, MPI_INT, topology[2].workers[i], topology[2].workers[i], MPI_COMM_WORLD, &status);
            }

            n += n_i;
        }

        // Send the dimension and the reassembled array to the coordinator 0;
        MPI_Send(&n, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", 2, 0);

        for(int i = 0; i < n; i++) {
            MPI_Send(&partial_array[i], 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", 2, 0);
        }
    }

// Stage where coordinator 0 receives the assembled parts from each coordinator and reassemble
// them again to reform the initial array, but with every value being doubled;
    if(rank == 0) {
        // Received the arrays from each cluster's coordinator and reassemble them;
        int n = 0;
        MPI_Status status;


        for(int i = 0; i <= 2; i++) {
            int n_i;

            MPI_Recv(&n_i, 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            for (int j = n; j < n + n_i; j++) {
                MPI_Recv(&initial_array[j], 1, MPI_INT, i, i, MPI_COMM_WORLD, &status);
            }

            n += n_i;
        }

        // Prints the final result
        printf("Rezultat: ");
        for(int i = 0; i < N; i++) {
            printf("%d ", initial_array[i]);
        }
        printf("\n");
    }
    MPI_Finalize();
    return 0;
}
