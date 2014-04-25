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
	avg = 8; //Decimation factor

	

	SampleFreq = 20.0e6;
	UpperFreq = SampleFreq/(double)avg;
	CutOffIndex = ceil(UpperFreq * ( (N/2) + 1) / (SampleFreq/2.0) );
	printf("Zero all frequencies after index %li\n",CutOffIndex);

	//Create arrays
	data = (unsigned short int*)malloc(N * sizeof(unsigned short int));
	out = (fftw_complex*)fftw_malloc(N * sizeof(fftw_complex));
	DataDouble = (double*)malloc(N * sizeof(double));
	OutData = (unsigned short int*)malloc(N/avg * sizeof(unsigned short int));

	//Open the file
	FileIn = fopen(argv[argc - 1],"r");
	FileOut = fopen("DecimatedData","w");
	SummaryOut = fopen("summary.graydata","w");
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
	fwrite(&CurrentTime,sizeof(CurrentTime),1,FileOut);

	printf("time = %i\n",CurrentTime.t_sec);
	//Big loop over the file, while the current position
	printf("Looping!"); 
	long int counter;
	int index;
	double sum, psd, totalSeconds;
	totalSeconds = 0.0;
	counter = 0;
	while (ftello(FileIn) < iSize - 8){
		//Read in the data
		N2 = fread(data, 2, N, FileIn);
	
		
		for (j = 0; j < N; j++){
			DataDouble[j] = (double)data[j];
		}
		//Execute the fft
//		printf("FFT'in!\n");
		fftw_execute(ForwardPlan);
		for (j =0; j< 5; j++){
//			printf("out[%li] = %lf + I*%lf\n",j,creal(out[j]),cimag(out[j]));
		}
//		return 0;
		//for (i = 0; i < N/2 + 1; i++){
			
		//}
		
		//Write to summary and detail files if applicable
		if (counter%7848  == 0){
			//Get the time
			time_2 = (CurrentTime.t_sec + TIME_OFFSET);
			printf("%li\n",time_2);	
			//gmtime_r(&time_2, &DateAndTime);
			DateAndTime = gmtime(&time_2);
			printf("Month = %i\n",DateAndTime->tm_mon);
			fprintf(SummaryOut, "%i/%i %02i:%02i:%02i.%i\n", 1 + DateAndTime->tm_mon, DateAndTime->tm_mday, DateAndTime->tm_hour, DateAndTime->tm_min,DateAndTime->tm_sec,CurrentTime.t_usec);
			for (i=0; i < N/16 ;i++){
				sum = 0.0;
				index = i * 8;
				for (j = 0; j < 8; j++){
					sum += cabs(out[index + j]);
				}
				
				psd = 20.0 * log10(sum/8.0);
				fprintf(SummaryOut,"%lf\n",psd);	

			}
		}
		
		if (counter%1560 == 0){
			time_2 = (CurrentTime.t_sec + TIME_OFFSET);
			DateAndTime = 	gmtime(&time_2);
			fprintf(DetailOut, "%i/%i %02i:%02i:%02i.%i\n",1 + DateAndTime->tm_mon, DateAndTime->tm_mday, DateAndTime->tm_hour, DateAndTime->tm_min,DateAndTime->tm_sec,CurrentTime.t_usec);
			for (i=0;i<N/16; i++){
				sum = 0.0;
				index = i * 8;
				for (j = 0; j < 8; j++){
					sum += cabs(out[index + j]);
				}
				psd = 20.0 * log10(sum/8.0);
				fprintf(DetailOut,"%lf\n",psd);
			}

		}

		//the FFT'd data to zero if is above the cut-off index
		for (i = CutOffIndex; i < N/2 + 1; i++){
			out[i] = 0.0 + 0.0 * I;
			out[i] = 0.0 + 0.0 * I;
			out[i+N/2] = 0.0 + 0.0 * I;
			out[i+N/2] = 0.0 + 0.0 * I;
		}

		//Inverse fft
		fftw_execute(InversePlan);

		//Decimate. The factor of 1/N normalizes the output data.
		for (i = 0; i < N/avg; i++){
			avgData = 0;
			for (j =0; j < avg; j++){
			//	printf("DataDouble[%li] = %lf\n",j,DataDouble[i * avg + j]/N);
				avgData += (1.0 / ((double)avg * (double)N)) * DataDouble[i * avg + j];
			}
			//printf("avgData = %lf\n",avgData);
			//return 0;
		//	printf("avgData = %lf\n",avgData);
			OutData[i] = (short unsigned int)avgData;
		}

		//Write to the output file
		fwrite(OutData,sizeof(short unsigned int),N/8,FileOut);

		//Update the timestamp
		totalSeconds += N * (1.0 / SampleFreq);
		CurrentTime.t_sec += (long int)floor(totalSeconds);
		CurrentTime.t_usec += (long int)floor(1.0e6 * (totalSeconds - floor(totalSeconds)));
		
		//If the total number of microseconds is greater than or equal to 10,0000 increment the timestamp one second
		if (CurrentTime.t_usec >= 10.0e6){
			CurrentTime.t_sec += 1;
			CurrentTime.t_usec -= 10e6;
		}
		counter +=1 ;	
	}

	//Read and write the timestamp
	fread(&CurrentTime,sizeof(CurrentTime),1,FileIn);
	fwrite(&CurrentTime,sizeof(CurrentTime),1,FileOut);

	printf("Program finished\n!");


}
