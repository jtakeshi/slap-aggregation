#include <vector>
#include <iostream>
#include <getopt.h>

#include "Aggregator_Coeff.h"

using namespace std;

const static float SCALE_DEFAULT = 0.5f;
const static uint64_t PLAIN_MODULUS_BITS_DEFAULT = 16;

int main(int argc, char ** argv){

	bool do_noise = true;
	int num, den;
	float scale = SCALE_DEFAULT;
	float_to_frac(scale, num, den);
	unsigned int num_aggregations = 100;
	uint64_t t = 1;
	unsigned int log_t = PLAIN_MODULUS_BITS_DEFAULT;
	unsigned int num_users = 100;

	int c;
	
	while ((c = getopt(argc, argv, "t:i:n:")) != -1){
		switch(c){
			case 't':{
				log_t = atoi(optarg);
				break;
			}
			case 'i':{
				num_aggregations = atoi(optarg);
				break;
			}
			case 'n':{
				num_users = atoi(optarg);
				break;
			}
			default:{
				cout << "ERROR: unrecognized input: " << c << endl;
				return 0;
			}
		}
	}


	t <<= log_t;

	vector<uint64_t> moduli, keys, delta_mod_q;

	vector<double> noise_times, enc_times;
	if(do_noise){
		noise_times.reserve(num_aggregations);
	}
	enc_times.reserve(num_aggregations);

	unsigned int ctext_bits = generate_params(t, num_aggregations, num_users,
		moduli, keys, delta_mod_q);
	DiscreteLaplacian dl(num, den);

	for(unsigned int i = 0; i < num_aggregations; i++){

		uint64_t user_input = dl.uniform_64(t);
		double noise_time, enc_time;

		auto ret = enc_noclass(user_input, 
			i, 
	    keys, do_noise, 
	    noise_time, enc_time, moduli, 
	    delta_mod_q,
	    dl, t,
	    num, den);

		if(do_noise){
			noise_times.push_back(noise_time);
		}
		enc_times.push_back(enc_time);

	}

	cout << "users," << num_users << endl;
	cout << "plain_bits," << log_t << endl;
	cout << "ctext_bits," << ctext_bits << endl;
	if(do_noise){
		cout << "noise,";
		for(const double d : noise_times){
			cout << d << ',';
		}
		cout << endl;
	}
	cout << "enc,";
	for(const double d : enc_times){
		cout << d << ',';
	}
	cout << endl;


	return 0;
}