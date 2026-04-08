//============================================================================
// Name         : Multi-language d
// Author       : Khalid Mengal
// Version      : 1.0
// Date Created : 
// Date Modified: 
// Description  : HashTable Implementation using C++
//============================================================================

#include<iostream>
#include<fstream>
#include<sstream>
#include<math.h>
#include<iomanip>
#include<list>
#include "hashtable.hpp"
using namespace std;
//======================================================
void help()
{
	cout<<"find <word>                         : Search a word and its meanings in the dictionary."<<endl;
	cout<<"import <path>                       : Import a dictionary file."<<endl;
	cout<<"add <word:meaning(s):language>      : Add a word and/or its meanings (separated by ;) to the dictionary."<<endl;
	cout<<"delTranslation <word:language>      : Delete a specific translation of a word from the dictionary."<<endl;
	cout<<"delMeaning <word:meaning:language>  : Delete only a specific meaning of a word from the dictionary."<<endl;
	cout<<"delWord <word>                      : Delete a word and its all translations from the dictionary."<<endl;
	cout<<"export <language:filename>          : Export a given language dictionary to a file."<<endl;
	cout<<"exit                                : Exit the program"<<endl;
}
//======================================================
int main(int argc, char** args)
{
	HashTable myHashTable(1171891);//(2124867);

	cout<<"select hash function 0 polynomial 1 cyclic shift 2 division"<<endl;
	int hashChoice = 0;
	if(!(cin>>hashChoice))
	{
		hashChoice = 0;
	}
	myHashTable.setHashType(hashChoice);
	cin.ignore(numeric_limits<streamsize>::max(), '\n');

	myHashTable.import("en-de.txt");



	cout<<"==================================================="<<endl;
	cout<<"Size of HashTable                = "<<myHashTable.getSize()<<endl;
	cout<<"Total Number of Collisions       = "<<myHashTable.getCollisions()<<endl;
	cout<<"Avg. Number of Collisions/Entry  = "<<setprecision(2)<<float(myHashTable.getCollisions())/myHashTable.getSize()<<endl;	
	cout<<"==================================================="<<endl;
	help();
	string user_input, command, argument1, argument2, argument3;

	while(true)
	{
		user_input=command=argument1=argument2=argument3=""; //clear all old values
		cout<<">";
		getline(cin,user_input);

		stringstream sstr(user_input);
		getline(sstr,command,' ');
		getline(sstr,argument1,':');
		getline(sstr,argument2,':');
		getline(sstr,argument3);
			if(command == "find")   		  myHashTable.find(argument1);
		else if(command == "import")		  myHashTable.import(argument1);
		else if(command == "add")		      myHashTable.insert(argument1,argument2,argument3); //word,meaning,language
		else if(command == "delWord")         myHashTable.delWord(argument1);
		else if(command == "delTranslation")  myHashTable.delTranslation(argument1,argument2);
		else if(command == "delMeaning")      myHashTable.delMeaning(argument1,argument2,argument3);
		else if(command == "export")          myHashTable.exportData(argument1,argument2);
		else if(command == "help")	  	      help();
		else if(command == "exit")	  	      break;
		else cout<<"Invalid command !!!"<<endl;
	}
	return 0;
}
