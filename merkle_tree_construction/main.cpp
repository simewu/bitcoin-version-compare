#include "handshake_proof_merklecpp.h"
#include "openssl/sha.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

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
	std::cout << "Sorting files..." << std::endl;
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
	directories.push_back("bitcoin-23.0");

	for(int d = 0; d < directories.size(); d++) {
		std::string directory = "../" + directories.at(d) + "/src";
		std::cout << "Processing directory \"" << directory << "\"..." << std::endl;

		std::string regexToIncludeStr = ".*(\\.cpp|\\.c|\\.h|\\.cc|\\.py|\\.sh)";
		std::string regexToIgnoreStr = ".*(/build-aux/|/config/|-config.h|/minisketch/|/obj/|/qt/|/univalue/gen/|/zqm/).*";

		// Get the list of code file names
		std::vector<std::string> files = getFiles(directory, regexToIncludeStr, regexToIgnoreStr);
		std::vector<std::string> hashes (files.size());
		// Compute the hash of the files
		for(int i = 0; i < files.size(); i++) {
			//totalBytes += getContents(files.at(i)).length();
			hashes.at(i) = sha256(getContents(files.at(i)));

			//std::cout << "File \"" << files.at(i) << "\" has has \"" << hashes.at(i) << "\"" << std::endl;
		}
		// Set the initial ID
		hashes.insert(hashes.begin(), "0000000000000000000000000000000000000000000000000000000000000000");
		// Adjust the size to make it a full binary tree
		int targetSize = nextPowerOfTwo(hashes.size()), i = 0;
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

		// Update the ID
		updateHashAtIndex(tree, 0, "0000000000000000000000000000000000000000000000000000000000000000");

		if(tree.root().to_string() == "db690426d6b029f9cf116e4b15895ef8105564762fd49408e026cc04fc579f4e") {
			std::cout << "Correct version!" << std::endl;
		} else {
			std::cout << "Incorrect version: " << tree.root().to_string() << std::endl;
		}

		//std::cout << "Total file bytes: " << totalBytes << std::endl;
		std::cout << "Total tree bytes: " << tree.to_string().length() << std::endl;

		// auto root = tree.root();
		// auto path = tree.path(0);
		// assert(path->verify(root));

	}
	return 0;
}