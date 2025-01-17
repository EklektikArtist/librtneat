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

#include "trait.h"
#include <sstream>
using namespace NEAT;

Trait::Trait(void) {
	for(int count=0;count<NEAT::num_trait_params;count++)
		params[count]=0;
}

Trait::Trait(int id,double p1,double p2,double p3,double p4,double p5,double p6,double p7,double p8,double p9) {
	trait_id=id;
	params[0]=p1;
	params[1]=p2;
	params[2]=p3;
	params[3]=p4;
	params[4]=p5;
	params[5]=p6;
	params[6]=p7;
	params[7]=0;
}

Trait::Trait(Trait *t) {
	for(int count=0;count<NEAT::num_trait_params;count++)
		params[count]=(t->params)[count];

	trait_id=t->trait_id;
}

Trait::Trait(const char *argline) {

    std::stringstream ss(argline);
	//Read in trait id
    //std::string curword;
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
    
	//trait_id = atoi(curword);
    ss >> trait_id;

    //std::cout << ss.str() << " trait_id: " << trait_id << std::endl;

	//IS THE STOPPING CONDITION CORRECT?  ALERT
	for(int count=0;count<NEAT::num_trait_params;count++) {
		//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
		//params[count] = atof(curword);
        ss >> params[count];
		//iFile>>params[count];
	}

}

Trait::Trait(Trait *t1,Trait *t2) {
	for(int count=0;count<NEAT::num_trait_params;count++)
		params[count]=(((t1->params)[count])+((t2->params)[count]))/2.0;
	trait_id=t1->trait_id;
}

void Trait::print_to_file(std::ofstream &outFile) {
  outFile<<"trait "<<trait_id<<" ";
  for(int count=0;count<NEAT::num_trait_params;count++)
    outFile<<params[count]<<" ";

  outFile<<std::endl;

}

void Trait::mutate() {
	for(int count=0;count<NEAT::num_trait_params;count++) {
		if (randfloat()>NEAT::trait_param_mut_prob) {
			params[count]+=(randposneg()*randfloat())*NEAT::trait_mutation_power;
			if (params[count]<0) params[count]=0;
			if (params[count]>1.0) params[count]=1.0;
		}
	}
}
