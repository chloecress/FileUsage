/*
Program Name: fileusage.cpp
Purpose: A command line program to display the occurences and sizes of various files in storage
Author: Chloe Cress
Date: April 12th, 2023
*/
#include <filesystem>
using namespace std::filesystem;
#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
#include <algorithm>
#include <string>
using namespace std;

struct fileData {
	int occurences;
	string ext;
	uintmax_t size;
};

void displayHelp() {
	cout << "fileusage {v3.0.0} (c) 2016-23, Garth Santor" << endl;
	cout << "\n\tUsage: fileusage [--help] [-hrs(x regularexpression)] [folder]" << endl;
	cout << "\tswitches:" << endl;
	cout << "\t\th\thelp" << endl;
	cout << "\t\tr\treverse the order of the listing" << endl;
	cout << "\t\ts\tsort by file sizes" << endl;
	cout << "\t\tx\tfilter with a regular expression" << endl;
	cout << "\n\tfolder" << endl;
	cout << "\t\tstarting folder or current folder if not provided" << endl;
}

string formatWithCommas(intmax_t num) {
	string numStr = to_string(num);
	for (int i = numStr.length() - 3; i > 0; i -= 3) {
		numStr.insert(i, ",");
	}
	return numStr;
}


//Check if extension is in array
//return index or -1
int checkFound(const vector<string>& in, string target) {
	auto result = find(in.begin(), in.end(), target);
	if (result != in.end()) {
		return result - in.begin();
	}
	else {
		return -1;
	}
}

//compare for sort
bool compareItemsByName(const fileData& item1, const fileData& item2) {
	return item1.ext < item2.ext;
}

bool compareItemsBySize(const fileData& item1, const fileData& item2) {
	return item1.size < item2.size;
}

//issue
uintmax_t getDirSize(const path& path) {
	uintmax_t size = 0;
	for (const auto& entry : recursive_directory_iterator(path)) {
		if (is_regular_file(entry)) {
			size += file_size(entry);
		}
	}
	return size;
}

