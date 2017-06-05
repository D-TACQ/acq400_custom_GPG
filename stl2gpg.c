#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

FILE *fp_out;

#define MAXCOUNT 0x00ffffff

void write_gpd(unsigned count, unsigned state)
{
	unsigned gpd = (count<<8) | (state&0x0ff);

	fwrite(&gpd, sizeof(unsigned), 1, fp_out);
}

long expand_state(unsigned state, long until_count)
{
	static unsigned count0;

	if (until_count < MAXCOUNT){
		write_gpd(until_count, state);
	}else{
		unsigned remain = until_count - count0;
		if (MAXCOUNT-count0 > remain){
			write_gpd(count0+remain, state);
		}else{
			write_gpd(MAXCOUNT-count0, state);
			remain -= MAXCOUNT-count0;
			
			while(remain > MAXCOUNT){
				write_gpd(0, state);
				remain -= MAXCOUNT;
			}
			write_gpd(remain, state);
		}
	}

	count0 = until_count;
	return until_count;
}

#define NSTATE 1
#define MAXSTATE 128	/* hardware limit */

int FINAL = 10;		/* final state length */

int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	FILE *fp_log = 0;
	int nstate = 0;
	int nl = 0;
	int state_count = 0;
	unsigned state0 = 0;

	if (getenv("FINAL")) FINAL = atoi(getenv("FINAL"));

	if (argc > 1){
		fp_out = fopen(argv[1], "w");
		if (fp_out == 0){
			perror("failed to open outfile");
			return -1;
		}
		snprintf(aline, 128, "/tmp/%s", basename(argv[1]));
		fp_log = fopen(aline, "w");
	}else{
		fp_out = stdout;
	}
	while(fgets(aline, 128, stdin) && ++nl){
		unsigned count;
		unsigned state;
		char* pline = aline;

		if (fp_log) {
			fputs(aline, fp_log);
		}
		if (aline[0] == '#' || strlen(aline) < 2){
			continue;
		}else if (aline[0] == '+'){
			pline = aline + 1;
			delta_times = 1;	/* better make them all delta */
		}
		if ((nstate = sscanf(pline, "%u,%x", &count, &state) - 1) >= 1){
			if (++state_count >= MAXSTATE-1){
				fprintf(stderr, "WARNING: state count limit %u exceeded\n", MAXSTATE);
				break;
			}
			abs_count = expand_state(state0, 
				delta_times? abs_count+count: count);
			state_count += 1;
			state0 = state;
		}else{
			fprintf(stderr, "scan failed\n");
			return -1;
		}
	}
	abs_count = expand_state(state0, 
                                delta_times? abs_count+FINAL: abs_count+FINAL);
	
}
