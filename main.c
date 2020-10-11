
#include <stdlib.h>
#include <stdio.h>
	 
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "pvm3.h"

#define GRPNAME "Gauss"


void matrix_pload( char nom[], double* tab, int N, int NPROC, int me, int * tids) {
	int i,j;
	if ( me == 0 ){		//si je suis main
		FILE *f;
		if ((f = fopen (nom, "r")) == NULL) {
			perror ("matrix_load : fopen ");
			printf("THEREIS AN ERRORE\n");
			fflush(0);
		}	//ouvre le fichier


		for ( int i=0; i<N; i++ ) {		//pour chaque ligne i de la matrice M
			if ( i%NPROC==me ){				//si c'est une ligne pour moi
				for ( int j=0; j<N; j++ ) {		//pour chaque colonne j 
					fscanf (f, "%lf", (tab + i*N/NPROC+j));		//store l'element Mij dans tab local: ligne i/NPROC, colonne j
				}
			}
			else{		//si c'est une ligne pour un autre 
				double buff[N];  //prepare un tableau à envoyer
				pvm_initsend( PvmDataDefault ); //prepare a envoyer
				for ( int j=0; j<N; j++ ) {		//pour chaque colonne j 
					fscanf (f, "%lf", &(buff[j]));		//store l'element Mij dans le tableau temporaire: index j
				}
				pvm_pkdouble(buff, N, 1 );  //prepare to send N doubles with stride 1
				pvm_send( tids[i%NPROC], 1 );  //dest i%NPROC and tag 1 
			}
		}
		fclose (f);
	}
	else{	// si je ne suis pas main
		for ( int i=0; i<N/NPROC; i++ ) {	//pour chaque ligne i de mon tableau tab
			pvm_recv(-1, -1);  //recois du main tag 1
			pvm_upkdouble( tab+i*N, N, 1 ); //store le tableau de N element stride 1 à la ligne i du tableau tab
		}
	}
}

void matrix_psave(char nom[], double *tab, int N, int NPROC, int me, int *tids)
{
	//save
	if (me == 0){	//si je suis main
		FILE *f;
		double buff[N];
		if ((f = fopen (nom, "w")) == NULL) {
			perror ("matrix_save : fopen ");
			printf("THEREIS AN ERRORE IN LOADE\n");
			fflush(0);
		}
		for (int i=0; i<N; i++) {	//pour chaque ligne i
			if (i%NPROC == 0){	//si c'est le tour du main
				//saving tab
				for (int j=0; j<N; j++) {	//pour chaque colonne j
					fprintf (f, "%8.2f ", *(tab+i*N/NPROC+j) );	//ecrit la valeur du tableau en i,j
				}
				fprintf (f, "\n");	//termine la ligne
			}
			else{			
				pvm_recv( tids[i%NPROC], -1);		//reçois du i%NPROC process le msg avec id i/NPROC 
				pvm_upkdouble( buff, N, 1 );
				for (int j=0; j<N; j++) {
					fprintf (f, "%8.2f ", buff[j] );
				}
				fprintf (f, "\n");
			}
		}
		fclose (f);
	}
	else{	//si je ne suis pas main
		for(int i=0; i<N/NPROC; i++){
			pvm_initsend( PvmDataDefault );
			pvm_pkdouble( tab+N*i, N, 1 );
			pvm_send( tids[0], i );	//envoie ma ligne i de tableau avec le tag i
		}
	}					  
}

void matrix_load ( char nom[], double *tab, int N ) {
	FILE *f;
	int i,j;

	if ((f = fopen (nom, "r")) == NULL) { perror ("matrix_load : fopen "); } 
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			fscanf (f, "%lf", (tab+i*N+j) );
		}
	}
	fclose (f);
}

void matrix_save ( char nom[], double *tab, int N ) {
	FILE *f;
	int i,j;

	if ((f = fopen (nom, "w")) == NULL) { perror ("matrix_save : fopen "); } 
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			fprintf (f, "%8.2f ", *(tab+i*N+j) );
		}
		fprintf (f, "\n");
	}
	fclose (f);
}

void matrix_display ( double *tab,int  N ) {
	int i,j;

	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			printf ("%8.2f ", *(tab+i*N+j) );
		}
		printf ("\n");
	}
}

