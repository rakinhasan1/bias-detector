#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <cmath>
#include <math.h>

using namespace std;

class node;
class connection;
class layer;
class neuralNetwork;

class connection {
public:
    connection(double w = 0, node* n = nullptr) {
        weight = w;
        next = n;
    }
    double weight;
    node* next;
};

class node {
public:
    node(int v = 0, int numOfConnections = 1) {
        value = v;
        for(int i = 0; i < numOfConnections; i++) {
            connection connection;
            connections.push_back(connection);
        }
    }
    vector<connection> connections;
    double value;
};

class layer {
public:
    layer(int l, int numOfNodes, int numOfNextNodes) {
        level = l;
        for(int i = 0; i < numOfNodes; i++) {
            node node(0, numOfNextNodes);
            nodes.push_back(node);
        }
    }
    vector<node> nodes;
    int level;
};

class neuralNetwork {
public:
    neuralNetwork(int numOfWords, int numOfNodes, int numOfLayers, vector<string> d) {
        for(int i = 0; i < numOfLayers; i++) {
            int numOfNextNodes = 0;
            if (i == 0) {
                if(numOfLayers == 2) {
                    numOfNextNodes = 1;
                } else if(numOfLayers > 2) {
                    numOfNextNodes = numOfNodes;
                }
                layer layer(i, numOfWords, numOfNextNodes);
                layers.push_back(layer);
            } else if(i == numOfLayers-1){
                layer layer(i, 1, 0);
                layers.push_back(layer);
            } else {
                if(i == numOfLayers-2) {
                    numOfNextNodes = 1;
                } else if(i < numOfLayers-2) {
                    numOfNextNodes = numOfNodes;
                }
                layer layer(i, numOfNodes, numOfNextNodes);
                layers.push_back(layer);
            }
        }
        for(int i = 0; i < layers.size()-1; i++) {
            for(int j = 0; j < layers[i].nodes.size(); j++) {
                for(int k = 0; k < layers[i].nodes[j].connections.size(); k++) {
                    layers[i].nodes[j].connections[k].next = &(layers[i+1].nodes[k]);
                }
            }
        }
        numLayers = numOfLayers;
        dictionary = d;
    }
    void train(map<string, int> sentences) {
        for(auto it = sentences.begin(); it != sentences.end(); it++) {
            vector<string> wordsInSentence = encode(it->first);
            for(int i = 0; i < wordsInSentence.size(); i++) {
                layers[0].nodes[distance(dictionary.begin(), find(dictionary.begin(), dictionary.end(), wordsInSentence[i]))].value = 1;
            }
            double outputValue = evaluateOutput();
            pair<string, int> sentenceTarget;
            sentenceTarget.first = it->first;
            sentenceTarget.second = it->second;
            updateWeights(outputValue, sentenceTarget);
            resetNodes();
        }
    }
    vector<string> encode(string sentence) {
        vector<string> words;
        string word = "";
        for(int i = 0; i < sentence.size(); i++) {
            if(sentence[i] < 91 && sentence[i] > 64) {
                word += tolower(sentence[i]);
            } else if(sentence[i] < 123 && sentence[i] > 96) {
                word += sentence[i];
            } else {
                words.push_back(word);
                word = "";
            }
        }
        return words;
    }
    double evaluateOutput() {
        for(int i = 0; i < layers.size()-1; i++) {
            for (int j = 0; j < layers[i].nodes.size(); j++) {
                for (int k = 0; k < layers[i].nodes[j].connections.size(); k++) {
                    (*(layers[i].nodes[j].connections[k].next)).value += (layers[i].nodes[j].value * layers[i].nodes[j].connections[k].weight);
                }
            }
            for(int j = 0; j < layers[i+1].nodes.size(); j++) {
                layers[i+1].nodes[j].value = 1/(1+pow(M_E,-(layers[i+1].nodes[j].value)));
            }
        }
        double output = layers[numLayers-1].nodes[0].value;
        return output;
    }

    void updateWeights(double outputValue, pair<string,int> sentence) {
        double totalError = (pow((sentence.second-outputValue),2))/2;
        for(int i = 0; i < layers.size(); i++) {
            for(int j = 0; j < layers[i].nodes.size(); j++) {
                for(int k = 0; k < layers[i].nodes[j].connections.size(); k++) {
                    layers[i].nodes[j].connections[k].weight += ((sentence.second - outputValue) * outputValue * (1-outputValue) * layers[i].nodes[j].value);
                }
            }
        }
    }

    void resetNodes() {
        for(int i = 0; i < layers.size(); i++) {
            for (int j = 0; j < layers[i].nodes.size(); j++) {
                layers[i].nodes[j].value = 0;
            }
        }
    }

    double testSentence(string sentence) {
        vector<string> wordsInSentence = encode(sentence);
        for(int i = 0; i < wordsInSentence.size(); i++) {
            layers[0].nodes[distance(dictionary.begin(), find(dictionary.begin(), dictionary.end(), wordsInSentence[i]))].value = 1;
        }
        double outputValue = evaluateOutput();
        resetNodes();
        return outputValue;
    }
    int numLayers;
    vector<string> dictionary;
    vector<layer> layers;
};


int main() {
    ifstream fin("OpinionatedSentences.txt");
    map<string, int> sentences;
    while (!fin.eof()) {
        string sentence;
        getline(fin, sentence);
        sentences[sentence] = 1;
    }
    fin.close();
    ifstream finnn("UnopinionatedSentences.txt");
    while (!finnn.eof()) {
        string sentence;
        getline(finnn, sentence);
        sentences[sentence] = 0;
    }
    finnn.close();
    ifstream finn("dictionary.txt");
    vector<string> dictionary;
    int numOfWords = 0;
    while (!finn.eof()) {
        string word;

        getline(finn, word);
        numOfWords++;
        dictionary.push_back(word);
    }
    finn.close();
    //TODO: change reading in dictionary to creating your own dictionary based on the read in sentences.
    int midLayerNodes = 20;
    int numOfLayers = 2;
    neuralNetwork network(numOfWords, midLayerNodes, numOfLayers, dictionary);
    network.train(sentences);
    double correctGuesses = 0;
    double totalGuesses = 0;
    for (auto it = sentences.begin(); it != sentences.end(); it++) {
        if (network.testSentence(it->first) > 0.5) {
            if (it->second == 1) {
                correctGuesses++;
                totalGuesses++;
            } else {
                totalGuesses++;
            }
        } else if (network.testSentence(it->first) < 0.5) {
            if (it->second == 0) {
                correctGuesses++;
                totalGuesses++;
            } else {
                totalGuesses++;
            }
        } else {
            correctGuesses += 0.5;
            totalGuesses++;
        }
    }
    double output = 100 * (correctGuesses / totalGuesses);
    cout << "The neural network guessed " << output << "% correctly." << endl;
    string test="";
    cout << "enter a sentence:" << endl;
    getline(cin, test);
    while (test != ""){
        cout << test << endl;
        std::cout << network.testSentence(test)<<endl;
        cout << "enter a sentence:" << endl;
        getline(cin, test);
   }
    return 0;
}



