#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cctype>
#include <unordered_set>
#include <time.h>
#include <sys/time.h>
#include <queue>
#include <unordered_map>

using namespace std;

string popularityParse(string* arr, int len);

string popularityParse(string* arr, int len){
	string commandPartOne = "curl 'https://www.google.com/trends/trendsReport?hl=en-US&q=";
	for(int a=0; a<len; a++){
		if(a != 0) {commandPartOne += "%2C";}
		commandPartOne += arr[a];
	}
	string commandPartTwo = "&tz=Etc%2FGMT%2B7&content=1&export=1' -s -H 'accept-encoding: gzip, deflate, sdch' -H 'accept-language: en-US,en;q=0.8' -H 'upgrade-insecure-requests: 1' -H 'user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.155 Safari/537.36' -H 'accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8' -H 'referer: https://www.google.com/trends/explore' -H 'authority: www.google.com' -H 'cookie: __utma=173272373.1880631044.1439444556.1439444556.1439444556.1; __utmb=173272373.2.10.1439673603; __utmc=173272373; __utmz=173272373.1439444556.1.1.utmcsr=itools.com|utmccn=(referral)|utmcmd=referral|utmcct=/tool/google-trends-topic-popularity; I4SUserLocale=en_US; __utma=173272373.1880631044.1439444556.1439444556.1439444556.1; __utmb=173272373.3.9.1439675319888; __utmc=173272373; __utmz=173272373.1439444556.1.1.utmcsr=itools.com|utmccn=(referral)|utmcmd=referral|utmcct=/tool/google-trends-topic-popularity; SID=DQAAAAsBAADcR5TLAY26lGF_Hw9gl5oXbVJ3DojtAEvwENGTYGuWdvSJjSkqvwIdBI7echz-LttNstQlcXto_1tcPG78WYFUaEVHMzfaA52EROr7yM3qK1T3gnwyC3JRCj3PMuaUPFfycQN3tC62ZhjlcCLUj6iSa1DteojjQaKoM9w03zlQoNMCFr6qG7w-tYurqA--11wGgcYd0y2mS9Z_x03MWFYEqFNruHVKYo8-b5DEg5VVCSybWD7YBRppYMyG2IGIaS_JFSAIcQf6WDjgZT5fj0fc8If5BJiFQ8jIZ3tlqefXNiw97bDghYBEjMWTebpA532jGgdCZ4K7-oLPg8yAshK2bg9l-8JuhWVOuh798iJJAA; HSID=Amj9DUkPoaHeMdde-; SSID=A4hz7yKW-P1y0UZpZ; APISID=uwb80Sy3NdLSREjp/As0Ej4bxXNhvwE05y; SAPISID=tprYCIX89c6_Yjy7/AaoMaDoV1S0TObkzv; PREF=ID=1111111111111111:FF=0:LD=en:TM=1439444549:LM=1439614118:GM=1:V=1:S=6g8UKduAVIDseHDQ; NID=70=edyjQqfX0CUjR7cKngHeoyrL0m8-RrDPkYR9eUiwXa5GXzd3GlWxWF0M-16YhCKA26FyOz5xIpwr8sMHETUfb6596IMCe_BeWXzWtC0VIHt2dMeAkqnbPd_FAy78h_qR31ZDD6nDEJCAINW-dZELln9hcxToSI7rU0c2Zo-uAGg4aQ4m9aeGzF_8LZ7SOwbzGwGF7SagVpmmJrs; S=izeitgeist-ad-metrics=1DvC7D_y31A' -H 'x-client-data: CJa2yQEIprbJAQiptskBCMS2yQEI8YjKAQ==' --compressed > ";
	string commandPartThree = "";
	for(int a=0; a<len; a++){
		commandPartThree += arr[a];
	}
	commandPartThree += ".csv";
	string finalCommand = commandPartOne + commandPartTwo + commandPartThree;
	system(finalCommand.c_str());
	int sums[len];
	int numLines = 0;
	string ret;
	for(int i=0; i<len; i++){
		sums[i] = 0;
	}
	ifstream in(commandPartThree);
	string parse;
	time_t currentTime;
	time(&currentTime);
	string year = ctime(&currentTime);
	bool isJan = (year.substr(4, 3) == "Jan");
	year = year.substr(year.length()-5, 4);
	if(isJan){
		int oneLessYear = stoi(year);
		oneLessYear--;
		year = to_string(oneLessYear); //if January, make sure we have enough results
	}
	while(getline(in, parse)){
		numLines++;
		int whichTerm = 0;
		if((parse.length() > 3 && parse.substr(0, 4) == year) || (parse.substr(0, 4) == "Week")){
			bool checkIfWeekRow = parse.substr(0, 4) == "Week";
			char* parseDuplicate = strdup(parse.c_str());
			char* tokens = strtok(parseDuplicate, ",");
			while(tokens != NULL){
				//cout << "    " << tokens;
				tokens = strtok(NULL, ",");
				if(tokens != NULL && checkIfWeekRow){
					string temp(tokens);
					arr[whichTerm] = temp;
					whichTerm++;
				}
				else if(tokens != NULL && strncmp(tokens, " ", 1) != 0){
					int toAdd = stoi(tokens);
					sums[whichTerm] += toAdd; //cout << toAdd << "      " << sums[whichTerm] << endl;
					whichTerm++;
				}
			}
		}
	}
	int res = 0;
	for(int i=0; i<len; i++){
		cout << sums[i] << endl;
		if(sums[i] > res){
			ret = arr[i];
			res = sums[i];
		}
	}
	string closingString = "rm " + commandPartThree;
	if(!commandPartThree.empty()) {system(closingString.c_str());}
	return ret;
}
