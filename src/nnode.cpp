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

#include "nnode.h"
#include <sstream>

using namespace NEAT;

NNode::NNode(void) {
	active_flag=false;
	activesum=0;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=NEURON; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=0;
	ftype=SIGMOID;
	gen_node_label=HIDDEN;
	dup=0;
	analogue=0;
	frozen=false;
	trait_id=1;
	override=false;
}

NNode::NNode(nodetype ntype,int nodeid) {
	active_flag=false;
	activesum=0;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	gen_node_label=HIDDEN;
	dup=0;
	analogue=0;
	frozen=false;
	trait_id=1;
	override=false;
}

NNode::NNode(nodetype ntype,int nodeid, nodeplace placement) {
	active_flag=false;
	activesum=0;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=ntype; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=nodeid;
	ftype=SIGMOID;
	gen_node_label=placement;
	dup=0;
	analogue=0;
	frozen=false;
	trait_id=1;
	override=false;
}

NNode::NNode(NNode *n,Trait *t) {
	active_flag=false;
	activation=0;
	output=0;
	last_activation=0;
	last_activation2=0;
	type=n->type; //NEURON or SENSOR type
	activation_count=0; //Inactive upon creation
	node_id=n->node_id;
	ftype=SIGMOID;
	gen_node_label=n->gen_node_label;
	dup=0;
	analogue=0;
	nodetrait=t;
	frozen=false;
	if (t!=0)
		trait_id=t->trait_id;
	else trait_id=1;
	override=false;
}

NNode::NNode (const char *argline, std::vector<Trait*> &traits) {
	int traitnum;
	std::vector<Trait*>::iterator curtrait;

	activesum=0;

    std::stringstream ss(argline);
	//char curword[128];
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//Get the node parameters
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//node_id = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//traitnum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//type = (nodetype)atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//gen_node_label = (nodeplace)atoi(curword);

    int nodety, nodepl;
    ss >> node_id >> traitnum >> nodety >> nodepl;
    type = (nodetype)nodety;
    gen_node_label = (nodeplace)nodepl;

	// Get the Sensor Identifier and Parameter String
	// mySensor = SensorRegistry::getSensor(id, param);
	frozen=false;  //TODO: Maybe change

	//Get a pointer to the trait this node points to
	if (traitnum==0)
    {}
	else {
		curtrait=traits.begin();
		while(((*curtrait)->trait_id)!=traitnum)
			++curtrait;
		nodetrait=(*curtrait);
		trait_id=nodetrait.trait_id;
	}

	override=false;
}

NNode::~NNode() {
	std::vector<Link*>::iterator curlink;

	//Kill off all incoming links
	for(curlink=incoming.begin();curlink!=incoming.end();++curlink) {
		delete (*curlink);
	}
	//if (nodetrait!=0) delete nodetrait;
}

//Returns the type of the node, NEURON or SENSOR
const nodetype NNode::get_type() {
	return type;
}

//If the node is a SENSOR, returns true and loads the value
bool NNode::sensor_load(double value) {
	if (type==SENSOR) {

		//Time delay memory
		last_activation2=last_activation;
		last_activation=activation;

		activation_count++;  //Puts sensor into next time-step
		activation=value;
		return true;
	}
	else return false;
}

// Return activation currently in node, if it has been activated
double NNode::get_active_out() {
	if (activation_count>0)
		return activation;
	else return 0.0;
}

// Return activation currently in node from PREVIOUS (time-delayed) time step,
// if there is one
double NNode::get_active_out_td() {
	if (activation_count>1)
		return last_activation;
	else return 0.0;
}

// This recursively flushes everything leading into and including this NNode, including recurrencies
void NNode::flushback() {
	std::vector<Link*>::iterator curlink;

	//A sensor should not flush black
	if (type!=SENSOR) {

		if (activation_count>0) {
			activation_count=0;
			activation=0;
			last_activation=0;
			last_activation2=0;
		}

		//Flush back recursively
		for(curlink=incoming.begin();curlink!=incoming.end();++curlink) {
			//Flush the link itself (For future learning parameters possibility) 
			(*curlink)->added_weight=0;
			if ((((*curlink)->in_node)->activation_count>0))
				((*curlink)->in_node)->flushback();
		}
	}
	else {
		//Flush the SENSOR
		activation_count=0;
		activation=0;
		last_activation=0;
		last_activation2=0;

	}

}

// Reserved for future system expansion
void NNode::derive_trait(Trait *curtrait) {

	if (curtrait!=0) {
		for (int count=0;count<NEAT::num_trait_params;count++)
			params[count]=(curtrait->params)[count];
	}
	else {
		for (int count=0;count<NEAT::num_trait_params;count++)
			params[count]=0;
	}

	if (curtrait!=0)
		trait_id=curtrait->trait_id;
	else trait_id=1;

}

// Returns the gene that created the node
NNode *NNode::get_analogue() {
	return analogue;
}

// Force an output value on the node
void NNode::override_output(double new_output) {
	override_value=new_output;
	override=true;
}

// Tell whether node has been overridden
bool NNode::overridden() {
	return override;
}

// Set activation to the override value and turn off override
void NNode::activate_override() {
	activation=override_value;
	override=false;
}


void NNode::print_to_file(std::ofstream &outFile) {
  outFile<<"node "<<node_id<<" ";
  outFile<<nodetrait.trait_id<<" ";
  outFile<<type<<" ";
  outFile<<gen_node_label<<std::endl;
}