
/* main.c
	*
	* Author: Ali Al Rubh
	* Date  : 21 Mar. 2025
	*
*/

/* Set features         */
#define _GNU_SOURCE

/* Standard C includes  */
/*  -> Standard Library */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
/*  -> Scheduling       */
#include <sched.h>
/*  -> Types            */
#include <stdbool.h>
#include <inttypes.h>
/*  -> Runtimes         */
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/ref.h"
#include "impl/naive.h"
#include "impl/opt.h"
#include "impl/vec.h"
#include "impl/para.h"

/* Include common headers */
#include "common/types.h"
#include "common/macros.h"

/* Include application-specific headers */
#include "include/types.h"

const int SIZE_DATA = 8 * 1024 * 1024;

void readCSVFile(FILE* file, float* array, int size);
void printHelpMenu(const char* arg, int nthreads, int cpu, char* data_str, int dump, int nruns, int nstdevs);

int main(int argc, char** argv) {

	/* Set the buffer for printf to NULL */
	setbuf(stdout, NULL);

	/* Arguments */
	int nthreads = 1;
 	int cpu      = 0;

	int nruns    = 10000;
	int nstdevs  = 3;

	int dump 	= 0;
	int n = 2500;
	int m = 3000;
	int p = 2100;

	float delta = 0.5;

	/* Data */
	int data_size = SIZE_DATA;
	char* data_str = "native";

	/* Parse arguments */
	/* Function pointers */
	void* (*impl_scalar_naive_ptr)(void* args) = impl_scalar_naive;
	void* (*impl_scalar_opt_ptr  )(void* args) = impl_scalar_opt;
	void* (*impl_vector_ptr      )(void* args) = impl_vector;
	void* (*impl_parallel_ptr    )(void* args) = impl_parallel;

	/* Chosen */
	void* (*impl)(void* args) = NULL;
	const char* impl_str      = NULL;

	bool help = false;

	/* Collect Arguments */
	for (int i = 1; i < argc; i++) {
		/* Implementations */
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {

			assert (++i < argc);

			if (strcmp(argv[i], "naive") == 0) {
				impl = impl_scalar_naive_ptr; impl_str = "scalar_naive";

			} else if (strcmp(argv[i], "opt"  ) == 0) {
				impl = impl_scalar_opt_ptr  ; impl_str = "scalar_opt"  ;

			} else if (strcmp(argv[i], "vec"  ) == 0) {
				impl = impl_vector_ptr      ; impl_str = "vectorized"  ;

			} else if (strcmp(argv[i], "para" ) == 0) {
				impl = impl_parallel_ptr    ; impl_str = "parallelized";

			} else {
				impl = NULL                 ; impl_str = "unknown"     ;

			}
			continue;
		}

		/* Input/output data size */
		if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0) {
			assert (++i < argc);
			if (strcmp(argv[i], "testing") == 0) {
				data_size = 256;
				data_str = "testing";
				n = 16; m = 12; p = 8;

                        } else if (strcmp(argv[i], "small"  ) == 0) {
				data_size = 32 * 1024;
				data_str = "small";
				n = 121; m = 180; p = 115;

                        } else if (strcmp(argv[i], "medium"  ) == 0) {
				data_size = 1024 * 1024;
				data_str = "medium";
				n = 550; m = 620; p = 480;

                        } else if (strcmp(argv[i], "large" ) == 0) {
				data_size = 2 * 1024 * 1024;
				data_str = "large";
				n = 962; m = 1012; p = 1221;

                        } else {
                                data_size = 8 * 1024 * 1024;
				data_str = "native";
				n = 2500; m = 3000; p = 2100;

			}

			continue;
		}

		/* Run parameterization */
		if (strcmp(argv[i], "--nruns") == 0) {
			assert (++i < argc);
			nruns = atoi(argv[i]);

			continue;
		}

		if (strcmp(argv[i], "--nstdevs") == 0) {
			assert (++i < argc);
			nstdevs = atoi(argv[i]);

			continue;
		}

		/* Parallelization */
		if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nthreads") == 0) {
			assert (++i < argc);
			nthreads = atoi(argv[i]);

			continue;
		}

		if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cpu") == 0) {
			assert (++i < argc);
			cpu = atoi(argv[i]);

			continue;
		}

		/* Dump Files */
		if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dump") == 0) {
			dump = 1;

			continue;
		}

		/* Help */
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			help = true;

			continue;
		}
	}

	/* Help Menu */
  	if (help || impl == NULL) {
		if (!help) {
      			if (impl_str != NULL) {
        			printf("\nERROR: Unknown \"%s\" implementation.\n", impl_str);
      			} else {
        			printf("\nERROR: No implementation was chosen.\n");
			}
		}

		printHelpMenu(argv[0],nthreads,cpu,data_str,dump,nruns,nstdevs);
	    	exit(help? 0 : 1);
  	}

	/* Set our priority the highest */
	int nice_level = -20;

	printf("Setting up schedulers and affinity:\n");
	printf("  * Setting the niceness level:\n");

	do {
		errno = 0;
		printf("      -> trying niceness level = %d\n", nice_level);
		int __attribute__((unused)) ret = nice(nice_level);
	} while (errno != 0 && nice_level++);

	printf("    + Process has niceness level = %d\n", nice_level);


	/* If we are on an apple operating system, skip the scheduling  *
	* routine; Darwin does not support sched_set* system calls ... *
	*                                                              *
	* hawajkm: and here I was--thinking that MacOS is POSIX ...    *
	*          Silly me!                                           */


	#if !defined(__APPLE__)
		/* Set scheduling to reduce context switching */
		/*    -> Set scheduling scheme                */
		printf("  * Setting up FIFO scheduling scheme and high priority ... ");
		pid_t pid    = 0;
		int   policy = SCHED_FIFO;
		struct sched_param param;

		param.sched_priority = sched_get_priority_max(policy);
		int res = sched_setscheduler(pid, policy, &param);
		if (res != 0) {
			printf("Failed\n");
		} else {
			printf("Succeeded\n");
		}

		/*    -> Set affinity                         */
		printf("  * Setting up scheduling affinity ... ");
		cpu_set_t cpumask;

		CPU_ZERO(&cpumask);
		for (int i = 0; i < nthreads; i++) {
			CPU_SET(cpu + i, &cpumask);
		}

		res = sched_setaffinity(pid, sizeof(cpumask), &cpumask);

		if (res != 0) {
			printf("Failed\n");
		} else {
			printf("Succeeded\n");
		}
	#endif
	printf("\n");

	/* Statistics */
	__DECLARE_STATS(nruns, nstdevs);

	/* Initialize Rand */
	srand(0xdeadbeef);

	/* Datasets */
	/* Allocation and initialization */

	/* Input Matrices */
	float* a = __ALLOC_DATA(float, data_size + 0);
	float* b = __ALLOC_DATA(float, data_size + 0);

	/* Output matrix */
	float* c = __ALLOC_DATA(float, data_size + 4);

	/* Since reference is already computed, no need for guard bit */
	float* r = __ALLOC_DATA(float, data_size + 0);

	/* Setting a guards, which is 0xdeadcafe.
	The guard should not change or be touched. */
	__SET_GUARD(c , 4*data_size);
	/* Establishin filenames */
        char golden_file_name[256];
	char test_file_name[256];
        strcpy(golden_file_name, data_str);
        strcat(golden_file_name, "_golden.csv");

        strcpy(test_file_name, data_str);
        strcat(test_file_name, "_test.csv");

	printf("Golden file name is %s\n", golden_file_name);
	printf("Test file name is %s\n", test_file_name);

	/* Reading Reference Data */
	FILE *file = fopen(golden_file_name, "r");

	if (!file) {
        	perror("Error opening refernce file\n");
        	return 1;
    	}

	printf("Golden file opened, attempting read...\n");
	readCSVFile(file,r, n * p);
	fclose(file);
	printf("Read of golden file succsessful\n");

	/* Reading Input Data */
	file = fopen(test_file_name, "r");

	if (!file) {
		perror("Error opening input file\n");
		return 1;
	}
	printf("Test file opened, attempting read...\n");
	printf("Reading a..\n");
	readCSVFile(file, a, n*m);
	printf("Reading b..\n");
	readCSVFile(file, b, m*p);
	fclose(file);
	printf("Test file read sucsessfully.\n");
	/* Execute the requested implementation */
	/* Arguments for the function */
	args_t args;


	args.inputA	= a;
	args.inputB	= b;
	args.output	= c;

	args.n		= n;
	args.m		= m;
	args.p		= p;

	args.size = data_size;

	args.cpu      = cpu;
	args.nthreads = nthreads;

	/* Start execution */
	printf("Running \"%s\" implementation:\n", impl_str);

	printf("  * Invoking the implementation %d times .... ", num_runs);
	for (int i = 0; i < num_runs; i++) {
		__SET_START_TIME();
		for (int j = 0; j < 16; j++) {
			(*impl)(&args);
		}
		__SET_END_TIME();
		runtimes[i] = __CALC_RUNTIME() / 16;
	}

	printf("Finished\n");

	/* Verfication */
	printf("  * Verifying results .... ");

	bool match = __CHECK_FLOAT_MATCH(r, c, data_size,delta);
	bool guard = __CHECK_GUARD(c, 4* data_size);

	if (match && guard) {
		printf("Success\n");

	} else if (!match && guard) {
		printf("Fail, but no buffer overruns\n");

	} else if (match && !guard) {
		printf("Success, but failed buffer overruns check\n");

	} else if(!match && !guard) {
		printf("Failed, and failed buffer overruns check\n");

	}

	/* Running analytics */
	double min     = -1;
	double max     =  0;

	double avg     =  0;
	uint64_t avg_n   =  0;

	double std     =  0;
	uint64_t std_n   =  0;

	int      n_msked =  0;
	int      n_stats =  0;

	for (int i = 0; i < num_runs; i++)
		runtimes_mask[i] = true;

	printf("  * Running statistics:\n");

	do {
		n_stats++;
		printf("    + Starting statistics run number #%d:\n", n_stats);
		avg_n =  0;
		avg   =  0;

		/*   -> Calculate min, max, and avg */
		for (int i = 0; i < num_runs; i++) {
			if (runtimes_mask[i]) {
				if (runtimes[i] < min) {
					min = runtimes[i];
				}

				if (runtimes[i] > max) {
					max = runtimes[i];
				}

				avg += runtimes[i];
				avg_n += 1;
			}
		}

		avg = avg / avg_n;

		/*   -> Calculate standard deviation */
		std   =  0;
		std_n =  0;

		for (int i = 0; i < num_runs; i++) {
			if (runtimes_mask[i]) {
				std   += ((runtimes[i] - avg) * (runtimes[i] - avg));
				std_n += 1;
			}
		}

		std = sqrt(std / std_n);

		/*   -> Calculate outlier-free average (mean) */
		n_msked = 0;

		for (int i = 0; i < num_runs; i++) {
			if (runtimes_mask[i]) {
				if (runtimes[i] > avg) {
					if ((runtimes[i] - avg) > (nstd * std)) {
						runtimes_mask[i] = false;
						n_msked += 1;
					}
				}
				else {
					if ((avg - runtimes[i]) > (nstd * std)) {
						runtimes_mask[i] = false;
						n_msked += 1;
					}
				}
			}
		}

		printf("      - Standard deviation = %lf\n", std);
		printf("      - Average = %lf\n", avg);
		printf("      - Number of active elements = %" PRIu64 "\n", avg_n);
		printf("      - Number of masked-off = %d\n", n_msked);

	} while (n_msked > 0);

	/* Display information */
	printf("  * Runtimes (%s): ", __PRINT_MATCH(match));
	printf(" %lf ns\n"  , avg                 );

	/* Output dump */
	if (dump == 1) {
		file;
		char name[256];

		strcpy(name, data_str);
		strcat(name, "_dump.csv");

		printf("    - Filename: %s\n", name);
		printf("    - Opening file .... ");

		file = fopen(name, "w");

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < p; j++) {
				fprintf(file,"%lf,", c[i*p + j]);
			}
		}
		printf("Writing output dump complete\n");
		fclose(file);
	}
	/* Runtime dump */
	printf("  * Dumping runtime informations:\n");

	FILE * fp;
	char filename[256];

	strcpy(filename, impl_str);
	strcat(filename, "_runtimes.csv");

	printf("    - Filename: %s\n", filename);
	printf("    - Opening file .... ");

	fp = fopen(filename, "w");

	if (fp != NULL) {
		printf("Succeeded\n");
		printf("    - Writing runtimes ... ");
		fprintf(fp, "impl,%s", impl_str);

		fprintf(fp, "\n");
		fprintf(fp, "num_of_runs,%d", num_runs);

		fprintf(fp, "\n");
		fprintf(fp, "runtimes");
		for (int i = 0; i < num_runs; i++) {
			fprintf(fp, ", ");
			fprintf(fp, "%" PRIu64 "", runtimes[i]);
		}

		fprintf(fp, "\n");
		fprintf(fp, "avg,%lf", avg);
		printf("Finished\n");
		printf("    - Closing file handle .... ");
		fclose(fp);
		printf("Finished\n");
	} else {
		printf("Failed\n");
	}

	printf("\n");

	/* Manage memory */
	free(a);
	free(b);
	free(c);
	free(r);

	/* Finished with statistics */
	__DESTROY_STATS();

	/* Done */
	return 0;
}


