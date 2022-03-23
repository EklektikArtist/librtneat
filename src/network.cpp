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

#include "network.h"

using namespace NEAT;

Network::Network(void) {
  name=0;   //Defaults to no name  ..NOTE: TRYING TO PRINT AN EMPTY NAME CAN CAUSE A CRASH
  numnodes=-1;
  numlinks=-1;
  net_id=0;
  adaptable=false;
}

Network::Network(std::vector<NNode> in,std::vector<NNode> out,std::vector<NNode> all,int netid) {
  inputs=in;
  outputs=out;
  all_nodes=all;
  name=0;   //Defaults to no name  ..NOTE: TRYING TO PRINT AN EMPTY NAME CAN CAUSE A CRASH
  numnodes=-1;
  numlinks=-1;
  net_id=netid;
  adaptable=false;
}

Network::~Network() {
			if (name!=0)
				delete [] name;

			destroy();  // Kill off all the nodes and links

		}

// Puts the network back into an initial state
void Network::flush() {
	std::vector<NNode>::iterator curnode;

	for(curnode=outputs.begin();curnode!=outputs.end();++curnode) {
		(curnode)->flushback();
	}
}

// If all output are not active then return true
bool Network::outputsoff() {
	std::vector<NNode>::iterator curnode;

	for(curnode=outputs.begin();curnode!=outputs.end();++curnode) {
		if (((curnode)->activation_count)==0) return true;
	}

	return false;
}

// Activates the net such that all outputs are active
// Returns true on success;
bool Network::activate() {
	std::vector<NNode>::iterator curnode;
	std::vector<Link>::iterator curlink;
	double add_amount;  //For adding to the activesum
	bool onetime; //Make sure we at least activate once
	int abortcount=0;  //Used in case the output is somehow truncated from the network

	//cout<<"Activating network: "<<this->genotype<<endl;

	//Keep activating until all the outputs have become active 
	//(This only happens on the first activation, because after that they
	// are always active)

	onetime=false;

	while(outputsoff()||!onetime) {

		++abortcount;

		if (abortcount==20) {
			return false;
			//cout<<"Inputs disconnected from output!"<<endl;
		}
		//std::cout<<"Outputs are off"<<std::endl;

		// For each node, compute the sum of its incoming activation 
		for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) {
			//Ignore SENSORS

			//cout<<"On node "<<(curnode)->node_id<<endl;

			if (((curnode)->type)!=SENSOR) {
				(curnode)->activesum=0;
				(curnode)->active_flag=false;  //This will tell us if it has any active inputs

				// For each incoming connection, add the activity from the connection to the activesum 
				for(curlink=(curnode->incoming).begin();curlink!=((curnode)->incoming).end();++curlink) {
					//Handle possible time delays
					if (!((*curlink)->time_delay)) {
						add_amount=((*curlink)->weight)*(((*curlink)->in_node)->get_active_out());
						if ((((*curlink)->in_node)->active_flag)||
							(((*curlink)->in_node)->type==SENSOR)) (curnode)->active_flag=true;
						(curnode)->activesum+=add_amount;
						//std::cout<<"Node "<<(curnode)->node_id<<" adding "<<add_amount<<" from node "<<((*curlink)->in_node)->node_id<<std::endl;
					}
					else {
						//Input over a time delayed connection
						add_amount=((*curlink)->weight)*(((*curlink)->in_node)->get_active_out_td());
						(curnode)->activesum+=add_amount;
					}

				} //End for over incoming links

			} //End if (((curnode)->type)!=SENSOR) 

		} //End for over all nodes

		// Now activate all the non-sensor nodes off their incoming activation 
		for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) {

			if (((curnode)->type)!=SENSOR) {
				//Only activate if some active input came in
				if ((curnode)->active_flag) {
					//cout<<"Activating "<<(curnode)->node_id<<" with "<<(curnode)->activesum<<": ";

					//Keep a memory of activations for potential time delayed connections
					(curnode)->last_activation2=(curnode)->last_activation;
					(curnode)->last_activation=(curnode)->activation;

					//If the node is being overrided from outside,
					//stick in the override value
					if ((curnode)->overridden()) {
						//Set activation to the override value and turn off override
						(curnode)->activate_override();
					}
					else {
						//Now run the net activation through an activation function
						if ((curnode)->ftype==SIGMOID)
							(curnode)->activation=NEAT::fsigmoid((curnode)->activesum,4.924273,2.4621365);  //Sigmoidal activation- see comments under fsigmoid
					}
					//cout<<(curnode)->activation<<endl;

					//Increment the activation_count
					//First activation cannot be from nothing!!
					(curnode)->activation_count++;
				}
			}
		}

		onetime=true;
	}

	if (adaptable) {

	  //std::cout << "ADAPTING" << std:endl;

	  // ADAPTATION:  Adapt weights based on activations 
	  for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) {
	    //Ignore SENSORS
	    
	    //cout<<"On node "<<(curnode)->node_id<<endl;
	    
	    if (((curnode)->type)!=SENSOR) {
	      
	      // For each incoming connection, perform adaptation based on the trait of the connection 
	      for(curlink=((curnode)->incoming).begin();curlink!=((curnode)->incoming).end();++curlink) {
		
		if (((*curlink)->trait_id==2)||
		    ((*curlink)->trait_id==3)||
		    ((*curlink)->trait_id==4)) {
		  
		  //In the recurrent case we must take the last activation of the input for calculating hebbian changes
		  if ((*curlink)->is_recurrent) {
		    (*curlink)->weight=
		      hebbian((*curlink)->weight,maxweight,
			      (*curlink)->in_node->last_activation, 
			      (*curlink)->out_node->get_active_out(),
			      (*curlink)->params[0],(*curlink)->params[1],
			      (*curlink)->params[2]);
		    
		    
		  }
		  else { //non-recurrent case
		    (*curlink)->weight=
		      hebbian((*curlink)->weight,maxweight,
			      (*curlink)->in_node->get_active_out(), 
			      (*curlink)->out_node->get_active_out(),
			      (*curlink)->params[0],(*curlink)->params[1],
			      (*curlink)->params[2]);
		  }
		}
		
	      }
	      
	    }
	    
	  }
	  
	} //end if (adaptable)

	return true;  
}

