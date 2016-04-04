/// version 1.0 - 20 march 2016
/// by thomas laureyssens t@toyfoo.com

#pragma once
#include "ofMain.h"
#include "ofxPlaylist.h"
#include "Pixel.h"
#include "ofxGui.h"
#include "ofxSvg.h"
#include <deque>
#include "ofxTime.h"

//WHISTLEDETECTOR
//#include "ofxIntegratedWhistleDetector.h"
#include <ofxIntegratedWhistleDetector.h>
#include <ofxWhistleSequenceDetector.h>

#define pixelAmount 50 // the amount of LED's on a sparrow

//--For WS281x RGB LEDS--
#ifdef __arm__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
extern "C" {
	//#include "board_info.h"
	#include "rpihw.h"
	#include "dma.h"
	#include "clk.h"
	#include "gpio.h"
	#include "pwm.h"
	#include "ws2811.h"
}
#endif

#define TARGET_FREQ                              WS2811_TARGET_FREQ
#define GPIO_PIN                                 18
#define DMA                                      5


class ofApp : public ofBaseApp{
	
	//debugging
	bool load_ws281xlib;
	string serverState;

    //WHISTLEDETECTOR
    ofxIntegratedWhistleDetector detector;
    ofxWhistleSequenceDetector sequenceDetector;  // state machine
    deque<pair<ofxWhistleSequenceDetector::Transition, ofxIntegratedWhistleDetector::Whistle> > transitions;
	static const size_t MaxTransitions = 20; // Count of last transitions and transition queue (with whistles those cause transitions) to be printed on screen

	//LED-related
    int eyePixel;
    int beakPixel;
    int maintenancePixel;
    vector<Pixel*> pixels;
    ofColor maintenanceNoError;
    ofColor maintenanceError;
    ofColor eyeWhistleDetected;
    ofColor eyeWhistleUncertain;
    ofColor eyeListening;
    ofColor beakConnectionError;
    ofColor beakPointSent;
    ofColor beakPointEnabled;
    ofColor profileColor;
    ofColor status200; // success
    ofColor status400; // connection error - orange
    ofColor status400severe; // connection error - about to set whistles to zero
    ofColor status400off; // connection error - no longer records whistles
    ofColor status400webserver;
    ofColor status461; // no active campaign

	// XML & communication
    ofXml settingsXML;
    ofXml logXML;
    string xmlStructure;
    string xmlMessage;
    int whistlesToSend; // the whistles to be sent (introducted for in case connection is down)
    bool freshMessage;
    bool isLoading; //is connecting to server
    bool hasConnectionError; // the most general error, includes connection and webserver error
    bool hasSevereConnectionError; // sustained internet connectivity issues
    bool webserverDown; // only the sparrows webserver down
    string objectID; //hash, object identifier to send to server


	//animation related
    vector<int> randomPixelOrder;
    int animateRandomPixelUntil;
    ofxPlaylist controlPlaylist;
    ofxPlaylist animPlaylist;
    ofxPlaylist connectionPlaylist;
    float fadeInTime;
    float fadeVariationTime;
    int discoLevel; // NEW* used to change the speed of the disco animation
    int discoChance;
    int discoTried;

    
    int whistleState;
    float lastWhistleFrequency; // used to log the frequency, once the timestamp is returned from the server.
    bool limitHours;
    bool noActiveCampaign;
    bool timeIsOff;
    int serverTimeOffHack;
    int onHour;
    int offHour;
    int gameType;
    int msecsPerWhistleDuration;

    float timeoutTimeStates;
    float timeoutTimeWhistles;
    float fadeOutTime;

    unsigned long long previousDebugWhistleTime;
    unsigned long long previousWhistleTime;
    unsigned long long connectionDownTime; // to keep track of when webserver goes down
    bool whistleIsCertain;

    float lastWhistleTime; // in seconds, ofGetElapsedTimef()
    ofTrueTypeFont font;


	// GUI (unused for now)
    ofxColorSlider colorEye;
    ofxColorSlider colorBeak;
    ofxColorSlider colorBody;
    ofxToggle editColor;
    ofxToggle listen;
    ofxButton whistleButton;
    ofxLabel screenSize;
    ofxPanel gui;
    ofxPanel guiColor;
//     ofxIntSlider HzPerWindow;
//     ofxIntSlider powerThresholdPercent;
//     ofxIntSlider msecsPerWhistleDuration;
//     ofxIntSlider hzPerFrequencyDeviation;
    ofxLabel whistleLabel;
    ofxToggle debugToggle;
    ofxFloatSlider debugWhistleCertainty;
    ofxIntSlider debugWhistleFrequency;
    ofxToggle chromeToggle;


public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);

    void whistleButtonPressed();
    void onWhistle( double frequency, float certainty );
    void sendPixels(); // to arduino
    void onKeyframe(ofxPlaylistEventArgs& args);
    ofColor shiftColor(ofColor inputColor);
    ofColor shiftHue(ofColor inputColor);
    unsigned char copyArray (unsigned char array[], int from, int to);
    void urlResponse(ofHttpResponse & response);
    void sendMessageToServer(string type);
    void showWhistleDetectorValues();
    void loadXMLSettings();
    void drawSparrowGraphic();
    void loadSparrowGraphic();
    void parseServerResponse(ofXml serverResponse);

    void newDiscoColors(); //helper for animating
    void seedRandomPixelOrder();
    void animateFillDisco(int level);
    void animateFillSimpleColors();
    void animateFillShiftPushColors();
    void animateFillShiftHueColors();
    void animateFillPercentage(ofColor fillColor, float percentage);
    void animateBeakAndEyeAmbientAnim();
    bool interactionAllowed();
    void addLogItem(string datetime, string animation, string serverresponse, float frequency);

    ws2811_t ledstring;
    int ledBrightness;
    int ledCount;

    //WHISTLEDETECTOR
    void setupDetector();
	void setupSequenceDetector();
	string getDetectorParamsAsString() const;
	string getSequenceDetectorParamsAsString() const;
	string getTransitionsAsString() const;


};
