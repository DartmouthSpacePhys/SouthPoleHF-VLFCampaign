#include<complex.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<fftw3.h>
#include<unistd.h>
#include<time.h>
#include<stdint.h>

#define TIME_OFFSET 946684800

#pragma pack(push,2)

struct time_stamp {
uint32_t t_sec;
uint32_t t_usec;
};
#pragma pack(pop)

int main (int argc, char *argv[]){
	fftw_plan ForwardPlan, InversePlan;
	fftw_complex *out;
	short unsigned int *data,*OutData;
	double *DataDouble,UpperFreq,avgData;
	long int N, i, j,k, l,iSize,N2,CutOffIndex,avg;
	FILE *FileIn, *FileOut,*SummaryOut, *DetailOut;
	double size,SampleFreq,CurrentFreq;
	struct time_stamp CurrentTime;
	struct tm *DateAndTime;
	unsigned flags = FFTW_MEASURE|FFTW_DESTROY_INPUT;
	time_t time_2;
	N = 131072;

	

	SampleFreq = 2.5e6;

	//Create arrays
	data = (unsigned short int*)malloc(N * sizeof(unsigned short int));
	out = (fftw_complex*)fftw_malloc(N * sizeof(fftw_complex));
	DataDouble = (double*)malloc(N * sizeof(double));

	//Open the file
	FileIn = fopen(argv[argc - 1],"r");
//	FileOut = fopen("DecimatedData.data","w");
	DetailOut = fopen("detail.graydata","w");

	//Plan the transform with threads
	//int fftw_init_threads(void);
	int ret;
	ret = fftw_init_threads(); 
	if (ret ==0 ) { printf("Failed to initialize FFTW threads!\n");}
	printf("Planning FFT!\n");
	fftw_plan_with_nthreads(4);
	ForwardPlan = fftw_plan_dft_r2c_1d(N,DataDouble,out,flags);
	InversePlan = fftw_plan_dft_c2r_1d(N,out,DataDouble,flags);


	//Get the filesize
	fseek(FileIn,-1,SEEK_END);
	iSize = (long int)ftello(FileIn);
	fseek(FileIn,0, SEEK_SET);

	//Read the timestamp

	fread(&CurrentTime,sizeof(CurrentTime),1,FileIn);
	//fwrite(&CurrentTime,sizeof(CurrentTime),1,FileOut);

	printf("time = %i\n",CurrentTime.t_sec);
	//Big loop over the file, while the current position
	printf("Looping!"); 
	long int counter;
	int index;
	double sum, psd;
	counter = 0;
	while (ftello(FileIn) < iSize - 8){
		//Read in the data
		N2 = fread(data, 2, N, FileIn);
	
		
		for (j = 0; j < N; j++){
			DataDouble[j] = (double)data[j];
		}
		fftw_execute(ForwardPlan);
			
		
		//Get the time and write to output file
		time_2 = (CurrentTime.t_sec + TIME_OFFSET);
		DateAndTime = gmtime(&time_2);
	//	printf("Month = %i\n",DateAndTime->tm_mon);
		fprintf(DetailOut, "%i/%i %02i:%02i:%02i.%i\n", 1 + DateAndTime->tm_mon, DateAndTime->tm_mday, DateAndTime->tm_hour, DateAndTime->tm_min,DateAndTime->tm_sec,CurrentTime.t_usec);

		//Average the FFT'd data, write to the output file
		for (i=0; i < N/16 ;i++){
			sum = 0.0;
			index = i * 8;
			for (j = 0; j < 8; j++){
				sum += cabs(out[index + j]);
			}
			
			psd = 20.0 * log10(sum/8.0);
			fprintf(DetailOut,"%lf\n",psd);	
		}
		


		//Update the timestamp and skip to the next second
		CurrentTime.t_sec += 2.0;
		fseek(FileIn,2.0 * (SampleFreq * 2 - N),SEEK_CUR);	
		counter +=1 ;	
	}


	printf("Program finished\n!");


}