// Takes an array of sensor values and loads it into SENSOR inputs ONLY
void Network::load_sensors(double *sensvals) {
	//int counter=0;  //counter to move through array
	std::vector<NNode*>::iterator sensPtr;

	for(sensPtr=inputs.begin();sensPtr!=inputs.end();++sensPtr) {
		//only load values into SENSORS (not BIASes)
		if (((*sensPtr)->type)==SENSOR) {
			(*sensPtr)->sensor_load(*sensvals);
			sensvals++;
		}
	}
}

// Destroy will find every node in the network and subsequently
// delete them one by one.  Since deleting a node deletes its incoming
// links, all nodes and links associated with a network will be destructed
// Note: Traits are parts of genomes and not networks, so they are not
//       deleted here
void Network::destroy() {
	std::vector<NNode*>::iterator curnode;
	std::vector<NNode*>::iterator location;
	std::vector<NNode*> seenlist;  //List of nodes not to doublecount

	// Erase all nodes from all_nodes list 

	for(curnode=all_nodes.begin();curnode!=all_nodes.end();++curnode) {
		delete (curnode);
	}


	// ----------------------------------- 

	//  OLD WAY-the old way collected the nodes together and then deleted them

	//for(curnode=outputs.begin();curnode!=outputs.end();++curnode) {
	//cout<<seenstd::vector<<endl;
	//cout<<curnode<<endl;
	//cout<<curnode->node_id<<endl;

	//  location=find(seenlist.begin(),seenlist.end(),(curnode));
	//  if (location==seenlist.end()) {
	//    seenlist.push_back(curnode);
	//    destroy_helper((curnode),seenlist);
	//  }
	//}

	//Now destroy the seenlist, which is all the NNodes in the network
	//for(curnode=seenlist.begin();curnode!=seenlist.end();++curnode) {
	//  delete (curnode);
	//}
}

// This checks a POTENTIAL link between a potential in_node and potential out_node to see if it must be recurrent 
bool Network::is_recur(NNode *potin_node,NNode *potout_node,int &count,int thresh) {
	std::vector<Link*>::iterator curlink;


	++count;  //Count the node as visited

	if (count>thresh) {
		//cout<<"returning false"<<endl;
		return false;  //Short out the whole thing- loop detected
	}

	if (potin_node==potout_node) return true;
	else {
		//Check back on all links...
		for(curlink=(potin_node->incoming).begin();curlink!=(potin_node->incoming).end();curlink++) {
			//But skip links that are already recurrent
			//(We want to check back through the forward flow of signals only
			if (!((*curlink)->is_recurrent)) {
				if (is_recur((*curlink)->in_node,potout_node,count,thresh)) return true;
			}
		}
		return false;
	}
}