#include <cstdlib>
#include <direct.h>
#include <vector>

#include <iostream>
#include <fstream>
#include <string>

#include <map>

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;


void GetDirectoryContents(const fs::path& path, std::vector<std::string>& v) {
	for (const auto & entry : fs::directory_iterator(path)) {
		std::string file = entry.path().string();
		v.push_back(file);
	}
}

// for string delimiter
std::vector<std::string> split(std::string s, std::string delimiter) {
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string token;
	std::vector<std::string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

bool ReadBOM(std::ifstream & is){
	char const c0 = is.get();
	if (c0 != '\xEF') {
		is.putback(c0);
		return false;
	}

	char const c1 = is.get();
	if (c1 != '\xBB') {
		is.putback(c1);
		is.putback(c0);
		return false;
	}

	char const c2 = is.get();
	if (c2 != '\xBF') {
		is.putback(c1);
		is.putback(c0);
		return false;
	}

	return true; 
}

std::vector<std::string> LoadPakLog(std::string file, std::vector<std::string>& files) {
	std::vector<std::string> filesInPak;

	std::ifstream infile(file);
	std::string str;

	ReadBOM(infile);

	while (std::getline(infile, str)) {
		auto item = split(str, "\"");
		std::string fileInLine = item[1];
		files.push_back(fileInLine);
		filesInPak.push_back(fileInLine);
	}


	infile.close();

	return filesInPak;
}

void CheckMainFileList(std::vector<std::string> &fileList) {
	std::vector<std::string> duplicates;
	for (auto &i : fileList) {
		int count = 0;
		for (auto &b : fileList) {
			if (b == i) {
				count++;
				if (count > 1) {
					break;
				}
			}
		}
		if (count > 1) {
			duplicates.push_back(i);
		}
	}


	std::sort(fileList.begin(), fileList.end());
	fileList.erase(std::unique(fileList.begin(), fileList.end()), fileList.end());

}

void CheckForDuplicates(std::vector<std::string> &files, std::vector<std::vector<std::string>> &filesInPak, std::vector<std::string> &pakLogFiles) {

	//create file stream
	std::ofstream output;
	output.open("duplicateAssets.csv", std::ofstream::out | std::ofstream::trunc);

	//loop through all files
	for (auto &currentFile : files) {
		//loop through paks to see if file exists
		int counter = 0;
		std::string line = currentFile;
		for (unsigned int index = 0; index < filesInPak.size(); index++) {
			for (auto &fileInPak : filesInPak[index]) {
				if (fileInPak == currentFile) {
					counter++;
					line = line + "," + pakLogFiles[index];
				}
			}
		}
		if (counter > 1) {
			output << line << "\n";
		}
	}
	std::cout << "checking complete." << std::endl;
	output.close();
}

int main(){
	std::experimental::filesystem::path pakPath = "PakLogs";
	std::vector<std::string> pakLogFiles;

	GetDirectoryContents(pakPath, pakLogFiles);
	
	std::vector<std::string> files;
	std::vector<std::vector<std::string>> filesInPak;

	std::cout << "opening pak logs" << std::endl;
	for (auto &i : pakLogFiles) {
		std::vector<std::string> filesInThisPak;
		filesInThisPak = LoadPakLog(i, files);
		filesInPak.push_back(filesInThisPak);
	}

	CheckMainFileList(files);

	std::cout << "checking files for duplicates" << std::endl;
	CheckForDuplicates(files, filesInPak, pakLogFiles);
	std::cout << files.size() << " total files." << std::endl;
	system("pause");
}
