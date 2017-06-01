#include <stdio.h>
#include <string.h>
#include <libgen.h>

FILE *fp_out;

long expand_state(unsigned state, long until_count)
{
	unsigned gpd = (until_count<<8) | (state&0x0ff);
	fwrite(&gpd, sizeof(unsigned), 1, fp_out);
	return until_count;
}

#define NSTATE 1
#define MAXSTATE 128	/* hardware limit */

int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	FILE *fp_log = 0;
	int nstate = 0;
	int nl = 0;
	int state_count = 0;

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
			if (++state_count >= MAXSTATE){
				fprintf(stderr, "WARNING: state count limit %u exceeded\n", MAXSTATE);
				break;
			}
			abs_count = expand_state(
				state,
				delta_times? abs_count+count: count);
		}else{
			fprintf(stderr, "scan failed\n");
			return -1;
		}
	}
}