int main(int argc, char* argv[])
{
	path folderPath;


	/*------------------ Get command line input ----------------------------------*/
	//move arguments to vector
	vector<string> args;  //size = 0
	for (int i = 1; i < argc; ++i)
	{
		args.push_back(argv[i]);
	}

	// report help and quit
	vector<string>::iterator locator = find(args.begin(), args.end(), "--help");
	vector<string>::iterator locator2 = find(args.begin(), args.end(), "-h");
	if (locator != args.end())
	{
		displayHelp();
		return 0;
	}
	else if (locator2 != args.end()) {
		displayHelp();
		return 0;
	}


	//look for switches
	string switches;
	string regexVal;
	string pattern;
	string folderIn;
	size_t index;
	bool isReverse = false;
	bool isSizeSort = false;
	bool applyRegex = false;
	if (args.size() > 0) {
		for (int i = 0; i < args.size(); i++) {
			if (args.front().at(0) == '-') {
				switches = args.at(0);
				args.erase(args.begin());
				if (args.size() == 0) {
					break;
				}
			}
			if (args.front().at(0) == '.' || args.front().at(0) == '\\') {
				for (int i = 0; i < args.at(0).length(); i++) {
					regexVal += args.at(0)[i];
				}
				args.erase(args.begin());
				
			}
		}
		
	}
	//test switches entered
	index = switches.find('r');
	if (index != string::npos) {
		isReverse = true;
		switches.erase(index,1);
	}
	index = switches.find('s');
	if (index != string::npos) {
		isSizeSort = true;
		switches.erase(index, 1);
	}
	index = switches.find('x');
	if (index != string::npos) {
		applyRegex = true;
		switches.erase(index, 1);
		pattern= regexVal;
	}

	// if empty continue
	if (args.empty())
	{
		path currPath = current_path();
		folderPath = currPath.string();
	}
	//if folder name if exists set to use
	if (args.size() > 0) {
		folderIn = args.at(0);
		args.erase(args.begin());
		if (is_directory(folderIn)) {
			folderPath = folderIn;
		}
		else {
			cout << "Error canonical: The system cannot find the file specified.: " <<
				folderIn << endl;
			return 1;
		}

	}

	/*------------------ Gather file data ----------------------------------------*/
	//create vector to hold all extensions
	vector<string> extensions;
	vector<uintmax_t> sizes;

	regex reg(pattern);
	//1. go through files and directories
	for (const auto& entry : recursive_directory_iterator(folderPath)) {
		// Check if the entry is a directory
		//if (applyRegex && regex_match(entry.path().extension().string(), pattern)) {
		//	string extension = entry.path().extension().string();
		//	const auto fileSize = file_size(entry);
			//continue;
		//}
		if (is_directory(entry)) {
			//directories are held and a single space
			//extensions.push_back(entry.path().extension().string());
			//sizes.push_back(getDirSize(entry.path()));
		}
		// Check if the entry is a regular file
		else if (is_regular_file(entry)) {
			//cout << "File: " << entry.path().string() << endl;
			string extension = entry.path().extension().string();
			const auto fileSize = file_size(entry);
			if (!extension.empty()) {
				if (applyRegex) {
					if (!regex_match(extension, reg)) {
						continue;
					}
				}
				extensions.push_back(extension);
				sizes.push_back(fileSize);
			}
		}
	}

	//2. extract number of each extension type
	vector<string> uniqueExt;
	vector<int> numExt;
	int numExtSum = 0;
	vector<uintmax_t> sizeCollect;
	uintmax_t sizeSum = 0;
	//Go through each extension collected
	//		- if extension not added: add to unique vector
	//		- if extension already there: add only to count
	int count = 0;
	for (string s : extensions) {
		//check if extension is already collected
		int index = checkFound(uniqueExt, s);
		if (index == -1) {
			uniqueExt.push_back(s);
			numExt.push_back(1);
			sizeCollect.push_back(sizes.at(count));
		}
		else {
			numExt.at(index) += 1;
			sizeCollect.at(index) += sizes.at(count);
		}
		count++;
	}

	//3. put data into a struct vector for easy editing
	string longestStr;

	vector<fileData> fileCollection;
	for (int i = 0; i < uniqueExt.size(); i++) {
		fileData newEntry;
		newEntry.ext = uniqueExt.at(i);
		if (uniqueExt.at(i).length() > longestStr.length()) {
			longestStr = uniqueExt.at(i);
		}
		newEntry.occurences = numExt.at(i);
		numExtSum += numExt.at(i);
		newEntry.size = sizeCollect.at(i);
		sizeSum += sizeCollect.at(i);
		fileCollection.push_back(newEntry);
	}

	// sort alphabetically
	sort(fileCollection.begin(), fileCollection.end(), compareItemsByName);

	//test sorting by size
	if (isSizeSort) {
		sort(fileCollection.begin(), fileCollection.end(), compareItemsBySize);
	}
	if (isReverse) {
		reverse(fileCollection.begin(), fileCollection.end());
	}
	if (applyRegex) {

	}

	//calculate spaces
	int numExtW = to_string(numExtSum).length() + 5;
	int sizeW = formatWithCommas(sizeSum).length() + 5;

	
	//print results
	cout << setw(longestStr.length() + 1) <<right << "Ext" << setw(numExtW) << right << "#" << setw(sizeW) << right << "Total" << endl;
	cout << endl;
	for (int i = 0; i < fileCollection.size(); i++) {
		cout << setw(longestStr.length() + 1) << right << fileCollection.at(i).ext << setw(numExtW) << right
			<< fileCollection.at(i).occurences << setw(sizeW) << right << formatWithCommas(fileCollection.at(i).size) << endl;
	}
	cout << endl;
	cout << setw(longestStr.length() + 1) << right << fileCollection.size() <<setw(numExtW)<< right << numExtSum << setw(sizeW) << right << formatWithCommas(sizeSum) << endl;

	return 0;
}

