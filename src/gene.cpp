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

#include "gene.h"
#include <sstream>

using namespace NEAT;

Gene::Gene(Trait tp,double w,NNode inode,NNode onode,bool recur,double innov,double mnum) {
	lnk=*new Link(tp,w,inode,onode,recur);
	innovation_num=innov;
	mutation_num=mnum;

	enable=true;

	frozen=false;
}

Gene::Gene(void) {
	//cout<<"Trying to attach nodes: "<<inode<<" "<<onode<<endl;
	innovation_num=0;
	mutation_num=0;
	enable=true;

	frozen=false;
}

Gene::Gene(Gene g,Trait tp,NNode inode,NNode onode) {
	//cout<<"Trying to attach nodes: "<<inode<<" "<<onode<<endl;
	lnk=*new Link(tp,(g.lnk).weight,inode,onode,(g.lnk).is_recurrent);
	innovation_num=g.innovation_num;
	mutation_num=g.mutation_num;
	enable=g.enable;

	frozen=g.frozen;
}

Gene::Gene(const char *argline, std::vector<Trait> &traits, std::vector<NNode> &nodes) {
	//Gene parameter holders
	int traitnum;
	int inodenum;
	int onodenum;
	NNode inode;
	NNode onode;
	double weight;
	int recur;
	Trait traitptr;

	std::vector<Trait>::iterator curtrait;
	std::vector<NNode>::iterator curnode;

	//Get the gene parameters

    std::stringstream ss(argline);

	//char curword[128];
	//char delimiters[] = " \n";
	//int curwordnum = 0;

	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//traitnum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//inodenum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//onodenum = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//weight = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//recur = atoi(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//innovation_num = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//mutation_num = atof(curword);
	//strcpy(curword, NEAT::getUnit(argline, curwordnum++, delimiters));
	//enable = (bool)(atoi(curword));

    ss >> traitnum >> inodenum >> onodenum >> weight >> recur >> innovation_num >> mutation_num >> enable;
    //std::cout << traitnum << " " << inodenum << " " << onodenum << " ";
    //std::cout << weight << " " << recur << " " << innovation_num << " ";
    //std::cout << mutation_num << " " << enable << std::endl;

	frozen=false; //TODO: MAYBE CHANGE

	//Get a pointer to the linktrait

	curtrait=traits.begin();
	while(((*curtrait).trait_id)!=traitnum)
		++curtrait;
	traitptr=(*curtrait);

	//Get a pointer to the input node
	curnode=nodes.begin();
	while(((curnode)->node_id)!=inodenum)
		++curnode;
	inode=(*curnode);

	//Get a pointer to the output node
	curnode=nodes.begin();
	while(((curnode)->node_id)!=onodenum)
		++curnode;
	onode=(*curnode);

	lnk=*new Link(traitptr,weight,inode,onode,recur);
}

Gene::~Gene() {
	delete &lnk;
}

void Gene::print_to_file(std::ofstream &outFile) {
  outFile<<"gene ";
  //Start off with the trait number for this gene
  outFile<<((lnk.linktrait).trait_id)<<" ";
  outFile<<(lnk.in_node).node_id<<" ";
  outFile<<(lnk.out_node).node_id<<" ";
  outFile<<(lnk.weight)<<" ";
  outFile<<(lnk.is_recurrent)<<" ";
  outFile<<innovation_num<<" ";
  outFile<<mutation_num<<" ";
  outFile<<enable<<std::endl;
}
