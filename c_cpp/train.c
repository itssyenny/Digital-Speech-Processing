#include <stdio.h>
#include <string.h>
#include "hmm.h"
void letsTrain(HMM *hmm, int iteration, char *seq_model) {

	for(int a = 0; a < iteration; a++) {
		
		FILE *fp = open_or_die(seq_model, "r");
		
		int row = 0;
		char str[MAX_SEQ];
		double expected_gamma[MAX_STATE] = {0.0};
		double expected_gamma_initial[MAX_SEQ] = {0.0};
		double expected_gamma_final[MAX_SEQ] = {0.0};
		double expected_epsilon[MAX_STATE][MAX_STATE] = {{0.0}};
		double obsertvation_gamma[MAX_SEQ][MAX_STATE] = {{0.0}};

		while(fscanf(fp, "%s", str) != EOF) {
			//read line by line
			row++;
			int seq[MAX_SEQ];
			int Time = strlen(str);

			for(int t = 0; t < Time; t++) {
				seq[t] = str[t]-'A';
			}

			double alfa[MAX_STATE][MAX_SEQ] = {{0.0}};
			double beta[MAX_STATE][MAX_SEQ] = {{0.0}};
			double gamma[MAX_STATE][MAX_SEQ] = {{0.0}};
			double epsilon[MAX_STATE][MAX_STATE][MAX_SEQ] = {{{0.0}}};
			
			
			
			//calculate alfa
			for(int t = 0; t < Time; t++) {
				for(int j = 0; j < hmm->state_num; j++) {
					if(t == 0) {
						alfa[j][t] = hmm->initial[j] * hmm->observation[seq[t]][j];
					} 
					else {
						for(int i = 0; i < hmm->state_num; i++) {
							alfa[j][t] += alfa[i][t-1] * hmm->transition[i][j];
						}
						alfa[j][t] *= hmm->observation[seq[t]][j];	
					}
				}
			}

			//calculate beta
			for(int t = Time-1; t >= 0; t--) {
				for(int i = 0; i < hmm->state_num; i++) {
					if(t == Time-1) {
						beta[i][Time-1] = 1; 
					} 
					else {
						for(int j = 0; j < hmm->state_num; j++) {
							beta[i][t] += hmm->transition[i][j] * hmm->observation[seq[t+1]][j] * beta[j][t+1];
						}
					}
				}
			}

			//calculate gamma
			double sum = 0.0;
			for(int t = 0; t < Time; t++) {
				sum = 0.0;
				for(int i = 0; i < hmm->state_num; i++) {
					gamma[i][t] = alfa[i][t] * beta[i][t];
					sum += gamma[i][t];
				}
				for(int i = 0; i < hmm->state_num; i++) {
					gamma[i][t] /= sum;
				}
			}

			//calculate epsilon
			for(int t = 0; t < Time-1; t++) {
				sum = 0.0;
				for(int i = 0; i < hmm->state_num; i++) {
					for(int j = 0; j < hmm->state_num; j++) {
						epsilon[i][j][t] = alfa[i][t] * hmm->transition[i][j] * hmm->observation[seq[t+1]][j] * beta[j][t+1];
						sum += epsilon[i][j][t];
					}
				}

				for(int i = 0; i < hmm->state_num; i++) {
					for(int j = 0; j < hmm->state_num; j++) {
						epsilon[i][j][t] /= sum;
					}
				}
			}

			//calculate the accumulation of gamma and epsilon through all samples
			for(int i = 0; i < hmm->state_num; i++) {
				expected_gamma_initial[i] += gamma[i][0];
				expected_gamma_final[i] += gamma[i][Time-1];
			}

			for(int i = 0; i < hmm->state_num; i++) {
				for(int t = 0; t < Time-1; t++) {
					expected_gamma[i] += gamma[i][t];
					obsertvation_gamma[seq[t]][i] += gamma[i][t];
				}
			}

			for(int i = 0; i < hmm->state_num; i++)
				obsertvation_gamma[seq[Time-1]][i] += gamma[i][Time-1];
			
			for(int t = 0; t < Time-1; t++) {
				for(int i = 0; i < hmm->state_num; i++) {
					for(int j = 0; j < hmm->state_num; j++) {
						expected_epsilon[i][j] += epsilon[i][j][t];
					}
				}
			}

		} //end

		fclose(fp);
		//re-estimate the model parameters
		// double sum_phi = 0.0;
		for(int i = 0; i < hmm->state_num; i++) {
			hmm->initial[i] = expected_gamma_initial[i]/row; 
			// sum_phi += hmm->initial[i];
		}	
		// printf("sum_phi = %.5f\n", sum_phi);

		for(int i = 0; i < hmm->state_num; i++) {
			for(int j = 0; j < hmm->state_num; j++) {
				hmm->transition[i][j] = expected_epsilon[i][j]/expected_gamma[i];
			}
		}

		// for(int i = 0; i < hmm->state_num; i++) {
		// 	double sum = 0.0;
		// 	for(int j = 0; j < hmm->state_num; j++)
		// 		sum += hmm->transition[i][j];
		// 	printf("sum transition = %.5f\n", sum);
		// }

		for(int i = 0; i < hmm->state_num; i++) 
			expected_gamma[i] += expected_gamma_final[i];

		for(int j = 0; j < hmm->state_num; j++) {
			for(int k = 0; k < hmm->observ_num; k++) {
				hmm->observation[k][j] = obsertvation_gamma[k][j]/expected_gamma[j];
			}
		}

		// for(int j = 0; j < hmm->state_num; j++) {
		// 	double sum = 0.0;
		// 	for(int i = 0; i < hmm->observ_num; i++)
		// 		sum += hmm->observation[i][j];
		// 	printf("sum observation = %.5f\n", sum);
		// }

	}
}

int main(int argc, char const *argv[]) {

	int iteration = atoi(argv[1]);	// number of iteration

	char seq_model[64];
	strcpy(seq_model, argv[3]);

	HMM hmm;
	loadHMM(&hmm, argv[2]);
	
	letsTrain(&hmm, iteration, seq_model);


	FILE *fout = fopen(argv[4], "wb");

	dumpHMM(fout, &hmm);
	fclose(fout);
}