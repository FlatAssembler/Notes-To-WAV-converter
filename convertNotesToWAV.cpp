#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <map>

std::map<std::string, float> notes;
int sampleRate = 8192; //I don't think it's at all my fault that some players claim to support WAV format, but fail to play a WAV file that has lower sample rate than "standard".

int main(int argc, char **argv) {
	notes["a"] = 220;
	notes["as"] = 233;
	notes["b"] = 247;
	notes["C"] = 262;
	notes["Cs"] = 277;
	notes["D"] = 293;
	notes["Ds"] = 311;
	notes["E"] = 329;
	notes["F"] = 349;
	notes["Fs"] = 370;
	notes["G"] = 391;
	notes["Gs"] = 415;
	notes["A"] = 440;
	notes["As"] = 466;
	notes["H"] = 493;
	notes["C5"] = 523;
	notes["Cs5"] = 554;
	notes["D5"] = 587;
	notes["Ds5"] = 622;
	notes["E5"] = 659;
	if (argc < 2) {
		std::cerr << "Please supply the text file with notes as an argument."
				<< std::endl;
		return 1;
	}
	std::ifstream input(argv[1]);
	if (argc > 2)
		sampleRate = atoi(argv[2]);
	if (!input) {
		std::cerr << "Can't open \"" << argv[1] << "\" for reading!"
				<< std::endl;
		return 1;
	}
	FILE *wav = std::fopen("output.wav", "wb");
	if (!wav) {
		std::cerr << "Can't open \"output.wav\" for output!" << std::endl;
		return 1;
	}
	bool isLittleEndian;
	int testNumber = 0x10;
	std::fwrite(&testNumber, sizeof(int), 1, wav);
	std::fclose(wav);
	wav = std::fopen("output.wav", "rb");
	char testCharacter = 0;
	std::fread(&testCharacter, 1, 1, wav);
	std::fclose(wav);
	if (testCharacter == 0x10) //The logic is: if the C library uses big endian for writing binary files, now "testCharacter" will still contain 0.
		isLittleEndian = true;
	else
		isLittleEndian = false;
	wav = std::fopen("output.wav", "wb");
	if (isLittleEndian)
		std::fprintf(wav, "RIFF"); //ASCII for 0x52494646, the magic number that WAV files start with. 
	else
		std::fprintf(wav, "RIFX"); //Big endian WAV file starts with magic number 0x52494658, or, in ASCII, "RIFX". 
	int32_t ChunkSize = 36 + 8 * sampleRate * 2;
	std::fwrite(&ChunkSize, 4, 1, wav);
	std::fprintf(wav, "WAVEfmt "); //The beginning of the header.
	int32_t Subchunk1Size = 16; //PCM header is always 16 bytes.
	std::fwrite(&Subchunk1Size, 4, 1, wav);
	int16_t AudioFormat = 1; //PCM format.
	std::fwrite(&AudioFormat, 2, 1, wav);
	int16_t NumChannels = 1; //MONO audio.
	std::fwrite(&NumChannels, 2, 1, wav);
	int32_t SampleRate = sampleRate;
	std::fwrite(&SampleRate, 4, 1, wav);
	int32_t ByteRate = 2 * sampleRate; //Since we are using 16 bits per sample, and "sampleRate" samples per second.
	std::fwrite(&ByteRate, 4, 1, wav);
	int16_t BlockAlign = 2; //Each block is two bytes.
	std::fwrite(&BlockAlign, 2, 1, wav);
	int16_t BitsPerSample = 16;
	std::fwrite(&BitsPerSample, 2, 1, wav);
	std::fprintf(wav, "data");
	while (!input.eof()) {
		std::string currentNote;
		input >> currentNote;
		if (currentNote.substr(0, 1) == "#") //Comment
				{
			while (!input.eof() && input.get() != '\n')
				continue;
			continue;
		}
		if (currentNote.length() == 0)
			break;
		std::string durationString = "";
		int i = 0;
		while ((currentNote[i] >= '0' && currentNote[i] <= '9')
				|| currentNote[i] == '.') {
			durationString += currentNote.substr(i, 1);
			i++;
		}
		std::cerr << "Read note name \"" << currentNote
				<< "\", the duration string is: " << durationString
				<< std::endl;
		int noteDuration = 3 * sampleRate / std::stof(durationString);
		std::string fullNoteName = currentNote.substr(i);
		if (std::stof(durationString) == 0
				|| std::isnan(std::stof(durationString))
				|| (notes[fullNoteName] == 0 && fullNoteName != "P")) {
			std::cerr << "Can't interpret the note name \"" << currentNote
					<< "\" or the duration number " << durationString
					<< ", aborting!" << std::endl;
			std::fclose(wav);
			input.close();
			return 1;
		}
		std::cerr << "Playing note \"" << fullNoteName << "\" for "
				<< noteDuration << " samples." << std::endl;
		for (int i = 0; i < noteDuration; i++) {
			float currentFrequency = notes[fullNoteName];
			float baseFrequency = sin(
					2 * M_PI * currentFrequency * i / sampleRate)
					* std::pow(2, 14);
			float secondHarmony = sin(
					2 * M_PI * 2 * currentFrequency * i / sampleRate + M_PI / 4)
					* std::pow(2, 12);
			float thirdHarmony = sin(
					2 * M_PI * 3 * currentFrequency * i / sampleRate + M_PI / 2)
					* std::pow(2, 10);
			float fourthHarmony = sin(
					2 * M_PI * 4 * currentFrequency * i / sampleRate - M_PI / 4)
					* std::pow(2, 9);
			float currentAmplitude = (baseFrequency + secondHarmony
					+ thirdHarmony + fourthHarmony)
					* std::exp(-(float) (2 * i) / (sampleRate)); //Attenuation. 
			int16_t numberToBeWritten =
					(fullNoteName == "P") ? (0) : (currentAmplitude);
			numberToBeWritten += std::rand() % (1 << 8) - (1 << 7); //A bit of noise makes it sound better.
			std::fwrite(&numberToBeWritten, 2, 1, wav);
		}
	}
	input.close();
	std::fclose(wav);
}
