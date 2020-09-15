#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>

FILE *fp_out;
FILE *fp_log;
FILE* fp_state;

#define MAXCOUNT 0x00ffffff

#define STARTUP	5	/* minimum count on start + 1 */
#define MINSTEP	2	/* minimum step size (by experiment) */

void write_gpd(unsigned count, unsigned state)
{
	unsigned gpd = (count<<8) | (state&0x0ff);

	fprintf(fp_log, "write_gpd %8u %02x %08x\n", count, state, gpd);

	fwrite(&gpd, sizeof(unsigned), 1, fp_out);
	if (fp_state){
		fwrite(&state, sizeof(unsigned), 1, fp_state);
	}
}

long expand_state(unsigned state, long until_count)
{
	static unsigned count0;

	if (until_count < MAXCOUNT){
		fprintf(fp_log, "expand_state() %d\n", __LINE__);
		if (until_count-count0 < MINSTEP){
			fprintf(fp_log, "ENFORCING MINSTEP %d\n", MINSTEP);
			until_count = count0 + MINSTEP;
		}
		write_gpd(until_count, state);
	}else{
		unsigned remain = until_count - count0;
		unsigned ontheclock = count0&MAXCOUNT;		/* what's already on the clock */
		if (remain < MINSTEP){
			fprintf(fp_log, "ENFORCING MINSTEP %d\n", MINSTEP);
			remain = MINSTEP;
		}
		if (MAXCOUNT-ontheclock > remain){
			fprintf(fp_log, "expand_state() %d\n", __LINE__);
			write_gpd(count0+remain, state);
		}else{
			fprintf(fp_log, "expand_state() %d\n", __LINE__);
			write_gpd(MAXCOUNT, state);
			remain -= (MAXCOUNT+1)-ontheclock;
			
			while(remain > MAXCOUNT){
				fprintf(fp_log, "expand_state() %d\n", __LINE__);
				write_gpd(MAXCOUNT, state);
				remain -= MAXCOUNT+1;
			}
			fprintf(fp_log, "expand_state() %d\n", __LINE__);
			write_gpd(remain, state);
		}
	}

	count0 = until_count;
	return until_count;
}

#define NSTATE 1
#define MAXSTATE 128	/* hardware limit */


int FINAL = MINSTEP;		/* final state length */

void prompt(int state_count){
	printf("%d ok>\n", state_count);
	fflush(stdout);
}

int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	int nstate = 0;
	int nl = 0;
	int state_count = 0;
	unsigned state0 = 0;

	if (getenv("FINAL")) FINAL = atoi(getenv("FINAL"));
	if (getenv("STL2GPG_LOG")){
		fp_log = fopen(getenv("STL2GPG_LOG"), "w");
		assert(fp_log);
	}else{
		fp_log = stderr;
	}
	if (argc > 1){
		fp_out = fopen(argv[1], "w");
		if (fp_out == 0){
			perror("failed to open outfile");
			return -1;
		}
		if (argc > 2){
			fp_state = fopen(argv[2], "w");
			if (fp_out == 0){
				perror("failed to open state file");
				return -1;
			}
		}
		snprintf(aline, 128, "/tmp/%s", basename(argv[1]));
		fp_log = fopen(aline, "w");
	}else{
		fp_out = stdout;
	}
	for (; fgets(aline, 128, stdin) && ++nl; prompt(state_count)){
		unsigned count;
		unsigned state;
		char* pline = aline;

		if (fp_log) {
			fputs(aline, fp_log);
		}
		if (aline[0] == '#' || strlen(aline) < 2){
			continue;
		}else if (strstr(aline, "EOFLOOP")){
			fprintf(stderr, "quit on EOFLOOP\n");
			return 0;
		}else if (strstr(aline, "EOF")){
			fprintf(stderr, "quit on EOF\n");
			break;
		}else if (aline[0] == '+'){
			pline = aline + 1;
			delta_times = 1;	/* better make them all delta */
		}
		/* scan two numbers. IGNORE any trailing data same line */
		if ((nstate = sscanf(pline, "%u,%x", &count, &state) - 1) >= 1){
			if (state_count+1 >= MAXSTATE-1){
				fprintf(stderr, "WARNING: state count limit %u exceeded\n", MAXSTATE);
				break;
			}
			if (state_count++ == 0){
				if (count < STARTUP){
					fprintf(stderr, "STARTUP min count %d\n", STARTUP);
					count = STARTUP;
				}
				if (delta_times){
					count -= 1;	/* first count from zero */
				}
			}
			/* abs times: all counts from zero */
			abs_count = expand_state(state0,
					delta_times? abs_count+count: count-1);
			state0 = state;
		}else{
			fprintf(stderr, "scan failed\n");
			return -1;
		}
	}

	abs_count = expand_state(state0, 
                                delta_times? abs_count+FINAL: abs_count+FINAL);

	fprintf(stderr, "return 0\n");
	return 0;
}
