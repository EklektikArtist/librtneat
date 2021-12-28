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

#include "species.h"
#include <iostream>

using namespace NEAT;

Species::Species(int i) {
	id=i;
	age=1;
	ave_fitness=0.0;
	expected_offspring=0;
	novel=false;
	age_of_last_improvement=0;
	max_fitness=0;
	max_fitness_ever=0;
	obliterate=false;

	average_est=0;
}

Species::Species(int i,bool n) {
	id=i;
	age=1;
	ave_fitness=0.0;
	expected_offspring=0;
	novel=n;
	age_of_last_improvement=0;
	max_fitness=0;
	max_fitness_ever=0;
	obliterate=false;

	average_est=0;
}


Species::~Species() {

	std::vector<Organism*>::iterator curorg;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		delete (*curorg);
	}

}

bool Species::rank() {
	//organisms.qsort(order_orgs);
    std::sort(organisms.begin(), organisms.end(), order_orgs);
	return true;
}

double Species::estimate_average() {
	std::vector<Organism*>::iterator curorg;
	double total = 0.0; //running total of fitnesses

	//Note: Since evolution is happening in real-time, some organisms may not
	//have been around long enough to count them in the fitness evaluation

	double num_orgs = 0; //counts number of orgs above the time_alive threshold


	for(curorg = organisms.begin(); curorg != organisms.end(); ++curorg) {
		//New variable time_alive
		if (((*curorg)->time_alive) >= NEAT::time_alive_minimum) {    
			total += (*curorg)->fitness;
			++num_orgs;
		}
	}

	if (num_orgs > 0)
		average_est = total / num_orgs;
	else {
		average_est = 0;
	}

	return average_est;
} 

	
	Organism *Species::reproduce_one(int generation, Population *pop,std::vector<Species*> &sorted_species) {
	//bool Species::reproduce(int generation, Population *pop,std::vector<Species*> &sorted_species) {
	int count=generation; //This will assign genome id's according to the generation
	std::vector<Organism*>::iterator curorg;


	std::vector<Organism*> elig_orgs; //This list contains the eligible organisms (KEN)

	int poolsize;  //The number of Organisms in the old generation

	int orgnum;  //Random variable
	int orgcount;
	Organism *mom = 0; //Parent Organisms
	Organism *dad = 0;
	Organism *baby;  //The new Organism

	Genome *new_genome;  //For holding baby's genes

	std::vector<Species*>::iterator curspecies;  //For adding baby
	Species *newspecies; //For babies in new Species
	Organism *comporg;  //For Species determination through comparison

	Species *randspecies;  //For mating outside the Species
	double randmult;
	int randspeciesnum;
	int spcount;  
	std::vector<Species*>::iterator cursp;

	Network *net_analogue;  //For adding link to test for recurrency
	int pause;

	bool outside;

	bool found;  //When a Species is found

	bool champ_done=false; //Flag the preservation of the champion  

	Organism *thechamp;

	int giveup; //For giving up finding a mate outside the species

	bool mut_struct_baby;
	bool mate_baby;

	//The weight mutation power is species specific depending on its age
	double mut_power=NEAT::weight_mut_power;

	//Roulette wheel variables
	double total_fitness=0.0;
	double marble;  //The marble will have a number between 0 and total_fitness
	double spin;  //Fitness total while the wheel is spinning


	//printf("In reproduce_one\n");

	//Check for a mistake
	if ((organisms.size()==0)) {
		//    cout<<"ERROR:  ATTEMPT TO REPRODUCE OUT OF EMPTY SPECIES"<<endl;
		return false;
	}

	rank(); //Make sure organisms are ordered by rank

	//ADDED CODE (Ken) 
	//Now transfer the list to elig_orgs without including the ones that are too young (Ken)
	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		if ((*curorg)->time_alive >= NEAT::time_alive_minimum)
			elig_orgs.push_back(*curorg);
	}

	//Now elig_orgs should be an ordered list of mature organisms
	//Special case: if it's empty, then just include all the organisms (age doesn't matter in this case) (Ken)
	if (elig_orgs.size()==0) {
			for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
					elig_orgs.push_back(*curorg);
		}		
	}

	//std::cout<<"Eligible orgs: "<<elig_orgs.size()<<std::endl;

	//Now elig_orgs is guaranteed to contain either an ordered list of mature orgs or all the orgs (Ken)
	//We may also want to check to see if we are getting pools of >1 organism (to make sure our survival_thresh is sensible) (Ken)

	//Only choose from among the top ranked orgs
	poolsize=(elig_orgs.size() - 1) * NEAT::survival_thresh;
	//poolsize=(organisms.size()-1)*.9;

	//Compute total fitness of species for a roulette wheel
	//Note: You don't get much advantage from a roulette here
	// because the size of a species is relatively small.
	// But you can use it by using the roulette code here
	for(curorg=elig_orgs.begin();curorg!=elig_orgs.end();++curorg) {
	  total_fitness+=(*curorg)->fitness;
	}

	//In reproducing only one offspring, the champ shouldn't matter  
	//thechamp=(*(organisms.begin()));

	//Create one offspring for the Species

	mut_struct_baby=false;
	mate_baby=false;

	outside=false;

	//First, decide whether to mate or mutate
	//If there is only one organism in the pool, then always mutate
	if ((randfloat()<NEAT::mutate_only_prob)||
		poolsize == 0) {

			//Choose the random parent

			//RANDOM PARENT CHOOSER
			orgnum=randint(0,poolsize);
			curorg=elig_orgs.begin();
			for(orgcount=0;orgcount<orgnum;orgcount++)
				++curorg;                       



			////Roulette Wheel
			//marble=randfloat()*total_fitness;
			//curorg=elig_orgs.begin();
			//spin=(*curorg)->fitness;
			//while(spin<marble) {
			//	++curorg;

				//Keep the wheel spinning
			//	spin+=(*curorg)->fitness;
			//}
			//Finished roulette
			

			mom=(*curorg);

			new_genome=(mom->gnome)->duplicate(count);

			//Do the mutation depending on probabilities of 
			//various mutations

			if (randfloat()<NEAT::mutate_add_node_prob) {
				//cout<<"mutate add node"<<endl;
				new_genome->mutate_add_node(pop->innovations,pop->cur_node_id,pop->cur_innov_num);
				mut_struct_baby=true;
			}
			else if (randfloat()<NEAT::mutate_add_link_prob) {
				//cout<<"mutate add link"<<endl;
				net_analogue=new_genome->genesis(generation);
				new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
				delete net_analogue;
				mut_struct_baby=true;
			}
			//NOTE:  A link CANNOT be added directly after a node was added because the phenotype
			//       will not be appropriately altered to reflect the change
			else {
				//If we didn't do a structural mutation, we do the other kinds

				if (randfloat()<NEAT::mutate_random_trait_prob) {
					//cout<<"mutate random trait"<<endl;
					new_genome->mutate_random_trait();
				}
				if (randfloat()<NEAT::mutate_link_trait_prob) {
					//cout<<"mutate_link_trait"<<endl;
					new_genome->mutate_link_trait(1);
				}
				if (randfloat()<NEAT::mutate_node_trait_prob) {
					//cout<<"mutate_node_trait"<<endl;
					new_genome->mutate_node_trait(1);
				}
				if (randfloat()<NEAT::mutate_link_weights_prob) {
					//cout<<"mutate_link_weights"<<endl;
					new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
				}
				if (randfloat()<NEAT::mutate_toggle_enable_prob) {
					//cout<<"mutate toggle enable"<<endl;
					new_genome->mutate_toggle_enable(1);

				}
				if (randfloat()<NEAT::mutate_gene_reenable_prob) {
					//cout<<"mutate gene reenable"<<endl;
					new_genome->mutate_gene_reenable();
				}
			}

			baby=new Organism(0.0,new_genome,generation);

		}

		//Otherwise we should mate 
	else {

		//Choose the random mom
		orgnum=randint(0,poolsize);
		curorg=elig_orgs.begin();
		for(orgcount=0;orgcount<orgnum;orgcount++)
			++curorg;


		////Roulette Wheel
		//marble=randfloat()*total_fitness;
		//curorg=elig_orgs.begin();
		//spin=(*curorg)->fitness;
		//while(spin<marble) {
		//	++curorg;

			//Keep the wheel spinning
	  //	spin+=(*curorg)->fitness;
	  //}
		//Finished roulette
		

		mom=(*curorg);         

		//Choose random dad

		if ((randfloat()>NEAT::interspecies_mate_rate)) {
			//Mate within Species

			orgnum=randint(0,poolsize);
			curorg=elig_orgs.begin();
			for(orgcount=0;orgcount<orgnum;orgcount++)
				++curorg;


			////Use a roulette wheel
			//marble=randfloat()*total_fitness;
			//curorg=elig_orgs.begin();
			//spin=(*curorg)->fitness;
			//while(spin<marble) {
			//	++curorg;

			
				//Keep the wheel spinning
		  //	spin+=(*curorg)->fitness;
		  //}
			////Finished roulette
				

			dad=(*curorg);
		}
		else {

			//Mate outside Species  
			randspecies=this;

			//Select a random species
			giveup=0;  //Give up if you cant find a different Species
			while((randspecies==this)&&(giveup<5)) {

				//This old way just chose any old species
				//randspeciesnum=randint(0,(pop->species).size()-1);

				//Choose a random species tending towards better species
				randmult=gaussrand()/4;
				if (randmult>1.0) randmult=1.0;
				//This tends to select better species
                randspeciesnum=(int) floor((randmult*(sorted_species.size()-1.0))+0.5);
				cursp=(sorted_species.begin());
				for(spcount=0;spcount<randspeciesnum;spcount++)
					++cursp;
				randspecies=(*cursp);

				++giveup;
			}

			//OLD WAY: Choose a random dad from the random species
			//Select a random dad from the random Species
			//NOTE:  It is possible that a mating could take place
			//       here between the mom and a baby from the NEW
			//       generation in some other Species
			//orgnum=randint(0,(randspecies->organisms).size()-1);
			//curorg=(randspecies->organisms).begin();
			//for(orgcount=0;orgcount<orgnum;orgcount++)
			//  ++curorg;
			//dad=(*curorg);            

			//New way: Make dad be a champ from the random species
			dad=(*((randspecies->organisms).begin()));

			outside=true;
		}

		//Perform mating based on probabilities of differrent mating types
		if (randfloat()<NEAT::mate_multipoint_prob) { 
			new_genome=(mom->gnome)->mate_multipoint(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
		}
		else if (randfloat()<(NEAT::mate_multipoint_avg_prob/(NEAT::mate_multipoint_avg_prob+NEAT::mate_singlepoint_prob))) {
			new_genome=(mom->gnome)->mate_multipoint_avg(dad->gnome,count,mom->orig_fitness,dad->orig_fitness,outside);
		}
		else {
			new_genome=(mom->gnome)->mate_singlepoint(dad->gnome,count);
		}

		mate_baby=true;

		//Determine whether to mutate the baby's Genome
		//This is done randomly or if the mom and dad are the same organism
		if ((randfloat()>NEAT::mate_only_prob)||
			((dad->gnome)->genome_id==(mom->gnome)->genome_id)||
			(((dad->gnome)->compatibility(mom->gnome))==0.0))
		{

			//Do the mutation depending on probabilities of 
			//various mutations
			if (randfloat()<NEAT::mutate_add_node_prob) {
				new_genome->mutate_add_node(pop->innovations,pop->cur_node_id,pop->cur_innov_num);
				//  cout<<"mutate_add_node: "<<new_genome<<endl;
				mut_struct_baby=true;
			}
			else if (randfloat()<NEAT::mutate_add_link_prob) {
				net_analogue=new_genome->genesis(generation);
				new_genome->mutate_add_link(pop->innovations,pop->cur_innov_num,NEAT::newlink_tries);
				delete net_analogue;
				//cout<<"mutate_add_link: "<<new_genome<<endl;
				mut_struct_baby=true;
			}
			else {
				//Only do other mutations when not doing strurctural mutations

				if (randfloat()<NEAT::mutate_random_trait_prob) {
					new_genome->mutate_random_trait();
					//cout<<"..mutate random trait: "<<new_genome<<endl;
				}
				if (randfloat()<NEAT::mutate_link_trait_prob) {
					new_genome->mutate_link_trait(1);
					//cout<<"..mutate link trait: "<<new_genome<<endl;
				}
				if (randfloat()<NEAT::mutate_node_trait_prob) {
					new_genome->mutate_node_trait(1);
					//cout<<"mutate_node_trait: "<<new_genome<<endl;
				}
				if (randfloat()<NEAT::mutate_link_weights_prob) {
					new_genome->mutate_link_weights(mut_power,1.0,GAUSSIAN);
					//cout<<"mutate_link_weights: "<<new_genome<<endl;
				}
				if (randfloat()<NEAT::mutate_toggle_enable_prob) {
					new_genome->mutate_toggle_enable(1);
					//cout<<"mutate_toggle_enable: "<<new_genome<<endl;
				}
				if (randfloat()<NEAT::mutate_gene_reenable_prob) {
					new_genome->mutate_gene_reenable(); 
					//cout<<"mutate_gene_reenable: "<<new_genome<<endl;
				}
			}

			//Create the baby
			baby=new Organism(0.0,new_genome,generation);

		}
		else {
			//Create the baby without mutating first
			baby=new Organism(0.0,new_genome,generation);
		}

	}

	//Add the baby to its proper Species
	//If it doesn't fit a Species, create a new one

	baby->mut_struct_baby=mut_struct_baby;
	baby->mate_baby=mate_baby;

	curspecies=(pop->species).begin();
	if (curspecies==(pop->species).end()){
		//Create the first species
		newspecies=new Species(++(pop->last_species),true);
		(pop->species).push_back(newspecies);
		newspecies->add_Organism(baby);  //Add the baby
		baby->species=newspecies;  //Point the baby to its species
	} 
	else {
		comporg=(*curspecies)->first();
		found=false;


		// Testing out what happens when speciation is disabled
		//found = true;
		//(*curspecies)->add_Organism(baby);
		//baby->species = (*curspecies);


		while((curspecies!=(pop->species).end()) && (!found)) 
		{	
			if (comporg==0) {
				//Keep searching for a matching species
				++curspecies;
				if (curspecies!=(pop->species).end())
					comporg=(*curspecies)->first();
			}
			else if (((baby->gnome)->compatibility(comporg->gnome))<NEAT::compat_threshold) {
				//Found compatible species, so add this organism to it
				(*curspecies)->add_Organism(baby);
				baby->species=(*curspecies);  //Point organism to its species
				found=true;  //Note the search is over
			}
			else {
				//Keep searching for a matching species
				++curspecies;
				if (curspecies!=(pop->species).end()) 
					comporg=(*curspecies)->first();
			}
		}

		//If we didn't find a match, create a new species
		if (found==false) {
			newspecies=new Species(++(pop->last_species),true);
			(pop->species).push_back(newspecies);
			newspecies->add_Organism(baby);  //Add the baby
			baby->species=newspecies;  //Point baby to its species
		}

	} //end else     

	//Put the baby also in the master organism list
	(pop->organisms).push_back(baby);

	return baby; //Return a pointer to the baby
}

