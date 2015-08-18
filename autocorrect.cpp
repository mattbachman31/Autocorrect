#include "csvparser.cpp"

#define NUM_THREADS 1
#define WORD_FILE_PATH "/usr/share/dict/words"

using namespace std;

string choiceWord;
int minDistance;
vector<string> minWords;
int numWords = 0;
int numWordsDigested = 0;
ifstream in;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
bool doneWithFile = false;
struct timeval t1, t2;
queue<string> inputParagraph;
unordered_map<string, string> outputParagraph;

void start_the_clock(struct timeval* time1){
	gettimeofday(time1, NULL);
}

double total_time(struct timeval* t1, struct timeval* t2){
	gettimeofday(t2, NULL);
	double ret = ((t2->tv_sec * 1000000 + t2->tv_usec) - (t1->tv_sec * 1000000 + t1->tv_usec));
	ret /= 1000000;
	return ret;
}

void* actualWork(void *);

string autoFill(string* arr, int len);

int main(int argc, char** argv){

	bool autoReplace = false;
	if(argc == 2 && strncmp(argv[1], "-a", 2) == 0) {autoReplace = true;}
	ifstream initial(WORD_FILE_PATH);
	string blah;
	unordered_set<string> constantTimeLookupWords[30];
	while(getline(initial, blah)){
		numWords++;
		constantTimeLookupWords[blah.size()].insert(blah);
	}
	initial.close();
	in.open(WORD_FILE_PATH);

	start_the_clock(&t1);
	
	while(cin >> choiceWord){
		string oldChoiceWord = choiceWord;
		inputParagraph.push(oldChoiceWord);
		minWords.clear();
		minDistance=100;
		pthread_t threads[NUM_THREADS];
		if(constantTimeLookupWords[choiceWord.size()].find(choiceWord) == constantTimeLookupWords[choiceWord.size()].end()){
			numWordsDigested = 0;
			doneWithFile = false;
			for(int tt=0; tt<NUM_THREADS; tt++){
				pthread_create(&threads[tt], NULL, actualWork, NULL);
			}
			actualWork(NULL);
			while(!doneWithFile);
			in.clear();
			in.seekg(0, in.beg);
		}
		if(minDistance != 100) {cout << choiceWord << " is misspelled. Did you mean:" << endl;}
		for(int i=0; i<minWords.size(); i++){
			cout << minWords[i] << endl;
		}
		if(minDistance != 100) {cout << "======================" << endl;}
		if(minDistance != 100 && autoReplace){
			string* toGoogle = new string[minWords.size()];
			for(int u=0; u<minWords.size(); u++){
				toGoogle[u] = minWords[u];
			}
			choiceWord = autoFill(toGoogle, minWords.size());
			cout << "Chose: " << choiceWord << endl;
			cout << "======================" << endl;
			delete [] toGoogle;
		}
		outputParagraph.insert(make_pair<string, string>(oldChoiceWord, choiceWord));
	}
	//thread waiting point
	while(!inputParagraph.empty()){
		string current = inputParagraph.front();
		inputParagraph.pop();
		cout << outputParagraph[current] << " ";
	}
	cout << endl;
	double t = total_time(&t1, &t2);
	in.close();
	cout << t << endl;
}

void* actualWork(void *){
	string editDistanceWord = "initalWordNotused";
	while(!doneWithFile){

		pthread_mutex_lock(&mutex1);
		getline(in, editDistanceWord);
		numWordsDigested++;
		if(numWordsDigested == numWords) {doneWithFile = true;}
		pthread_mutex_unlock(&mutex1);

		tolower(editDistanceWord[0]);
		int arr[editDistanceWord.size()+1][choiceWord.size()+1];
		bool subbed[editDistanceWord.size()+1][choiceWord.size()+1];
		for(int i=0; i<= editDistanceWord.size(); i++){
			arr[i][0] = i;
		}
		for(int i=0; i<= choiceWord.size(); i++){
			arr[0][i] = i;
		}

		for(int i=0; i<= editDistanceWord.size(); i++){
			for(int j=0; j<= choiceWord.size(); j++){
				subbed[i][j] = false;
			}
		}

		for(int i=1; i<= editDistanceWord.size(); i++){
			for(int j=1; j<= choiceWord.size(); j++){
				int insertion = arr[i][j-1] + 1;
				int deletion = arr[i-1][j] + 1;
				int substitute = arr[i-1][j-1];
				if(editDistanceWord[i-1] != choiceWord[j-1]){substitute += 1;}
				int minimum = insertion <= deletion ? insertion : deletion;
				if(substitute <= minimum) {
					subbed[i][j] = true;
					//cout << "I: " << i << " J: " << j << " Subbed " << editDistanceWord[i-1] << " with " << choiceWord[j-1] << endl;
					if(i != 1 && j != 1 && subbed[i-1][j-1] == true){
						if(choiceWord[j-1] == editDistanceWord[i-2] && editDistanceWord[i-1] == choiceWord[j-2] && choiceWord[j-1] != choiceWord[j-2]){
							//check swap case
							//cout << "I: " << i << " J: " << j << "     " << choiceWord[j-1] << " == " << editDistanceWord[i-2] << " & " << editDistanceWord[i-1] << " == " << choiceWord[j-2] << endl;
							subbed[i][j] = false; //NO DOUBLE SWAPPING
							substitute--;
						}
					}
					minimum = substitute;
				}
				arr[i][j] = minimum;
			}
		}



		//strictly for printing out array
		for(int i=1; i<= editDistanceWord.size(); i++){
			for(int j=1; j<= choiceWord.size(); j++){
				//cout << arr[i][j];
				if(i == editDistanceWord.size() && j == choiceWord.size()){
					pthread_mutex_lock(&mutex2);
					if(arr[i][j] < minDistance){
						minDistance = arr[i][j];
						minWords.clear();
						minWords.push_back(editDistanceWord);
					}
					else if(arr[i][j] == minDistance){
						minWords.push_back(editDistanceWord);
					}
					pthread_mutex_unlock(&mutex2);
				}
			}
			//cout << endl;
		}
	}
	return NULL;
}

string autoFill(string* arr, int len){
	if(len == 1) {return arr[0];}
	else if(len <= 5){
		return popularityParse(arr, len);
	}
	else if(len <= 25){
		int index = 0;
		string resultingArr[5];
		while(len > 0){
			resultingArr[index] = autoFill(&arr[5*index], len > 5 ? 5 : len);
			index++;
			len -= 5;
		}
		return autoFill(resultingArr, index);
	}
	else{   //Assume less than 125...
		int index = 0;
		string resultingArr[5];
		while(len > 0){
			resultingArr[index] = autoFill(&arr[25*index], len > 25 ? 25 : len);
			index++;
			len -= 25;
		}
		return autoFill(resultingArr, index);
	}
}
