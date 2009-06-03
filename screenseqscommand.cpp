/*
 *  screenseqscommand.cpp
 *  Mothur
 *
 *  Created by Pat Schloss on 6/3/09.
 *  Copyright 2009 Patrick D. Schloss. All rights reserved.
 *
 */

#include "screenseqscommand.h"

//***************************************************************************************************************

ScreenSeqsCommand::ScreenSeqsCommand(){
	try {
		globaldata = GlobalData::getInstance();
		
		if(globaldata->getFastaFile() != "")		{	readSeqs = new ReadFasta(globaldata->inputFileName);	}
		else if(globaldata->getNexusFile() != "")	{	readSeqs = new ReadNexus(globaldata->inputFileName);	}
		else if(globaldata->getClustalFile() != "") {	readSeqs = new ReadClustal(globaldata->inputFileName);	}
		else if(globaldata->getPhylipFile() != "")	{	readSeqs = new ReadPhylip(globaldata->inputFileName);	}
		
		readSeqs->read();
		db = readSeqs->getDB();
		numSeqs = db->size();
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the SeqCoordCommand class Function SeqCoordCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the SeqCoordCommand class function SeqCoordCommand. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}	
}

//***************************************************************************************************************

ScreenSeqsCommand::~ScreenSeqsCommand(){
	delete readSeqs;
}

//***************************************************************************************************************

int ScreenSeqsCommand::execute(){
	try{
		int startPos, endPos, maxAmbig, maxHomoP, minLength, maxLength;
		convert(globaldata->getStartPos(), startPos);
		convert(globaldata->getEndPos(), endPos);
		convert(globaldata->getMaxAmbig(), maxAmbig);
		convert(globaldata->getMaxHomoPolymer(), maxHomoP);
		convert(globaldata->getMinLength(), minLength);
		convert(globaldata->getMaxLength(), maxLength);
		
		set<string> badSeqNames;
		
		string goodSeqFile = getRootName(globaldata->inputFileName) + "good" + getExtension(globaldata->inputFileName);
		string badSeqFile = getRootName(globaldata->inputFileName) + "bad" + getExtension(globaldata->inputFileName);
		
		ofstream goodSeqOut;	openOutputFile(goodSeqFile, goodSeqOut);
		ofstream badSeqOut;		openOutputFile(badSeqFile, badSeqOut);		

		for(int i=0;i<numSeqs;i++){
			Sequence currSeq = db->get(i);
			bool goodSeq = 1;		//	innocent until proven guilty
			if(goodSeq == 1 && startPos != -1 && startPos < currSeq.getStartPos())		{	goodSeq = 0;	}
			if(goodSeq == 1 && endPos != -1 && endPos > currSeq.getEndPos())			{	goodSeq = 0;	}
			if(goodSeq == 1 && maxAmbig != -1 && maxAmbig <	currSeq.getAmbigBases())	{	goodSeq = 0;	}
			if(goodSeq == 1 && maxHomoP != -1 && maxHomoP < currSeq.getLongHomoPolymer()){	goodSeq = 0;	}
			if(goodSeq == 1 && minLength != -1 && minLength > currSeq.getNumBases())	{	goodSeq = 0;	}
			if(goodSeq == 1 && maxLength != -1 && maxLength < currSeq.getNumBases())	{	goodSeq = 0;	}
			
			if(goodSeq == 1){
				currSeq.printSequence(goodSeqOut);	
			}
			else{
				currSeq.printSequence(badSeqOut);	
				badSeqNames.insert(currSeq.getName());
			}
		}
		
		if(globaldata->getNameFile() != ""){
			screenNameGroupFile(badSeqNames);
			
		}
		
		return 0;
	}
	catch(exception& e) {
		cout << "Standard Error: " << e.what() << " has occurred in the FilterSeqsCommand class Function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	catch(...) {
		cout << "An unknown error has occurred in the FilterSeqsCommand class function execute. Please contact Pat Schloss at pschloss@microbio.umass.edu." << "\n";
		exit(1);
	}
	
}

//***************************************************************************************************************

void ScreenSeqsCommand::screenNameGroupFile(set<string> badSeqNames){

	ifstream inputNames;
	openInputFile(globaldata->getNameFile(), inputNames);
	set<string> badSeqGroups;
	string seqName, seqList, group;
	set<string>::iterator it;

	string goodNameFile = getRootName(globaldata->getNameFile()) + "good" + getExtension(globaldata->getNameFile());
	string badNameFile = getRootName(globaldata->getNameFile()) + "bad" + getExtension(globaldata->getNameFile());
	
	ofstream goodNameOut;	openOutputFile(goodNameFile, goodNameOut);
	ofstream badNameOut;	openOutputFile(badNameFile, badNameOut);		
	
	while(!inputNames.eof()){
		inputNames >> seqName >> seqList;
		it = badSeqNames.find(seqName);
		
		if(it != badSeqNames.end()){
			badSeqNames.erase(it);
			badNameOut << seqName << '\t' << seqList << endl;
			if(globaldata->getNameFile() != ""){
				int start = 0;
				for(int i=0;i<seqList.length();i++){
					if(seqList[i] == ','){
						badSeqGroups.insert(seqList.substr(start,i-start));
						start = i+1;
					}					
				}
				badSeqGroups.insert(seqList.substr(start,seqList.length()-start));
			}
		}
		else{
			goodNameOut << seqName << '\t' << seqList << endl;
		}
		gobble(inputNames);
	}
	inputNames.close();
	goodNameOut.close();
	badNameOut.close();
	
	if(globaldata->getGroupFile() != ""){
		
		ifstream inputGroups;
		openInputFile(globaldata->getGroupFile(), inputGroups);

		string goodGroupFile = getRootName(globaldata->getGroupFile()) + "good" + getExtension(globaldata->getGroupFile());
		string badGroupFile = getRootName(globaldata->getGroupFile()) + "bad" + getExtension(globaldata->getGroupFile());
		
		ofstream goodGroupOut;	openOutputFile(goodGroupFile, goodGroupOut);
		ofstream badGroupOut;	openOutputFile(badGroupFile, badGroupOut);		
		
		while(!inputGroups.eof()){
			inputGroups >> seqName >> group;

			it = badSeqGroups.find(seqName);
			
			if(it != badSeqGroups.end()){
				badSeqGroups.erase(it);
				badGroupOut << seqName << '\t' << group << endl;
			}
			else{
				goodGroupOut << seqName << '\t' << group << endl;
			}
			gobble(inputGroups);
		}
		inputGroups.close();
		goodGroupOut.close();
		badGroupOut.close();
	}
}

//***************************************************************************************************************


