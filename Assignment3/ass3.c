#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int main (int argc, char *argv[]) {
    int	numtasks,            /* number of tasks in partition */
        taskid,                /* a task identifier */
        numworkers,            /* number of worker tasks */
        source,                /* task id of message source */
        dest,                  /* task id of message destination */
        mtype,                 /* message type */
        rows,                  /* rows of matrix A sent to each worker */
        columns,               /* columns of matrix A sent to each worker */
        avecolumn, averow,/* used to determine rows and column sent to each worker */
        extra, offset,    /* used to determine rows and column sent to each worker */
        i, j, k, rc;           /* misc */

    int NCA = atoi(argv[1]);
    int NRA = atoi(argv[2]);
    int choice = atoi(argv[3]);
    double elapseTime;

    double	*a = (double *)malloc(NRA * NCA * sizeof(double)),  /* matrix A to be multiplied */
            *b = (double *)malloc(NCA * NRA * sizeof(double)),  /* matrix B to be multiplied */
            *c = (double *)malloc(NRA * NRA * sizeof(double));  /* matrix C to be returned */


    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    if (numtasks < 2 ) {
    MPI_Abort(MPI_COMM_WORLD, rc);
    exit(1);
    }
    numworkers = numtasks-1;

    if (choice == 0) { 
    // If user enters 0, then Row-Major
	   if (taskid == MASTER) {
           // randomize the matrix entries using drand48.
            for (i=0; i<NRA; i++) {
                for (j=0; j<NCA; j++) {
                    *(a + i * NCA + j)= drand48();
                }
            }
            for (i=0; i<NCA; i++) {
                for (j=0; j<NRA; j++) {
                    *(b + i * NRA + j)= drand48();
                }
            }

            averow = NRA/numworkers;
            extra = NRA%numworkers;
            offset = 0;
            mtype = FROM_MASTER;
            elapseTime = MPI_Wtime(); // Initial time recorded
            // Use a for loop to send data to worker tasks
            for (dest=1; dest<=numworkers; dest++) {
                rows = (dest <= extra) ? averow+1 : averow;
                printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
                MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(a + offset * NCA + 0, rows*NCA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(b, NCA*NRA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
                offset = offset + rows;
            }

            mtype = FROM_WORKER;
            // Use a for loop to receice data from workers tasks after they finish.
            for (i=1; i<=numworkers; i++) {
                source = i;
                MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
                MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
                MPI_Recv(c + offset * NRA + 0, rows*NRA, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
            }
            // Use the new time stamp and the old time recorded to calculate time in between.
            elapseTime = MPI_Wtime()-elapseTime;

            // Print the results.
            printf("elapseTime is %f s\n", elapseTime);
	    }


        if (taskid > MASTER) {
            // workers tasks
            mtype = FROM_MASTER;

            // receive data from master side.
            MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(b, NCA*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

            // Calculation
            for (k=0; k<NRA; k++) {
                for (i=0; i<rows; i++) {
                    *(c + i *NRA + k) = 0.0;
                }
                for (j=0; j<NCA; j++){
                    *(c + i * NRA + k) += *(a + i * NCA + j) * (*(b + j * NRA  + k));
                }
            }
            
            mtype = FROM_WORKER;
            // Send results back to master task.
            MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
            MPI_Send(c, rows*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
	   }
       // Finalize the MPI task after finish.
	   MPI_Finalize();
	}

	if (choice == 1) {
        // If user input 1, then column-Major.
		if (taskid == MASTER) {
            // Master task part.
            // Randomize the enties.
            for (i=0; i<NRA; i++) {
                for (j=0; j<NCA; j++) {
                    *(a + i * NCA + j)= drand48();
                }
            }
            for (i=0; i<NRA; i++) {
                for (j=0; j<NCA; j++) {
                    *(b + i * NCA + j)= drand48();
                }
            }
            averow = NRA/numworkers;
            extra = NRA%numworkers;
            offset = 0;
            mtype = FROM_MASTER;
            elapseTime = MPI_Wtime();
            // Use a for loop to send data to worker tasks
            for (dest=1; dest<=numworkers; dest++) {
                rows = (dest <= extra) ? averow+1 : averow;
	            printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
                MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(a + offset * NCA + 0, rows*NCA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
                MPI_Send(b, NRA*NCA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
                offset = offset + rows;
            }
            mtype = FROM_WORKER;
            // Use a for loop to receice data from workers tasks after they finish.
            for (i=1; i<=numworkers; i++) {
                source = i;
                MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
                MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
                MPI_Recv(c + offset * NRA + 0, rows*NRA, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
            }
            elapseTime = MPI_Wtime()-elapseTime;
            printf("elapseTime is %f s\n", elapseTime);
 	    }

 	    if (taskid > MASTER) {
            // Worker tasks
            mtype = FROM_MASTER;
            // Receive data from master task
            MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(b, NRA*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);


            // Calculation
            for (k=0; k<NRA; k++) {
                for (i=0; i<rows; i++) {
                    *(c + i *NRA + k) = 0.0;
                    for (j=0; j<NCA; j++){
                        *(c + i * NRA + k) += *(a + i * NCA + j) * (*(b + k * NCA  + j));
                    }
                }
            }
            mtype = FROM_WORKER;

            // Send the results back to master
            MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
            MPI_Send(c, rows*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
        }
        MPI_Finalize();
    }
}
