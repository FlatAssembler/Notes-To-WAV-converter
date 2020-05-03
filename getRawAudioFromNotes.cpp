#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>

std::map<std::string,float> notes;
int sampleRate=44100;

int main(int argc,char **argv) {
	notes["a"]=220;
	notes["as"]=233;
	notes["b"]=247;
	notes["C"]=262;
	notes["Cs"]=277;
	notes["D"]=293;
	notes["Ds"]=311;
	notes["E"]=329;
	notes["F"]=349;
	notes["Fs"]=370;
	notes["G"]=391;
	notes["Gs"]=415;
	notes["A"]=440;
	notes["As"]=466;
	notes["H"]=493;
	notes["C5"]=523;
	notes["Cs5"]=554;
	notes["D5"]=587;
	notes["Ds5"]=622;
	notes["E5"]=659;
	if (argc<2) {
		std::cerr<<"Please supply the text file with notes as an argument." <<std::endl;
		return 1;
	}
	std::ifstream input(argv[1]);
	if (argc>2)
		sampleRate=atoi(argv[2]);
	if (!input)
	{
		std::cerr<<"Can't open \"" <<argv[1] <<"\" for reading!" <<std::endl;
		return 1;
	}
	FILE *raw=std::fopen("output.raw","wb");
	while (!input.eof()) {
		std::string currentNote;
		input >>currentNote;
		if (currentNote.length()==0) break;
		std::string durationString="";
		int i=0;
		while ((currentNote[i]>='0' && currentNote[i]<='9') || currentNote[i]=='.')
		{
			durationString+=currentNote.substr(i,1);
			i++;
		}
		std::cerr <<"Read note name \"" <<currentNote <<"\", the duration string is: " <<durationString <<std::endl;
		int noteDuration=3*sampleRate/std::stof(durationString);
		std::string fullNoteName=currentNote.substr(i);
		std::cerr <<"Playing note \"" <<fullNoteName <<"\" for " <<noteDuration <<" samples." <<std::endl;
		for (int i=0; i<noteDuration; i++) {
			float currentFrequency=notes[fullNoteName];
			float baseFrequency=sin(2*M_PI*currentFrequency*i/sampleRate)*16384;
			float secondHarmony=sin(2*M_PI*2*currentFrequency*i/sampleRate+M_PI/4)*4096;
			float thirdHarmony=sin(2*M_PI*3*currentFrequency*i/sampleRate+M_PI/2)*1024;
			float fourthHarmony=sin(2*M_PI*4*currentFrequency*i/sampleRate+M_PI/2)*512;
			float currentAmplitude=baseFrequency+secondHarmony+thirdHarmony*exp(-(float)(i%sampleRate+sampleRate/4)/(sampleRate/4)); 
			int16_t numberToBeWritten=(fullNoteName=="P")?(0):(currentAmplitude);
			numberToBeWritten+=rand()%256-128;
			fwrite(&numberToBeWritten,2,1,raw);
		}
	}
	fclose(raw);
}
