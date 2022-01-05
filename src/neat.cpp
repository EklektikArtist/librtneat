/*
 * Copyright (C) The University of Texas, 2006. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT
 * AND WARRANTIES OF PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE
 * ARISE FROM COURSE OF DEALING OR USAGE OF TRADE. NO WARRANTY IS EITHER
 * EXPRESS OR IMPLIED WITH RESPECT TO THE USE OF THE SOFTWARE OR
 * DOCUMENTATION. Under no circumstances shall University be liable for
 * incidental, special, indirect, direct or consequential damages or loss
 * of profits, interruption of business, or related expenses which may
 * arise from use of Software or Documentation, including but not limited
 * to those resulting from defects in Software and/or Documentation, or
 * loss or inaccuracy of data of any kind.
 */

// Modifications made by David Roberts <d@vidr.cc>, 2010.

#include "code/main.h"
#include "neat.h"

int NEAT::time_alive_minimum = 0;
double NEAT::trait_param_mut_prob = 0.3;
double NEAT::trait_mutation_power = 0.3; // Power of mutation on a signle trait param 
double NEAT::weight_mut_power = 0.3; // The power of a linkweight mutation 
double NEAT::disjoint_coeff = 0.3;
double NEAT::excess_coeff = 0.3;
double NEAT::mutdiff_coeff = 0.3;
double NEAT::compat_threshold = 0.3;
double NEAT::survival_thresh = 0.3; // Percent of ave fitness for survival 
double NEAT::mutate_only_prob = 0.3; // Prob. of a non-mating reproduction 
double NEAT::mutate_random_trait_prob = 0.3;
double NEAT::mutate_link_trait_prob = 0.3;
double NEAT::mutate_node_trait_prob = 0.3; 
double NEAT::mutate_link_weights_prob = 0.3;
double NEAT::mutate_toggle_enable_prob = 0.3;
double NEAT::mutate_gene_reenable_prob = 0.3;
double NEAT::mutate_add_node_prob = 0.3;
double NEAT::mutate_add_link_prob = 0.3;
double NEAT::interspecies_mate_rate = 0.3; // Prob. of a mate being outside species 
double NEAT::mate_multipoint_prob = 0.3;     
double NEAT::mate_multipoint_avg_prob = 0.3;
double NEAT::mate_singlepoint_prob = 0.3;
double NEAT::mate_only_prob = 0.3; // Prob. of mating without mutation 
double NEAT::recur_only_prob = 0.3;  // Probability of forcing selection of ONLY links that are naturally recurrent 
int NEAT::pop_size = MAX_HUBS;  // Size of population 
int NEAT::newlink_tries = 1;  // Number of tries mutate_add_link will attempt to find an open link 

int NEAT::getUnitCount(const char *string, const char *set)
{
	int count = 0;
	short last = 0;
	while(*string)
	{
		last = *string++;

		for(int i =0; set[i]; i++)
		{
			if(last == set[i])
			{
				count++;
				last = 0;
				break;
			}   
		}
	}
	if(last)
		count++;
	return count;
}   

double NEAT::gaussrand() {
	static int iset=0;
	static double gset;
	double fac,rsq,v1,v2;

	if (iset==0) {
		do {
			v1=2.0*(randfloat())-1.0;
			v2=2.0*(randfloat())-1.0;
			rsq=v1*v1+v2*v2;
		} while (rsq>=1.0 || rsq==0.0);
		fac=sqrt(-2.0*log(rsq)/rsq);
		gset=v1*fac;
		iset=1;
		return v2*fac;
	}
	else {
		iset=0;
		return gset;
	}
}

double NEAT::fsigmoid(double activesum,double slope,double constant) {
	//RIGHT SHIFTED ---------------------------------------------------------
	//return (1/(1+(exp(-(slope*activesum-constant))))); //ave 3213 clean on 40 runs of p2m and 3468 on another 40 
	//41394 with 1 failure on 8 runs

	//LEFT SHIFTED ----------------------------------------------------------
	//return (1/(1+(exp(-(slope*activesum+constant))))); //original setting ave 3423 on 40 runs of p2m, 3729 and 1 failure also

	//PLAIN SIGMOID ---------------------------------------------------------
	//return (1/(1+(exp(-activesum)))); //3511 and 1 failure

	//LEFT SHIFTED NON-STEEPENED---------------------------------------------
	//return (1/(1+(exp(-activesum-constant)))); //simple left shifted

	//NON-SHIFTED STEEPENED
	return (1/(1+(exp(-(slope*activesum))))); //Compressed
}

double NEAT::hebbian(double weight, double maxweight, double active_in, double active_out, double hebb_rate, double pre_rate, double post_rate) {

	bool neg=false;
	double delta;

	//double weight_mag;

	double topweight;

	if (maxweight<5.0) maxweight=5.0;

	if (weight>maxweight) weight=maxweight;

	if (weight<-maxweight) weight=-maxweight;

	if (weight<0) {
		neg=true;
		weight=-weight;
	}


	//if (weight<0) {
	//  weight_mag=-weight;
	//}
	//else weight_mag=weight;


	topweight=weight+2.0;
	if (topweight>maxweight) topweight=maxweight;

	if (!(neg)) {
		//if (true) {
		delta=
			hebb_rate*(maxweight-weight)*active_in*active_out+
			pre_rate*(topweight)*active_in*(active_out-1.0);
		//post_rate*(weight+1.0)*(active_in-1.0)*active_out;

		//delta=delta-hebb_rate/2; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		//cout<<"delta: "<<delta<<endl;

		//if (weight+delta>0)
		//  return weight+delta;
		//else return 0.01;

		return weight+delta;

	}
	else {
		//In the inhibatory case, we strengthen the synapse when output is low and
		//input is high
		delta=
			pre_rate*(maxweight-weight)*active_in*(1.0-active_out)+ //"unhebb"
			//hebb_rate*(maxweight-weight)*(1.0-active_in)*(active_out)+
			-hebb_rate*(topweight+2.0)*active_in*active_out+ //anti-hebbian
			//hebb_rate*(maxweight-weight)*active_in*active_out+
			//pre_rate*weight*active_in*(active_out-1.0)+
			//post_rate*weight*(active_in-1.0)*active_out;
			0;

		//delta=delta-hebb_rate; //decay

		//delta=delta+randposneg()*randfloat()*0.01; //noise

		//if (-(weight+delta)<0)
		//  return -(weight+delta);
		//  else return -0.01;

		return -(weight+delta);

	}

}