void gauss ( double * tab, int N ) {
	int i,j,k;
	double pivot;

	for ( k=0; k<N-1; k++ ){ /* mise a 0 de la col. k */
		/* printf (". "); */
		if ( fabs(*(tab+k+k*N)) <= 1.0e-11 ) {
			printf ("ATTENTION: pivot %d presque nul: %g\n", k, *(tab+k+k*N) );
			exit (-1);
		}
		for ( i=k+1; i<N; i++ ){ /* update lines (k+1) to (n-1) */
			pivot = - *(tab+k+i*N) / *(tab+k+k*N);
			for ( j=k; j<N; j++ ){ /* update elts (k) - (N-1) of line i */
				*(tab+j+i*N) = *(tab+j+i*N) + pivot * *(tab+j+k*N);
			}
		/* *(tab+k+i*N) = 0.0; */
		}
	}
	printf ("\n");
}

void pgauss ( double * tab, int N, int NPROC, int me, int *tids ) {
	int i,j,k;
	double * ligne_pivot = malloc(N*sizeof(double));
	double pivot;

	for ( k=0; k<N-1; k++ ){ /* mise a 0 de la col. k */
		/* printf (". "); */
		if(me==k%NPROC){//si c'est à mon tour de partager ma ligne pivot
			memcpy(ligne_pivot, tab+(k/NPROC)*N, N*sizeof(double));	
			pvm_initsend( PvmDataDefault );
			pvm_pkdouble( ligne_pivot, N, 1 );	//partage le ligne entiere
			//int info = pvm_cast( tids, NPROC, k ); //envoie le message id k aux autres process
			int info = pvm_bcast(GRPNAME, k);
		}
		else{ //si ce n'est pas à mon tour de partager le pivot
			pvm_recv( tids[k%NPROC], -1);	//reçois un msg du k%NPROC process id k
			pvm_upkdouble( ligne_pivot , N, 1 );
		}
		//partie calcul
		if ( fabs(*(ligne_pivot+k)) <= 1.0e-11 ) {
			//printf ("ATTENTION: pivot %d presque nul: %g\n", k, *(tab+k+k*N) );
			//fflush(0);
			exit (-1);
		}
		//printf("PTDR:, i:%d, me:%d, k:%d\n", i, me, k);
		//fflush(0);

		for ( i = (k+NPROC-me)/NPROC ; i<N/NPROC ; i ++){ 

			pivot = *(tab+k+i*N) / *(ligne_pivot+k);
			for ( j=k; j<N; j++ ){  //pour les j rien à changer
				*(tab+j+i*N) = *(tab+j+i*N) - pivot * *(ligne_pivot+j);
			}
			*(tab+k+i*N) = 0.0; //on sait que la colonne k sera à 0
		}
	}
	free(ligne_pivot);
	printf ("\n");
}

int main (int argc, char **argv) {
	char nom[255];
	strcpy(nom, argv[2]);
	int NPROC;
	int my_tid;
	int me;
	int N = atoi(argv[1]);
	int *tids;
	struct timeval tv1, tv2;	/* for timing */
	int duree;



	printf("TEST\n");
	fflush(0);
	
	my_tid = pvm_mytid();
	NPROC = pvm_siblings( &tids );
	me = pvm_joingroup( GRPNAME );
	

	printf("NPROC %d \n", NPROC);
	fflush(0);

	printf("name: %s \nsize %d\n", nom, N);
	fflush(0);

	pvm_barrier( GRPNAME, NPROC );
	pvm_freezegroup ( GRPNAME, NPROC );
	for (int i = 0; i < NPROC; i++) tids[i] = pvm_gettid ( GRPNAME, i);

	double * tab = (double *)malloc ( N/NPROC*N*sizeof(double));	//assume que N est divisible par NPROC

	matrix_pload(nom, tab, N, NPROC, me, tids);

	gettimeofday( &tv1, (struct timezone*)0 );
	pgauss (tab, N, NPROC, me, tids);
	gettimeofday( &tv2, (struct timezone*)0 );
  	duree = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;

	matrix_psave("matrix_result", tab, N, NPROC, me, tids);
  	printf ("computation time: %10.8f sec.\n", duree/1000000.0 );

	pvm_lvgroup( GRPNAME );
	free(tab);
	pvm_exit();
}
