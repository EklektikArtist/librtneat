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

#ifndef _NEAT_ORGANISM_H_
#define _NEAT_ORGANISM_H_

#include "genome.h"
#include "species.h"

namespace NEAT {

	class Species;
	class Population;

	// ---------------------------------------------  
	// ORGANISM CLASS:
	//   Organisms are Genomes and Networks with fitness
	//   information 
	//   i.e. The genotype and phenotype together
	// ---------------------------------------------  
	class Organism {
	public:
		double fitness;  //A measure of fitness for the Organism
		double orig_fitness;  //A fitness measure that won't change during adjustments
		double error;  //Used just for reporting purposes
		bool winner;  //Win marker (if needed for a particular task)
		Network net;  //The Organism's phenotype
		Genome gnome; //The Organism's genotype 
		Species species;  //The Organism's Species 
		double expected_offspring; //Number of children this Organism may have
		int generation;  //Tells which generation this Organism is from
		bool eliminate;  //Marker for destruction of inferior Organisms
		bool champion; //Marks the species champ
		int super_champ_offspring;  //Number of reserved offspring for a population leader
		bool pop_champ;  //Marks the best in population
		bool pop_champ_child; //Marks the duplicate child of a champion (for tracking purposes)
		double high_fit; //DEBUG variable- high fitness of champ
		int time_alive; //When playing in real-time allows knowing the maturity of an individual

		// Track its origin- for debugging or analysis- we can tell how the organism was born
		bool mut_struct_baby;
		bool mate_baby;

		// MetaData for the object
		char metadata[128];
		bool modified;

		// Regenerate the network based on a change in the genotype 
		void update_phenotype();

		// Print the Organism's genome to a file preceded by a comment detailing the organism's species, number, and fitness 
		bool print_to_file(char *filename);   
		bool write_to_file(std::ostream &outFile);

		Organism(double fit, Genome g, int gen, const char* md = 0);        
		Organism(void);
		//Organism(const Organism& org);	// Copy Constructor
		~Organism();

	};

	// This is used for list sorting of Organisms by fitness..highest fitness first
	bool order_orgs(Organism *x, Organism *y);

	bool order_orgs_by_adjusted_fit(Organism *x, Organism *y);

} // namespace NEAT

#endif
