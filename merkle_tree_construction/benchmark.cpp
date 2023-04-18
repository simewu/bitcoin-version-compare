#include "handshake_proof_merklecpp.h"
#include "openssl/sha.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#define DEBUG_JUST_PRINT_FILES false
#define NUM_SAMPLES 1000

// Computes the SHA-256 hash of a string
std::string sha256(const std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// The function used to sort the vector of file names
bool pathCompareFunction (std::string a, std::string b) {
	return a < b;
}

// Recursively list the files in a directory
std::vector<std::string> getFiles(std::string directory, std::string regexToIncludeStr, std::string regexToIgnoreStr) {
	std::vector<std::string> files;
	for(std::filesystem::recursive_directory_iterator i(directory), end; i != end; ++i) {
		if(!is_directory(i->path())) {
			std::string str = i->path();
			if(std::regex_match(str, std::regex(regexToIncludeStr)) && !std::regex_match(str, std::regex(regexToIgnoreStr))) {
				files.push_back(str);
			}
		}
	}
	//std::cout << "Sorting files..." << std::endl;
	std::sort(files.begin(),files.end(), pathCompareFunction);
	// for(int i = 0; i < files.size(); i++) {
	// 	std::cout << "Including \"" << files.at(i) << "\"" << std::endl;
	// }
	return files;
}

// Read the contents of a file at a given file path
std::string getContents(std::string filePath) {
	std::string contents = "";
	std::ifstream f(filePath);
	while(f) {
		std::string line;
		getline(f, line);
		contents += line + '\n';
	}
	return contents;
}

// Given a number (e.g. 10) compute the next power of two (e.g. 16)
int nextPowerOfTwo(int num) {
	double n = log2(num);
	return (int)pow(2, ceil(n));
}

// Update the hash at an index within the tree
void updateHashAtIndex(merkle::Tree &tree, int index, std::string hash_string) {
	merkle::TreeT<32, merkle::sha256_compress>::Node* ID = tree.walk_to(index, true, [](merkle::TreeT<32, merkle::sha256_compress>::Node* n, bool go_right) {
		n->dirty = true;
        return true;
    });
    merkle::Tree::Hash newHash(hash_string);
    ID->hash = newHash;
	tree.compute_root();
}





int main() {
	std::string regexToIncludeStr = ".*(\\.cpp|\\.cc|\\.py|\\.h|\\.sh|\\.c|\\.java|\\.mk)";
	std::string regexToIgnoreStr = ".*(/build-aux/|/config/|-config\\.h|/minisketch/|/obj/|/qt/|/univalue/gen/|/zqm/).*";


	std::string fileName = "Algorithm_benchmark_" + std::to_string(NUM_SAMPLES) + ".csv";

	std::ofstream outputFile;
	outputFile.open(fileName);

	std::string header = "";
	header += "Directory,";
	header += "Construct leaves (ms),";
	header += "Form tree (ms),";
	header += "Generate proof (ms),";
	header += "Verify proof (ms),";
	header += "Number of files,";
	outputFile << header << std::endl;


	
	std::vector<std::string> directories;
	directories.push_back("bitcoin-0.10.0");
	directories.push_back("bitcoin-0.10.1");
	directories.push_back("bitcoin-0.10.2");
	directories.push_back("bitcoin-0.10.3");
	directories.push_back("bitcoin-0.10.4");
	directories.push_back("bitcoin-0.11.1");
	directories.push_back("bitcoin-0.11.2");
	directories.push_back("bitcoin-0.12.0");
	directories.push_back("bitcoin-0.12.1");
	directories.push_back("bitcoin-0.13.0");
	directories.push_back("bitcoin-0.13.1");
	directories.push_back("bitcoin-0.13.2");
	directories.push_back("bitcoin-0.14.0");
	directories.push_back("bitcoin-0.14.1");
	directories.push_back("bitcoin-0.14.2");
	directories.push_back("bitcoin-0.14.3");
	directories.push_back("bitcoin-0.15.0");
	directories.push_back("bitcoin-0.15.0.1");
	directories.push_back("bitcoin-0.15.1");
	directories.push_back("bitcoin-0.15.2");
	directories.push_back("bitcoin-0.16.0");
	directories.push_back("bitcoin-0.16.1");
	directories.push_back("bitcoin-0.16.2");
	directories.push_back("bitcoin-0.16.3");
	directories.push_back("bitcoin-0.17.0");
	directories.push_back("bitcoin-0.17.0.1");
	directories.push_back("bitcoin-0.17.1");
	directories.push_back("bitcoin-0.18.0");
	directories.push_back("bitcoin-0.18.1");
	directories.push_back("bitcoin-0.19.0.1");
	directories.push_back("bitcoin-0.19.1");
	directories.push_back("bitcoin-0.20.0");
	directories.push_back("bitcoin-0.20.1");
	directories.push_back("bitcoin-0.21.0");
	directories.push_back("bitcoin-0.21.1");
	directories.push_back("bitcoin-22.0");
	directories.push_back("bitcoin-22.1");
	directories.push_back("bitcoin-23.0");
	directories.push_back("bitcoin-23.1");
	directories.push_back("bitcoin-24.0.1");

	if(DEBUG_JUST_PRINT_FILES) {
		std::string directory = "../" + directories.at(directories.size() - 1) + "/src";
		std::vector<std::string> files = getFiles(directory, regexToIncludeStr, regexToIgnoreStr);
		for(int i = 0; i < files.size(); i++) {
			std::cout << files.at(i) << std::endl;
		}
		throw "DEBUG_JUST_PRINT_FILES flag was set";
	}

	for(int d = 0; d < directories.size(); d++) {
		std::string directory = "../" + directories.at(d) + "/src";
		std::cout << "Processing directory \"" << directory << "\"..." << std::endl;

		std::vector<std::string> files = getFiles(directory, regexToIncludeStr, regexToIgnoreStr);
		int numFiles = files.size();
		
		for(int s = 0; s < NUM_SAMPLES; s++) {
			if(s % 10 == 0) std::cout << "Sample " << std::to_string(s) << std::endl;

			std::clock_t t1_readHashContents = std::clock();
			// Get the list of code file names
			std::vector<std::string> files = getFiles(directory, regexToIncludeStr, regexToIgnoreStr);
			std::vector<std::string> hashes (files.size());
			// Compute the hash of the files
			for(int i = 0; i < files.size(); i++) {
				hashes.at(i) = sha256(getContents(files.at(i)));

				//std::cout << "File \"" << files.at(i) << "\" has has \"" << hashes.at(i) << "\"" << std::endl;
			}
			std::clock_t t2_readHashContents = std::clock();


			std::clock_t t1_formTheTree = std::clock();
			// Set the initial ID
			hashes.insert(hashes.begin(), "0000000000000000000000000000000000000000000000000000000000000000");
			// Adjust the size to make it a full binary tree
			int targetSize = nextPowerOfTwo(hashes.size()), i = 0;
			//std::cout << "    From " << hashes.size() << " to " << targetSize << std::endl; break;
			while(hashes.size() < targetSize) {
				hashes.push_back(hashes.at(i));
				i++;
			}

			// Cybersecurity Lab: Testing a mini tree
			// std::vector<std::string> hashes (16);
			// for(int i = 0; i < 16; i++) {
			// 	hashes.at(i) = sha256(to_string(i + 1));
			// }

			// Convert the hashes to Merkle node objects
			std::vector<merkle::Tree::Hash> leaves (hashes.size());
			for(int i = 0; i < hashes.size(); i++) {
				merkle::Tree::Hash hash(hashes.at(i));
				leaves.at(i) = hash;
			}

			// Create the tree
			merkle::Tree tree;
			tree.insert(leaves);
			std::clock_t t2_formTheTree = std::clock();


			updateHashAtIndex(tree, 0, "0000000000000000000000000000000000000000000000000000000000000000");


			// Update the ID

			std::clock_t t1_changeALeafNode = std::clock();
			updateHashAtIndex(tree, 0, "0000000000000000000000000000000000000000000000000000000000000000");
			std::clock_t t2_changeALeafNode = std::clock();


			bool valid = false;
			std::clock_t t1_verify = std::clock();
			updateHashAtIndex(tree, 0, "0000000000000000000000000000000000000000000000000000000000000001");
			if(tree.root().to_string() == "db690426d6b029f9cf116e4b15895ef8105564762fd49408e026cc04fc579f4e") {
				valid = true;
				//std::cout << "Correct version!" << std::endl;
			} else {
				//std::cout << "Incorrect version: " << tree.root().to_string() << std::endl;
			}
			std::clock_t t2_verify = std::clock();

			double msToReadAndHash = (t2_readHashContents - t1_readHashContents) / (double)(CLOCKS_PER_SEC / 1000);
			double msToFormTree = (t2_formTheTree - t1_formTheTree) / (double)(CLOCKS_PER_SEC / 1000);
			double msToGenerateProof = (t2_changeALeafNode - t1_changeALeafNode) / (double)(CLOCKS_PER_SEC / 1000);
			double msToVerifyProof =  (t2_verify - t1_verify) / (double)(CLOCKS_PER_SEC / 1000);

			std::string row = "";

			row += directory + ",";
			row += std::to_string(msToReadAndHash) + ",";
			row += std::to_string(msToFormTree) + ",";
			row += std::to_string(msToGenerateProof) + ",";
			row += std::to_string(msToVerifyProof) + ",";
			row += std::to_string(numFiles) + ",";

			outputFile << row << std::endl;
		}
	}


	// auto root = tree.root();
	// auto path = tree.path(0);
	// assert(path->verify(root));
	return 0;
}