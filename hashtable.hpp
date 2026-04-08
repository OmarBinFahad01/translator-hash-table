
#ifndef _HASHTABLE_HPP
#define _HASHTABLE_HPP
#include <vector>
#include <string>
using namespace std;


class Translation
{
	private:
		string language;
		vector<string> meanings;
	public:
		Translation(string meanings,string language);
		void addMeaning(string newMeanings);
		friend class Entry;
		friend class HashTable;
};

class Entry
{
	private:
		string word;
		vector<Translation> translations;
		bool deleted;  // is the bucket is available to be reused after being deleted
	public:
		Entry(string word, string meanings,string language);
		void addTranslation(string newMeanings, string language);
		void print();
		friend class HashTable;
};

class HashTable
{
	private:
			Entry **buckets;		        			// Array of Pointers to Entries for Linear and Quadratic Probing
			unsigned int size;					   		//Current Size of HashTable
			unsigned int capacity;				    	// Total Capacity of HashTable
			unsigned int collisions; 					// Total Number of Collisions
			// current hash function mode 0 polynomial 1 cyclic shift 2 summation
			int hashType;
			// helper methods
			string toLower(const string& s);
			string trim(const string& s);
			int findIndex(const string& word, unsigned long &comparisons);
			// helper hash implementations for different strategies
			unsigned long hashCodePolynomial(const string& word);
			unsigned long hashCodeCyclic(const string& word);
			unsigned long hashCodeDivision(const string& word);
	public:
			// constructor to create hash table with given capacity
			HashTable(int capacity);
			// hash function for a given word
			unsigned long hashCode(string word);
			// return number of active entries
			unsigned int getSize();
			// return total number of collisions
			unsigned int getCollisions();
		// set hash function type for this table
		void setHashType(int type);
		// read data from file and insert into table
		void import(string path);
		// insert or update a word with meanings and language
		void insert(string word, string meanings,string language);
		// delete a word lazily from the table
		void delWord(string word);
		// delete all meanings for a word in a language
		void delTranslation(string word, string language);
		// delete one meaning of a word in a language
		void delMeaning(string word, string meaning, string language);
		// export all records for a language to a file
		void exportData(string language, string filePath);
			// find a word and print its translations
			void find(string word);
			// release all allocated memory
			~HashTable();
			//You may add more helper methods however you are not allowed to change the 
			//signature of the already given methods. 
	};
	//================================================================================
	//Define the methods below this line


	//================ Translation ==========================
	Translation::Translation(string meanings, string language)
	{
		// store language for this translation
		this->language = language;
		// temporary buffer to build one meaning at a time
		string current;
		for(char ch : meanings)
		{
			if(ch == ';')
			{
				if(!current.empty())
				{
					this->meanings.push_back(current);
					current.clear();
				}
			}
			else
			{
				current.push_back(ch);
			}
		}
		if(!current.empty())
			this->meanings.push_back(current);
	}

	void Translation::addMeaning(string newMeanings)
	{
		// temporary buffer to build one new meaning at a time
		string current;
		for(char ch : newMeanings)
		{
			if(ch == ';')
			{
				if(!current.empty())
				{
					bool exists = false;
					for(const auto &m : meanings)
						if(m == current)
						{
							exists = true;
							break;
						}
					if(!exists)
						meanings.push_back(current);
					current.clear();
				}
			}
			else
			{
				current.push_back(ch);
			}
		}
		if(!current.empty())
		{
			bool exists = false;
			for(const auto &m : meanings)
				if(m == current)
				{
					exists = true;
					break;
				}
			if(!exists)
				meanings.push_back(current);
		}
	}

	//================ Entry ==========================

	Entry::Entry(string word, string meanings, string language)
	{
		// set the word stored in this entry
		this->word = word;
		// mark entry as not deleted initially
		deleted = false;
		Translation t(meanings, language);
		translations.push_back(t);
	}

	void Entry::addTranslation(string newMeanings, string language)
	{
		// iterate over existing translations for this word
		for(auto &t : translations)
		{
			if(t.language == language)
			{
				t.addMeaning(newMeanings);
				return;
			}
		}
		Translation t(newMeanings, language);
		translations.push_back(t);
	}

	void Entry::print()
	{
		// iterate over all translations of this entry
		for(const auto &t : translations)
		{
			if(t.meanings.empty())
				continue;

			string lang = t.language;
			// capitalize first character of language name
			if(!lang.empty())
				lang[0] = std::toupper(static_cast<unsigned char>(lang[0]));

			cout<<lang<<"  : ";
			for(size_t i = 0; i < t.meanings.size(); ++i)
			{
				cout<<t.meanings[i];
				if(i + 1 < t.meanings.size())
					cout<<"; ";
			}
			cout<<endl;
		}
	}

	//================ HashTable helpers ==========================

	string HashTable::toLower(const string& s)
	{
		// create copy of input string
		string res = s;
		// convert each character to lowercase
		for(char &c : res)
			c = std::tolower(static_cast<unsigned char>(c));
		return res;
	}

	string HashTable::trim(const string& s)
	{
		// index of first non whitespace character
		size_t start = 0;
		while(start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
			start++;
		if(start == s.size()) return "";
		// index of last non whitespace character
		size_t end = s.size() - 1;
		while(end > start && std::isspace(static_cast<unsigned char>(s[end])))
			end--;
		return s.substr(start, end - start + 1);
	}

	int HashTable::findIndex(const string& word, unsigned long &comparisons)
	{
		// reset comparison counter at start of search
		comparisons = 0;
		// handle case where table has zero capacity
		if(capacity == 0) return -1;
		// canonical key used for lookup
		string key = toLower(trim(word));
		// starting index computed from hash function
		unsigned long index = hashCode(key);
		// remember first index to know when we wrapped around
		unsigned long start = index;
		while(true)
		{
			Entry *e = buckets[index];
			if(e == nullptr)
				return -1; // not found
			if(!e->deleted)
			{
				// count comparison only for active bucket
				comparisons++;
				if(toLower(e->word) == key)
					return static_cast<int>(index);
			}
			// move to next bucket with wrap around
			index = (index + 1) % capacity;
			if(index == start)
				return -1;
		}
	}

	//================ HashTable ==========================

	HashTable::HashTable(int capacity)
	{
		// store maximum number of buckets for this table
		this->capacity = capacity;
		// start with zero active entries
		size = 0;
		// start with zero recorded collisions
		collisions = 0;
		// default hash type uses polynomial hashing
		hashType = 0;
		// allocate array that will hold entry pointers
		buckets = new Entry*[capacity];
		for(int i=0;i<capacity;++i)
			buckets[i] = nullptr;
	}

	unsigned long HashTable::hashCodePolynomial(const string& cleanWord)
	{
		// normalize word so case differences are ignored
		// start with empty hash value
		unsigned long hash = 0;
		// constant base factor for polynomial hash
		const unsigned long p = 31;
		for(char c : cleanWord)
		{
			hash = (hash * p + static_cast<unsigned char>(c)) % capacity;
		}
		return hash;
	}

	unsigned long HashTable::hashCodeCyclic(const string& cleanWord)
	{
		// start cyclic shift hash value at zero
		unsigned long hash = 0;
		for(char c : cleanWord)
		{
			// shift bits left and wrap high bits
			hash = (hash << 5) | (hash >> 27);
			// add current character value
			hash += static_cast<unsigned char>(c);
		}
		return hash % capacity;
	}

	unsigned long HashTable::hashCodeDivision(const string& cleanWord)
	{
		// build integer key from characters
		unsigned long key = 0;
		for(char c : cleanWord)
		{
			// multiply current key and add character value
			key = key * 131 + static_cast<unsigned char>(c);
		}
		// apply division method using table capacity
		return key % capacity;
	}

	unsigned long HashTable::hashCode(string word)
	{
		// normalize input word once for all hash strategies
		string clean = toLower(trim(word));
		// select hash implementation based on configured type
		if(hashType == 1)
			return hashCodeCyclic(clean);
		if(hashType == 2)
			return hashCodeDivision(clean);
		return hashCodePolynomial(clean);
	}

	void HashTable::setHashType(int type)
	{
		// store non negative hash type value
		if(type < 0 || type > 2) type = 0;
		hashType = type;
	}

unsigned int HashTable::getSize()
	{
		// provide current number of active entries
		return size;
	}

	unsigned int HashTable::getCollisions()
	{
		// provide total collisions recorded during insert operations
		return collisions;
	}

void HashTable::import(string path)
	{
		// open translation file for reading from disk
		ifstream fin(path.c_str());
		if(!fin.is_open())
		{
			cout<<"Unable to open file: "<<path<<endl;
			return;
		}
		// buffer to store first line containing language
		string languageLine;
		if(!getline(fin, languageLine))
		{
			fin.close();
			return;
		}
		// language text as shown to user
		string languagePrinted = trim(languageLine);
		// lower case language used inside table
		string language = toLower(languagePrinted);
		// buffer to hold each following line
		string line;
		// number of words successfully imported
		unsigned int wordsImported = 0;
	while(getline(fin, line))
		{
			// remove leading and trailing spaces from whole line
			line = trim(line);
			if(line.empty()) continue;
			size_t pos = line.find(':');
			if(pos == string::npos) continue;
			// part before colon is the english word
			string word = trim(line.substr(0,pos));
			// part after colon contains meanings list
			string meanings = trim(line.substr(pos+1));
			if(!word.empty() && !meanings.empty())
			{
				insert(word, meanings, language);
				wordsImported++;
			}
		}
		fin.close();
		if(wordsImported > 0)
		{
			cout<<wordsImported<<" "<<languagePrinted<<" words have been imported successfully."<<endl;
		}
	}

	void HashTable::insert(string word, string meanings, string language)
	{
		// check if there is free space to insert another entry
		if(size >= capacity)
		{
			cout<<"HashTable is full. Cannot insert."<<endl;
			return;
		}
		// normalize word for case insensitive storage
		word = toLower(trim(word));
		// normalize language for consistent comparisons
		language = toLower(trim(language));
		// remove extra spaces around meanings
		meanings = trim(meanings);

		// starting index computed from hash function
		unsigned long index = hashCode(word);
		// remember first deleted index we can reuse later
		int firstDeleted = -1;
		while(true)
		{
			Entry *e = buckets[index];
			if(e == nullptr)
			{
				// choose index of first deleted bucket or current position
				int insertIndex = (firstDeleted != -1) ? firstDeleted : static_cast<int>(index);
				if(buckets[insertIndex] == nullptr)
					buckets[insertIndex] = new Entry(word, meanings, language);
				else
				{
					// reuse lazily deleted entry object for new word data
					buckets[insertIndex]->word = word;
					buckets[insertIndex]->translations.clear();
					buckets[insertIndex]->deleted = false;
					buckets[insertIndex]->addTranslation(meanings, language);
				}
				size++;
				return;
			}
			if(e->deleted)
			{
				// record first deleted position encountered during probing
				if(firstDeleted == -1)
					firstDeleted = static_cast<int>(index);
			}
			else
			{
				if(toLower(e->word) == word)
				{
					// extend meanings of existing word in given language
					e->addTranslation(meanings, language);
					return;
				}
				// increase collision count for occupied bucket with different key
				collisions++;
			}
			index = (index + 1) % capacity;
		}
	}

	void HashTable::delWord(string word)
	{
		// track how many comparisons find performs
		unsigned long comps = 0;
		// locate index of the word using helper
		int idx = findIndex(word, comps);
		// trimmed word used for messages
		string originalWord = trim(word);
		if(idx == -1)
		{
			cout<<originalWord<<" not found in the Dictionary."<<endl;
			return;
		}
		Entry *e = buckets[idx];
		if(e && !e->deleted)
		{
			// mark entry as deleted without removing it physically
			e->deleted = true;
			size--;
			cout<<originalWord<<" has been successfully deleted from the Dictionary."<<endl;
		}
		else
		{
			cout<<originalWord<<" not found in the Dictionary."<<endl;
		}
	}

	void HashTable::delTranslation(string word, string language)
	{
		// count comparisons performed while locating word
		unsigned long comps = 0;
		int idx = findIndex(word, comps);
		if(idx == -1)
		{
			cout<<"Translation not found in the Dictionary."<<endl;
			return;
		}
		// normalize language for comparison
		language = toLower(trim(language));
		Entry *e = buckets[idx];
		if(e && !e->deleted)
		{
			bool foundLang = false;
			for(auto it = e->translations.begin(); it != e->translations.end(); ++it)
			{
				if(toLower(it->language) == language)
				{
					// erase translation object for the requested language
					e->translations.erase(it);
					foundLang = true;
					break;
				}
			}
			if(!foundLang)
			{
				cout<<"Translation not found in the Dictionary."<<endl;
				return;
			}
			if(e->translations.empty())
			{
				// mark entry as deleted when no translations remain
				e->deleted = true;
				size--;
			}
			cout<<"Translation has been successfully deleted from the Dictionary."<<endl;
		}
		else
		{
			cout<<"Translation not found in the Dictionary."<<endl;
		}
	}

	void HashTable::delMeaning(string word, string meaning, string language)
	{
		// track comparisons used to locate entry
		unsigned long comps = 0;
		int idx = findIndex(word, comps);
		if(idx == -1)
		{
			cout<<"Meaning not found in the Translation."<<endl;
			return;
		}
		// normalize language for comparisons
		language = toLower(trim(language));
		// trimmed meaning used when matching meanings
		meaning = trim(meaning);
		Entry *e = buckets[idx];
		if(e && !e->deleted)
		{
			bool removed = false;
			for(auto tIt = e->translations.begin(); tIt != e->translations.end(); ++tIt)
			{
				if(toLower(tIt->language) == language)
				{
					// reference to list of meanings for this language
					auto &ms = tIt->meanings;
					for(auto mIt = ms.begin(); mIt != ms.end(); )
					{
						if(toLower(*mIt) == toLower(meaning))
						{
							// remove current meaning that matches target
							mIt = ms.erase(mIt);
							removed = true;
						}
						else
							++mIt;
					}
					if(ms.empty())
					{
						// erase translation when its meaning list becomes empty
						e->translations.erase(tIt);
					}
					break;
				}
			}
			if(!removed)
			{
				cout<<"Meaning not found in the Translation."<<endl;
				return;
			}
			if(e->translations.empty())
			{
				// mark whole entry as deleted when no translations remain
				e->deleted = true;
				size--;
			}
			cout<<"Meaning has been successfully deleted from the Translation"<<endl;
		}
		else
		{
			cout<<"Meaning not found in the Translation."<<endl;
		}
	}

	void HashTable::exportData(string language, string filePath)
	{
		// normalize language argument for lookups
		language = toLower(trim(language));
		// open output file for writing
		ofstream fout(filePath.c_str());
		if(!fout.is_open())
		{
			cout<<"Unable to open file for writing: "<<filePath<<endl;
			return;
		}
		// first line of output stores language name
		fout<<language<<endl;
		// counter for exported records
		unsigned int records = 0;
		for(unsigned int i=0;i<capacity;++i)
		{
			// read entry pointer from current bucket
			Entry *e = buckets[i];
			if(e == nullptr || e->deleted) continue;
			for(const auto &t : e->translations)
			{
				if(toLower(t.language) == language && !t.meanings.empty())
				{
					// output english word followed by colon
					fout<<e->word<<":";
					for(size_t j=0;j<t.meanings.size();++j)
					{
						fout<<t.meanings[j];
						if(j+1<t.meanings.size()) fout<<";";
					}
					fout<<endl;
					records++;
				}
			}
		}
		fout.close();
		cout<<records<<" records have been successfully exported to "<<filePath<<endl;
	}

	void HashTable::find(string word)
	{
		// count how many buckets are probed
		unsigned long comps = 0;
		// search for index of requested word
		int idx = findIndex(word, comps);
		// trimmed word used in printed messages
		string originalWord = trim(word);
		if(idx == -1)
		{
			cout<<originalWord<<" not found in the Dictionary."<<endl;
			return;
		}
		Entry *e = buckets[idx];
		if(e && !e->deleted)
		{
			cout<<originalWord<<" found in the Dictionary after "<<comps<<" comparisons."<<endl;
			e->print();
		}
		else
		{
			cout<<originalWord<<" not found in the Dictionary."<<endl;
		}
	}

	HashTable::~HashTable()
	{
		// ensure bucket array is not null before deleting
		if(buckets)
		{
			// delete each entry pointed to by bucket
			for(unsigned int i=0;i<capacity;++i)
			{
				delete buckets[i];
			}
			delete [] buckets;
		}
	}
// Do not add code below this line
#endif
