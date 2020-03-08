#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>


#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

int main (int argc, char *argv[])
{
   int	numtasks,              /* number of tasks in partition */
	      taskid,                /* a task identifier */
	      numworkers,            /* number of worker tasks */
	      source,                /* task id of message source */
   	   dest,                  /* task id of message destination */
	      mtype,                 /* message type */
	      rows,                  /* rows of matrix A sent to each worker */
         columns,
	      averow, extra, offset, /* used to determine rows sent to each worker */
	      i, j, k, rc;           /* misc */
   double total_time = 0;
   double t = 0;
   int NRA = atoi(argv[1]), NCA = atoi(argv[2]); 


   double	*a = (double *)malloc(NRA * NCA * sizeof(double)),           /* matrix A to be multiplied */
	         *b = (double *)malloc(NRA * NCA * sizeof(double)),           /* matrix B to be multiplied */
	         *c = (double *)malloc(NRA * NRA * sizeof(double));           /* result matrix C */
   int choice = atoi(argv[3]);
   MPI_Status status;
   double test = 0;

         MPI_Init(&argc,&argv);
         MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
         MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
         printf("%d\n", numtasks);
         
         if (numtasks < 2 ) {
             printf("Need at least two MPI tasks. Quitting...\n");
             MPI_Abort(MPI_COMM_WORLD, rc);
             exit(1);
            }
         numworkers = numtasks-1;

         


/**************************** master task ************************************/
if(choice == 0)
{         if (taskid == MASTER){
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
            for (dest=1; dest<=numworkers; dest++){
               rows = (dest <= extra) ? averow+1 : averow;   	
               printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
               MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
               MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
               MPI_Send(a + offset * NCA + 0, rows*NCA, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
               MPI_Send(b, NCA*NRA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
               offset = offset + rows;
            }

      /* Receive results from worker tasks */
            mtype = FROM_WORKER;
            for (i=1; i<=numworkers; i++){
               source = i;
               MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(c + offset * NRA + 0, rows*NRA, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(&total_time,1, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
               t += total_time;
               printf("Received time from task %d is %lf\n",source, total_time);
               printf("Received results from task %d\n",source);
            }

      /* Print total time of computation */
            if(source == numworkers){
               printf("The total time of computation is %lf seconds \n", t);

            }

     }


/**************************** worker task ************************************/
     if (taskid > MASTER){
         mtype = FROM_MASTER;
         MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(a, rows*NCA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(b, NCA*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
         double t1 = MPI_Wtime();
         printf("The choice is %d", choice);
         for (k=0; k<NRA; k++)
            for (i=0; i<rows; i++)
            {
               *(c + i *NRA + k) = 0.0;
               for (j=0; j<NCA; j++)
                  *(c + i * NRA + k) +=  *(a + i * NCA + j) * (*(b + j * NRA  + k));
            }
         mtype = FROM_WORKER;
         double t2 = MPI_Wtime();
         total_time += (t2 - t1);
         MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(c, rows*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(&total_time,1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      }}
else if (choice == 1) /* column major*/
{         if (taskid == MASTER){
            for (i=0; i<NCA; i++)
               for (j=0; j<NRA; j++)
                  *(a + i* NRA + j)= drand48();
            for (i=0; i<NRA; i++)
               for (j=0; j<NCA; j++)
                  *(b + i * NCA + j)= drand48();

      /* Send matrix data to the worker tasks */
            averow = NCA/numworkers;
            extra = NCA%numworkers;
            offset = 0;
            mtype = FROM_MASTER;
            for (dest=1; dest<=numworkers; dest++){
               columns = (dest <= extra) ? averow+1 : averow;   	
               printf("Sending %d columns to task %d offset=%d\n",columns,dest,offset);
               MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
               MPI_Send(&columns, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
               MPI_Send(a + offset * NRA + 0, columns*NRA, MPI_DOUBLE, dest, mtype,MPI_COMM_WORLD);
               MPI_Send(b, NCA*NRA, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
               offset = offset + columns;
            }

      /* Receive results from worker tasks */
            mtype = FROM_WORKER;
            for (i=1; i<=numworkers; i++){
               source = i;
               MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(&columns, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(c + offset * NRA + 0, columns*NRA, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
               MPI_Recv(&total_time,1, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
               t += total_time;
               printf("Received time from task %d is %lf\n",source, total_time);
               printf("Received results from task %d\n",source);
            }

      /* Print total time of computation */
            if(source == numworkers){
               printf("The total time of computation is %lf seconds \n", t);

            }

     }


/**************************** worker task ************************************/
     if (taskid > MASTER){
         mtype = FROM_MASTER;
         MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(&columns, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(a, columns*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
         MPI_Recv(b, NCA*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
         double t1 = MPI_Wtime();
     
         for (k=0; k<NCA; k++)
            for (i=0; i<columns; i++)
            {
               *(c + i * NCA  + k ) = 0.0;
               for (j=0; j<NCA; j++)
                  *(c + i * NCA + k) +=  *(a + i * NRA + j) * (*(b + j * NCA  + k));
            }
         mtype = FROM_WORKER;
         double t2 = MPI_Wtime();
         total_time += (t2 - t1);
         MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(&columns, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(c, columns*NRA, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
         MPI_Send(&total_time,1, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);
      }}
      MPI_Finalize();
}