bool Species::add_Organism(Organism *o){
	organisms.push_back(o);
	return true;
}

Organism *Species::get_champ() {
	double champ_fitness=-1.0;
	Organism *thechamp;
	std::vector<Organism*>::iterator curorg;

	for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {
		//TODO: Remove DEBUG code
		//cout<<"searching for champ...looking at org "<<(*curorg)->gnome->genome_id<<" fitness: "<<(*curorg)->fitness<<endl;
		if (((*curorg)->fitness)>champ_fitness) {
			thechamp=(*curorg);
			champ_fitness=thechamp->fitness;
		}
	}

	//cout<<"returning champ #"<<thechamp->gnome->genome_id<<endl;

	return thechamp;

}

bool Species::remove_org(Organism *org) {
	std::vector<Organism*>::iterator curorg;

	curorg=organisms.begin();
	while((curorg!=organisms.end())&&
		((*curorg)!=org))
		++curorg;

	if (curorg==organisms.end()) {
		//cout<<"ALERT: Attempt to remove nonexistent Organism from Species"<<endl;
		return false;
	}
	else {
		organisms.erase(curorg);
		return true;
	}

}

Organism *Species::first() {
	return *(organisms.begin());
}

//Print Species to a file outFile
bool Species::print_to_file(std::ofstream &outFile) {
  std::vector<Organism*>::iterator curorg;

  //Print a comment on the Species info
  outFile<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl<<std::endl;

  //Show user what's going on
  std::cout<<std::endl<<"/* Species #"<<id<<" : (Size "<<organisms.size()<<") (AF "<<ave_fitness<<") (Age "<<age<<")  */"<<std::endl;

  //Print all the Organisms' Genomes to the outFile
  for(curorg=organisms.begin();curorg!=organisms.end();++curorg) {

    //Put the fitness for each organism in a comment
    outFile<<std::endl<<"/* Organism #"<<((*curorg)->gnome)->genome_id<<" Fitness: "<<(*curorg)->fitness<<" Error: "<<(*curorg)->error<<" */"<<std::endl;

    //If it is a winner, mark it in a comment
    if ((*curorg)->winner) outFile<<"/* ##------$ WINNER "<<((*curorg)->gnome)->genome_id<<" SPECIES #"<<id<<" $------## */"<<std::endl;

    ((*curorg)->gnome)->print_to_file(outFile);
    //We can confirm by writing the genome #'s to the screen
    //std::cout<<((*curorg)->gnome)->genome_id<<std::endl;
  }

  return true;

}