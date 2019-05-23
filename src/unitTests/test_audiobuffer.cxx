#include "catch.hpp"
#include "test_debug.hxx"
#include "../audiobuffer.hxx"

extern OUTPUT output;

int AudioBuffer::privateID = 0;

TEST_CASE("AudioBuffer Tests")
{
	AudioBuffer *ab_sizeless = new AudioBuffer();
	AudioBuffer *ab_size = new AudioBuffer(1000);

	SECTION("Test correct initial state")
	{
		REQUIRE(ab_size->getBeats() == 0);
		REQUIRE(ab_size->getAudioFrames() == 0);
		REQUIRE(ab_size->getSize() == 1000);

		REQUIRE(ab_sizeless->getBeats() == 0);
		REQUIRE(ab_sizeless->getAudioFrames() == 0);
		REQUIRE(ab_sizeless->getSize() == 0);
	}

	SECTION("Test ID changes")
	{
		ab_size->setID(500);
		REQUIRE(ab_size->getID() == 500);
		ab_sizeless->setID(501);
		REQUIRE(ab_sizeless->getID() == 501);
	}

	SECTION("Test name changes") 
	{
		ab_size->setName("abcdef");
		REQUIRE(ab_size->getName() == "abcdef");
		ab_size->setName("Lorem ipsum dolor sit amet");
		REQUIRE(ab_size->getName() == "Lorem ipsum dolor si");
	}

	SECTION("Test beat num changes") {
		ab_size->setBeats(5);
		REQUIRE(ab_size->getBeats() == 5);
	}

	SECTION("Test audio frames change") {
		ab_size->setAudioFrames(900);
		REQUIRE(ab_size->getAudioFrames() == 900);

		// Set AudioFrames higher than size
		REQUIRE(ab_size->setAudioFrames(1500) == false);
		REQUIRE(ab_size->getAudioFrames() == 900);
	}

	SECTION("Test Set Audio Buffer with different Length") {
		std::vector<float> tempL;
		for(int i = 0; i < 500; i++) {
			tempL.push_back(rand());
		}
		std::vector<float> tempR;
		for(int i = 0; i < 900; i++) {
			tempR.push_back(rand());
		}

		std::vector<float> argL = tempL;
		std::vector<float> argR = tempR;
		ab_size->nonRtSetSample(argL, argR);

		REQUIRE(ab_size->getSize() == 500);
		REQUIRE(strcmp(output.format,
			"left and right channels of audio buffer have different size: %i vs %i") == 0);
		REQUIRE(output.warnLevel == 1);

		REQUIRE(ab_size->getDataL() == tempL);
		REQUIRE(ab_size->getDataR() == tempR);

		output.reset();
	}
}