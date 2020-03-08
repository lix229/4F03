#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int main (int argc, char *argv[])
{
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
double time;

double	*a = (double *)malloc(NRA * NCA * sizeof(double)),  /* matrix A to be multiplied */
        *b = (double *)malloc(NCA * NRA * sizeof(double)),  /* matrix B to be multiplied */
				*c = (double *)malloc(NRA * NRA * sizeof(double));  /* matrix C to be returned */


MPI_Status status;

MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

printf("The number of tasks is %d\n", numtasks);
if (numtasks < 2 ) {
  printf("Need at least two MPI tasks. Quitting...\n");
  MPI_Abort(MPI_COMM_WORLD, rc);
  exit(1);
  }
numworkers = numtasks-1;


/**************************** master task for row******************************/
if (choice == 0)
	{
	   if (taskid == MASTER)
	   {
	      printf("mpi_mm row has started with %d tasks.\n",numtasks);
	      printf("Initializing arrays...\n");
	      for (i=0; i<NRA; i++)
	         for (j=0; j<NCA; j++)
	            *(a + i * NCA + j)= drand48();
	      for (i=0; i<NCA; i++)
	         for (j=0; j<NRA; j++)
	            *(b + i * NRA + j)= drand48();

	      /* Send matrix data to the worker tasks */
	      averow = NRA/numworkers;
	      extra = NRA%numworkers;
	      offset = 0;
	      mtype = FROM_MASTER;
				time = MPI_Wtime();                           /*start time recording*/
	      for (dest=1; dest<=numworkers; dest++)
	      {
	         rows = (dest <= extra) ? averow+1 : averow;
	         printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
	         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
	         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
	         MPI_Send(a + offset * NCA + 0, rows*NCA, MPI_DOUBLE, dest, mtype,
	                   MPI_COMM_WORLD);
	         MPI_Send(b, NCA*NRA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
	         offset = offset + rows;
	      }

	      /* Receive results from worker tasks */
	      mtype = FROM_WORKER;
	      for (i=1; i<=numworkers; i++)
	      {
	         source = i;
	         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
	         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
	         MPI_Recv(c + offset * NRA + 0, rows*NRA, MPI_DOUBLE, source, mtype,
	                  MPI_COMM_WORLD, &status);
	         printf("Received results from task %d\n",source);
	      }
				time = MPI_Wtime()-time;                      /*time recording end*/
				printf("Computation time is %f s\n", time);

	   }


	/**************************** worker task for row*****************************/
	   if (taskid > MASTER)
	   {
	      mtype = FROM_MASTER;
	      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
	      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
	      MPI_Recv(a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
	      MPI_Recv(b, NCA*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

	      for (k=0; k<NRA; k++)
	         for (i=0; i<rows; i++)
	         {
	            *(c + i *NRA + k) = 0.0;
	            for (j=0; j<NCA; j++){
	               *(c + i * NRA + k) += *(a + i * NCA + j) * (*(b + j * NRA  + k));
							}
	         }
	      mtype = FROM_WORKER;
	      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
	      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
	      MPI_Send(c, rows*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
	   }
	   MPI_Finalize();
	}

	/**************************** master task for column******************************/
	if (choice == 1)
		{
			if (taskid == MASTER)
 	   {
 	      printf("mpi_mm row has started with %d tasks.\n",numtasks);
 	      printf("Initializing arrays...\n");
 	      for (i=0; i<NRA; i++)
 	         for (j=0; j<NCA; j++)
 	            *(a + i * NCA + j)= drand48();
 	      for (i=0; i<NRA; i++)
 	         for (j=0; j<NCA; j++)
 	            *(b + i * NCA + j)= drand48();

 	      /* Send matrix data to the worker tasks */
 	      averow = NRA/numworkers;
 	      extra = NRA%numworkers;
 	      offset = 0;
 	      mtype = FROM_MASTER;
 				time = MPI_Wtime();                           /*start time recording*/
 	      for (dest=1; dest<=numworkers; dest++)
 	      {
 	         rows = (dest <= extra) ? averow+1 : averow;
 	         printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
 	         MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
 	         MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
 	         MPI_Send(a + offset * NCA + 0, rows*NCA, MPI_DOUBLE, dest, mtype,
 	                   MPI_COMM_WORLD);
 	         MPI_Send(b, NRA*NCA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
 	         offset = offset + rows;
 	      }

 	      /* Receive results from worker tasks */
 	      mtype = FROM_WORKER;
 	      for (i=1; i<=numworkers; i++)
 	      {
 	         source = i;
 	         MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
 	         MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
 	         MPI_Recv(c + offset * NRA + 0, rows*NRA, MPI_DOUBLE, source, mtype,
 	                  MPI_COMM_WORLD, &status);
 	         printf("Received results from task %d\n",source);
 	      }
 				time = MPI_Wtime()-time;                      /*time recording end*/
 				printf("Computation time is %f s\n", time);

 	      /* Print results */

 	      // printf("******************************************************\n");
 	      // printf("Result Matrix:\n");
 	      // for (i=0; i<NRA; i++)
 	      // {
 	      //    printf("\n");
 	      //    for (j=0; j<NRA; j++)
 	      //       printf("%6.2f   ", *(c + i * NRA + j));
 	      // }
 	      // printf("\n******************************************************\n");
 	      // printf ("Done.\n");

 	   }


 	/**************************** worker task for column*****************************/
 	   if (taskid > MASTER)
 	   {
 	      mtype = FROM_MASTER;
 	      MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
 	      MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
 	      MPI_Recv(a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
 	      MPI_Recv(b, NRA*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);

 	      for (k=0; k<NRA; k++)
 	         for (i=0; i<rows; i++)
 	         {
 	            *(c + i *NRA + k) = 0.0;
 	            for (j=0; j<NCA; j++){
 	               *(c + i * NRA + k) += *(a + i * NCA + j) * (*(b + k * NCA  + j));
 							}
 	         }
 	      mtype = FROM_WORKER;
 	      MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
 	      MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
 	      MPI_Send(c, rows*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
 	   }
 	   MPI_Finalize();
		}
}
