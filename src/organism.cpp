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

#include "organism.h"

using namespace NEAT;

Organism::Organism(double fit, Genome *g,int gen, const char* md) {
	fitness=fit;
	orig_fitness=fitness;
	gnome=*g;
	net=*gnome.genesis(gnome.genome_id);
	species=0;  //Start it in no Species
	expected_offspring=0;
	generation=gen;
	eliminate=false;
	error=0;
	winner=false;
	champion=false;
	super_champ_offspring=0;

	// If md is null, then we don't have metadata, otherwise we do have metadata so copy it over
	if(md == 0) {
		strcpy(metadata, "");
	} else {
		strncpy(metadata, md, 128);
	}

	time_alive=0;

	//DEBUG vars
	pop_champ=false;
	pop_champ_child=false;
	high_fit=0;
	mut_struct_baby=0;
	mate_baby=0;

	modified = true;
}

Organism::Organism(void) {
	fitness=0;
	orig_fitness=fitness;
	gnome=*new Genome( 0, 0, 0, 0 );
	net=*gnome.genesis(gnome.genome_id);
	species=0;  //Start it in no Species
	expected_offspring=0;
	generation=0;
	eliminate=false;
	error=0;
	winner=false;
	champion=false;
	super_champ_offspring=0;

    strcpy(metadata, "");

	time_alive=0;

	//DEBUG vars
	pop_champ=false;
	pop_champ_child=false;
	high_fit=0;
	mut_struct_baby=0;
	mate_baby=0;

	modified = true;
}

Organism::~Organism() {
	delete &net;
	delete &gnome;
}

bool NEAT::order_orgs(Organism *x, Organism *y) {
	return (x)->fitness > (y)->fitness;
}