void printHelpMenu(const char* arg, int nthreads, int cpu, char* data_str, int dump, int nruns, int nstdevs) {
	    printf("\n");
	    printf("Usage:\n");
	    printf("  %s {-i | --impl} impl_str [Options]\n", arg);
	    printf("  \n");
	    printf("  Required:\n");
	    printf("    -i | --impl      Available implementations = {naive, opt, vec, para}\n");
	    printf("    \n");
	    printf("  Options:\n");
	    printf("    -h | --help      Print this message\n");
	    printf("    -n | --nthreads  Set number of threads available (default = %d)\n", nthreads);
	    printf("    -c | --cpu       Set the main CPU for the program (default = %d)\n", cpu);
	    printf("    -s | --size      Size of input and output data (default = %s), Available implementations: {testing, small, medium, large,native}\n", data_str);
	    printf("    -d | --dump      Enable dump files (default = %d)\n", dump);
	    printf("         --nruns     Number of runs to the implementation (default = %d)\n", nruns);
	    printf("         --stdevs    Number of standard deviation to exclude outliers (default = %d)\n", nstdevs);
	    printf("\n");
}

/* This function will be used to read CSV files for inputs as well as reference output */
void readCSVFile(FILE* file, float* array, int size) {
    int i = 0;
    char c;

    if (!file) {
        perror("Error: File is not defined");
        exit(1);
    }

    if (!array) {
        perror("Error: Array is not defined");
        exit(1);
    }

    while (i < size && fscanf(file, "%f", &array[i]) == 1) {
        //printf("%f\n", array[i]);
        i++;
        c = fgetc(file);
        if (c == '\n' || c == EOF) {
            break;
        }
    }
}
