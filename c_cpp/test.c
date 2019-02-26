#include <stdio.h>
#include <math.h>
#include <string.h>
#include "hmm.h"
//./test modellist.txt testing_data.txt result.txt
int main(int argc, char const *argv[]) {

	HMM hmm_model[100];
	char file[100][64];
	int model = 0;

	FILE *fp = open_or_die(argv[1], "r");

	while(fscanf(fp, "%s", file[model]) != EOF) {
		//build an HMM for every model file to access the new phi, transition, and observation
		loadHMM(&hmm_model[model], file[model]);
		model++;
	}

	fclose(fp);

	FILE *testing_data = open_or_die(argv[2], "r");
	FILE *result = open_or_die(argv[3], "w");
//    FILE *testing_answer = open_or_die("testing_answer.txt", "r");

	char str[MAX_SEQ];
	while(fscanf(testing_data, "%s", str) != EOF) {
		int seq[MAX_SEQ];
		int Time = strlen(str);
		for(int t = 0; t < Time; t++) {
			seq[t] = str[t] - 'A';
		}

		double P = -1.0;
		int result_model_idx = -1;
		//do the viterbi Algorithm for 5 models
		for(int n = 0; n < model; n++) {
			
			double delta[MAX_STATE][MAX_SEQ] = {{0.0}};

			//calcuate delta

			//initialization
			for(int i = 0; i < hmm_model[n].state_num; i++)
				delta[i][0] = hmm_model[n].initial[i] * hmm_model[n].observation[seq[0]][i];


			for(int t = 1; t < Time; t++) {
				for(int j = 0; j < hmm_model[n].state_num; j++) {
					double maximum = -1.0;
					for(int i = 0; i < hmm_model[n].state_num; i++) {
						if(delta[i][t-1] * hmm_model[n].transition[i][j] > maximum) {
							maximum = delta[i][t-1] * hmm_model[n].transition[i][j];
						}
					}

					delta[j][t] = maximum * hmm_model[n].observation[seq[t]][j];
				}
			}


			//termination
			double tmp_P = -1.0;
			//Among the n states, which one has the highest value of delta at time Time-1
			for(int i = 0; i < hmm_model[n].state_num; i++) {
				if(tmp_P < delta[i][Time-1]) {
					tmp_P = delta[i][Time-1];
				}
			}

			//Among the 5 models, which model has the highest value of delta at time Time-1
			if(tmp_P > P) {
				P = tmp_P;
				result_model_idx = n;
			}


		}	//end of nmodel

		fprintf(result, "%s %.6e\n", file[result_model_idx], P);
        
	}	//end of while
    
	fclose(result);
	fclose(testing_data);
//    fclose(testing_answer);
	
	return 0;
}
