#include <stdio.h>
#include <string.h>
#include <libgen.h>

FILE *fp_out;

long expand_state(unsigned state, long until_count)
{
	unsigned gpd = (until_count<<8) | state;
	fwrite(&gpd, sizeof(unsigned), 1, fp_out);
	return until_count;
}

#define NSTATE 1
#define MAXSTATE	6	/* 6 x DIO432 is max payload */

int main(int argc, char* argv[])
{
	char aline[128];
	int delta_times = 0;
	long abs_count = 0;
	FILE *fp_log = 0;
	int nstate = 0;
	int nstate0 = 0;
	int nl = 0;

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
			if (nstate0){
				if (nstate != nstate0){
					fprintf(stderr, "ERROR: line:%d state count change %d=>%d."
							"Please apply state columns consistently\n",
							nl, nstate0, nstate);
					return -1;
				}
			}else{
				nstate0 = nstate;
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
