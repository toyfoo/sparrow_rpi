/// sparrows
/// version 1.1 - 7 july 2016
/// by thomas laureyssens t@toyfoo.com


#include "ofApp.h"

void ofApp::setup()
{
    // for debugging
    //ofSetLogLevel(OF_LOG_VERBOSE);

    //sparrow default settings
    ledCount = 50;
    ledBrightness = 255;
    eyePixel = 48;
    beakPixel = 49;
    maintenancePixel = 0;
    onHour = 7;
    offHour = 22;
	limitHours = false;

    //gamesettings
    gameType = 1;
    timeoutTimeStates = 9000.f;
    timeoutTimeWhistles = 20000.f;
    msecsPerWhistleDuration = 150;
    serverTimeOffHack = 0; // the hours the server is off
    discoChance = 3;  // percentage chance there is discolight (after a minimum of 15 whistles)
    serverConnectionRetries = 0;

    // animation settings
    fadeOutTime = 1000.f;
    fadeInTime = 500.f;
    fadeVariationTime = 30.f;

    maintenanceNoError = ofColor::green;
    maintenanceError = ofColor::red;
    eyeWhistleDetected = ofColor::green;
    eyeWhistleUncertain = ofColor::orange;
    eyeListening = ofColor::grey;
    beakConnectionError = ofColor::red;
    beakPointSent = ofColor::green;
    beakPointEnabled = ofColor::brown;

    status200 = ofColor::green;; // success
    status461 = ofColor::blueViolet;; // no active campaign
    status400 = ofColor::darkorange; // error
    status400severe = ofColor::red; // connection error - about to set whistles to zero
    status400webserver = ofColor::magenta; // webserver down

    // ofApp settings
    ofSetVerticalSync(true);
    ofSetFrameRate(25);
    ofSetWindowTitle("Sparrows");

    //zeroing variables
    webserverDown = false;
    lastWhistleTime = 0;
    whistlesToSend = 0; //todo: save this to an xml file in case the app restarts
    isLoading = false;
    noActiveCampaign = false;
    freshMessage = false;
    whistleState = 0;
    whistleIsCertain = false;
    previousWhistleTime = 0;

    hasConnectionError = false;
    hasSevereConnectionError = false;
    connectionDownTime = 0;


    timeIsOff = false; //by default, the time can be off so we're sure the whistle function runs until the time is synchronised with the time servers by the system
    lastWhistleFrequency = 0;
    discoLevel = 0;
    discoTried = 0;

    // event registration
    ofxKeyframeAnimRegisterEvents(this);
    ofRegisterURLNotification(this);

    //*
    // GUI
    whistleButton.addListener(this,&ofApp::whistleButtonPressed);

    // setting gui size 300, 18
    //ofxGuiSetDefaultWidth(250);
    //ofxGuiSetDefaultHeight(18);

    //gui.setup("Sparrow Configuration", "settings.xml", 170, 3);
    //gui.add(HzPerWindow.setup("HzPerWindow", 100, 10, 500)); // a window width in frequency domain within signal energy is integrated; then ratio of windowed energy to entire signal energy is used as a spike value; the window floats across frequency axis. A good whistle has very narrow window ~10 Hz. Whistle on spectrogram looks as line. The line thickness shows frequency band the whistle occupies. Good whistle has very narrow band ~10 Hz , bad - 100s of Hz. In ideal, Spike Window must exactly cover a whistle, but we don't know the whistle in advance. Spike Window lower values - more matching for good whistles, higher values - decreasing certainty of good whistles but allowing bad whistles. Spike Window = 100 Hz is reasonable value from my experimentation.

    //gui.add(powerThresholdPercent.setup("powerThresholdPercent", 80, 1, 100)); // minimal value of spike energy to be considered;
    // Look the output: Whistle detected: frequency 1754.96Hz, certainty (Ewin/Eblock) 0.858128
    // In this case Ewin/Eblock = 0.858128 and this is an averaged ratio of signal energy within Spike Window to energy of entire signal block (FFT frame). Another words, this ratio shows how much energy is contained in the whistle. In noisy environment this value would be lower, in clean - higher. The threshold discards signal with such ratio below.

    //gui.add(msecsPerWhistleDuration.setup("msecsPerWhistleDuration", 500, 50, 1000)); // minimal whistle duration (msecs.) to be considered
    // If we detect a whistle (it may be false whistle) that has less duration, this whistle is discarded.  At a small value ~10s msecs there a lot of false positives from ordinal speech. Values ~100s msecs stop such alarms. A simple way to increase reliability is to set relatively big value, 250 msecs or 500 msecs or even 1000 msecs.

    //gui.add(hzPerFrequencyDeviation.setup("hzPerFrequencyDeviation", 100, 20, 500)); // maximal allowed frequency shift between two consequitive (in time) spikes. Between two FFT frames whistle frequency may change (e.g. increasing or decreasing tone whistling - looks on spectrogram as a line with some angle to horizontal axis). The parameters cuts off too steep lines (discards respective whistles).

    //gui.add(debugToggle.setup("Debug", true));
    //gui.add(debugWhistleCertainty.setup("debugWhistleCertainty", 0.0, 0, 1));
    //gui.add(debugWhistleFrequency.setup("debugWhistleFrequency", 10000, 8000, 20000));
    //gui.add(whistleButton.setup("whistle"));




    font.loadFont("DIN.otf", 10);


    // drawing pixels on screen for debug purposes
    int xposstart = 3;
    int xpos = xposstart;
    int ypos = 3;
    for(int i = 0; i < pixelAmount; i++){
        pixels.push_back(new Pixel());
        pixels[i] -> ledpos = i; // adding the position in the LED strip
        pixels[i] -> set(xpos,ypos,12,12);
        xpos += 15;
        if(xpos >= xposstart + 150) {
            xpos = xposstart;
            ypos += 15;
        }
        pixels[i] -> setup();
    }
    pixels[beakPixel]->partType = "beak";
    pixels[eyePixel]->partType = "eye";
    pixels[maintenancePixel]->partType = "maintenance";


    // load xml settings
    if (settingsXML.load("sparrowsSettings.xml") ){
        ofLog(OF_LOG_NOTICE, "sparrowsSettings.xml loaded");
	} else {
        settingsXML.addChild("Sparrow");
        settingsXML.setTo("Sparrow");
        ofLog(OF_LOG_NOTICE, "unable to load sparrowsSettings.xml check /bin/data/ folder");
	}

    if(!settingsXML.exists("//objectId")) {
        settingsXML.addChild("objectId");
    }
    if (settingsXML.getValue("//objectId") == "") {
        settingsXML.setValue("//objectId", "VISIT_WWW.WHISTLE.CITY_TO_GENERATE_OBJECT_ID");
    } else {
        objectID = settingsXML.getValue("//objectId");
    }

    if(!settingsXML.exists("//onHour")) {
        settingsXML.addChild("onHour");
    }
    if (settingsXML.getValue("//onHour") == "") {
        settingsXML.setValue("//onHour", ofToString(onHour));
    } else {onHour = settingsXML.getValue<int>("//onHour");}

    if(!settingsXML.exists("//offHour")) {
        settingsXML.addChild("offHour");
    }
    if (settingsXML.getValue("//offHour") == "") {
        settingsXML.setValue("//offHour", ofToString(offHour));
    } else {offHour = settingsXML.getValue<int>("//offHour");}

    if(!settingsXML.exists("//limitHours")) {
        settingsXML.addChild("limitHours");
    }
    if (settingsXML.getValue("//limitHours") == "") {
        settingsXML.setValue("//limitHours", ofToString(limitHours));
    } else {limitHours = settingsXML.getValue<bool>("//limitHours");}

    if(!settingsXML.exists("//eyeLED")) {settingsXML.addChild("eyeLED");}
    if (settingsXML.getValue("//eyeLED") == "") {settingsXML.setValue("//eyeLED", ofToString(eyePixel));
    } else {eyePixel = settingsXML.getValue<int>("//eyeLED");}

    if(!settingsXML.exists("//beakLED")) {settingsXML.addChild("beakLED");}
    if (settingsXML.getValue("//beakLED") == "") {settingsXML.setValue("//beakLED", ofToString(beakPixel));
    } else {beakPixel = settingsXML.getValue<int>("//beakLED");}

    if(!settingsXML.exists("//maintenanceLED")) {settingsXML.addChild("maintenanceLED");}
    if (settingsXML.getValue("//maintenanceLED") == "") {settingsXML.setValue("//maintenanceLED", ofToString(maintenancePixel));
    } else {maintenancePixel = settingsXML.getValue<int>("//maintenanceLED");}

    if(!settingsXML.exists("//minimumWhistleDuration")) {settingsXML.addChild("minimumWhistleDuration");}
    if (settingsXML.getValue("//minimumWhistleDuration") == "") {settingsXML.setValue("//minimumWhistleDuration", ofToString(msecsPerWhistleDuration));
    } else {msecsPerWhistleDuration = settingsXML.getValue<int>("//minimumWhistleDuration");}

    if(!settingsXML.exists("//profileColor")) {settingsXML.addChild("profileColor");}
    if (settingsXML.getValue("//profileColor") == "") {settingsXML.setValue("//profileColor", "#FF8000");
    } else {
        string bodycolorstring = settingsXML.getValue("//profileColor");
        bodycolorstring = bodycolorstring.replace(0, 1, "0x");
        profileColor = ofColor::fromHex(ofHexToInt(bodycolorstring));
    }

    if(!settingsXML.exists("//gameType")) {
        settingsXML.addChild("gameType");
    }
    if (settingsXML.getValue("//gameType") == "") {
        settingsXML.setValue("//gameType", ofToString(gameType));
    } else {gameType = settingsXML.getValue<int>("//gameType");}

    settingsXML.save("sparrowsSettings.xml");



    // load xml settings
    if (logXML.load("log.xml") ){
        ofLog(OF_LOG_NOTICE, "log.xml loaded");
	} else {
        logXML.addChild("sparrowslog");
        logXML.setTo("sparrowslog");
        ofLog(OF_LOG_NOTICE, "unable to load log.xml check /bin/data/ folder");
	}

    if(!logXML.exists("//objectId")) { //the double forward slash //RED means: search from the root of the xml for a tag could RED
        logXML.addChild("objectId");
    }
    logXML.setValue("//objectId", objectID);

    if(!logXML.exists("//unsent")) { //the double forward slash //RED means: search from the root of the xml for a tag could RED
        logXML.addChild("unsent");
    }
    if (logXML.getValue("//unsent") == "") {
        logXML.setValue("//unsent", ofToString(whistlesToSend));
    } else {
        whistlesToSend = logXML.getValue<int>("//unsent");
    }

    if(!logXML.exists("//log")) {
        logXML.addChild("log");
    }
    logXML.save("log.xml");

    //ofLogToFile("ofLogFile.txt", true);

    seedRandomPixelOrder(); //first seed of the randompixelorder

    // setup ws281x
    #if ws281xLoad
    const rpi_hw_t *piVersion = rpi_hw_detect();
    ledstring = {
        0,
        piVersion,
        TARGET_FREQ,
        DMA,
        {
            {GPIO_PIN, 0, ledCount, ledBrightness },
        }
    };

     if (ws2811_init(&ledstring)){
     ofLogError("ws2811_init() failed");
      }else{
       ofLogNotice("ws2811_init() success");
    }
    #endif

    // setup WHISTLEDETECTOR
	#if soundLoad
    setupDetector();
    setupSequenceDetector();
    #endif

	// initiate eye/beak animation
    animateBeakAndEyeAmbientAnim();


    // starting the code using .bashprofile loads the code faster than the system can join a wifi network.
    // this might block this application when starting up and no network connection is available
    // so let's add a pause
    
    using namespace Playlist;

    if (whistlesToSend > 0) {
        connectionPlaylist.addKeyFrame(Action::pause(1000.f));
        connectionPlaylist.addKeyFrame(Action::event(this, "sendWhistle_delayed"));
    }
    
    connectionPlaylist.addKeyFrame(Action::pause(5000.f));
    connectionPlaylist.addKeyFrame(Action::event(this, "keepAlive"));
}

void ofApp::update()
{
    controlPlaylist.update();
    animPlaylist.update();
    connectionPlaylist.update();


	//#if loadSound
    // WHISTLE DETECTOR CODE

    //*
    // Get next whistle, it may be null whistle (Whistle::isNull()) if no whistle is detected
	const ofxIntegratedWhistleDetector::Whistle whistle = detector.getNextWhistle();

	// Regardless of we got a whistle or not, we need to update whistle sequence detector
	const ofxWhistleSequenceDetector::Transition transition = sequenceDetector.update(!whistle.isNull());

	// Check if we below the final state, i.e. below 100% certainty (this is mode 1 - collecting
	// 100% certainty)
	if (!transition.isToFinal()) { // == transition.certaintyPercent() < 100
		// There is no actual transition. In this case we have two or more whistles those look like
		// one already counted whistle, i.e. pause between the whistles less then
		// ofxWhistleSequenceDetector::msecsPerStateGap() msecs.
		// Nothing needs to be done in this case
		if (!transition.hasTransition())
			;
		// Else we have real transition from one state to another. Here it is not possible to meet
		// final 100% certainty state, because we check this condition on upper level
		else {
			// Initial state. We go to initial state after the first line timeout
			// ofxWhistleSequenceDetector::msecsPerStateTimeout() or the second line timeout
			// ofxWhistleSequenceDetector::msecsPerFinalStateTimeout()
			if (transition.isToInitial()) {
                // TurnOffAllLEDs();
				// ...

				ofLog() << "back to initial " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");

                if (whistleState == 5) { // extra reset for disco mode

                    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
                        int cPixel = randomPixelOrder[i];

                        if (pixels[cPixel] -> partType == "body") {
                            pixels[cPixel] -> clearCuelist();
                            pixels[cPixel] -> addFade(ofColor::black, 0.f, true);
                            pixels[cPixel] -> play();
                        }
                    }
                }


                whistleState = 0;
                discoTried = 0;
                discoTried = 0;

				seedRandomPixelOrder();

                animateBeakAndEyeAmbientAnim();


				// If we go to initial state from final 100% certainty state, i.e. from
				// mode 2 - playing, do something
				if (transition.isFromFinal()) {

                    //ofLog() << "transition.isFromFinal() - saving the logfile";
                    //logXML.save("log.xml"); // we save this here not to overload the system when the file gets too big
                    //ofLog() << "logfile saved";

				}
				// Here we go to initial state from non-final state, i.e. we do not reach 100%
				// certainty and begin from zero due to timeout
				else {
					//int x = 0; (void)x;
					// May be some processing is needed here
				}
			}
			// Here only can be some non-initial and non-final state
			else {

                animPlaylist.clear();

                // TurnOnSomeLEDs(transition.certaintyPercent() - describes how many LEDs);
                //ofLog() << "whistle: initial state. " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");

                if (interactionAllowed()) {
                    pixels[eyePixel]->addBlink(2, eyeWhistleUncertain);
                    pixels[maintenancePixel]->addBlink(1, eyeWhistleUncertain); // white blink if uncertain -- similar to filling up
                    animateFillPercentage(ofColor::white, transition.certaintyPercent()/100.f);
                }
                else {
                	pixels[maintenancePixel]->addBlink(2, profileColor); // whistling outside the allowed time
                }

            }
        }
    }
	// Here we are in final state, after 100% certainty (this is mode 2 - playing). In this state
	// we will stay exactly ofxWhistleSequenceDetector::msecsPerFinalStateTimeout() msecs. (this is
	// the second line timeout) regardless of any whistles
	else {
		// If we detect a separate whistle, do some LED animation
		if (!whistle.isNull()) {

            if (interactionAllowed()) {
                pixels[eyePixel]->addBlink(2, eyeWhistleDetected);
                if (hasConnectionError) {
                    whistlesToSend++; //don't send it to the server, but save it locally once over a certain treshold (20)
                    if (whistlesToSend - logXML.getValue<int>("//unsent") > 20){
                        logXML.setValue("//unsent", ofToString(whistlesToSend));
                        if (logXML.save("log.xml")){
                            ofLogNotice("Logfile saved since connection is down");
                        } else {ofLogNotice("Error saving logfile (connection is down)");}
                    }
                } else { // in a campaign, and no connection error
                    pixels[maintenancePixel]->addBlink(1, ofColor::white);
                    sendMessageToServer("whistle");
                }

                if (whistleState < 4) {
                    whistleState++;
                }

                if (whistleState == 5) {
                    discoLevel ++;
                }

                if (whistleState == 4) {
                    if ((ofRandom(100) > (100 - discoChance)) && discoTried > 15 ) {
                        discoLevel = 0;
                        discoTried = 0;
                        whistleState = 5;
                    } else {
                        discoTried++;
                    }
                }

                switch (whistleState) {
                    case 1: // animation of whistle states below 100% certainty.
                        animateFillPercentage(ofColor::white, 1.f);
                    case 2: // animation of whistle state 100%: simple colour
                        whistleState++;
                        animateFillSimpleColors();
                        break;

                    case 3: // animation of whistle state 100%: first modification of colors
                        animateFillShiftPushColors();
                        break;

                    case 4: // animation of whistle state 100%: second modification of colors
                        animateFillShiftHueColors();
                        break;

                    case 5: // animation of whistle state 100%: second modification of colors
                        animateFillDisco(discoLevel);
                        break;

                    default:
                        break;
                }

            }
            else {pixels[eyePixel]->addBlink(2, ofColor::red);}
		}
	}
	//#endif

     //*/


}

//--------------------------------------------------------------
void ofApp::draw(){

	#if ws281xLoad
        for (int i = 0; i < ledCount; i++){
            //ledstring.channel[0].leds[i] = createRGB(200,155,255);
            ledstring.channel[0].leds[i] = pixels[i] -> color.getHex();
        }
        ws2811_render(&ledstring);
	#endif
	
    ofBackground(ofColor::lightGray);
	//gui.draw();
	ofSetColor(0);

	std::stringstream ss;
	ss << "Sparrow\tv1.1 " << std::endl;
    ss << "\tWhistles to send to server: " << whistlesToSend << std::endl;
    ss << "\tInteraction state: " << whistleState << std::endl;
    ss << std::endl;
    ss << "Time\t" << ofGetHours() << ":" << ofGetMinutes() << ":" << ofGetSeconds() << std::endl;
    if (interactionAllowed()) {ss << "\tSystem time is within the allowed time." << std::endl;}
    else {ss << "\tSystem time not in the allowed time." << std::endl;}
    if (limitHours) {ss << "\tTime restriction enabled: whistle only within allowed times." << std::endl;}
    else {ss << "\tTime restriction disabled: whistle anytime." << std::endl;}
    ss << std::endl;
    if (connectionAlive) {ss << "Server\tUp. This is good. " << std::endl;}
    else {ss << "Server\tDown. This it not good." << std::endl;}
    ss << "\t" << serverState << std::endl;
	ss << "\t" << serverResponseRaw << std::endl;
	ofDrawBitmapString(ss.str(), ofVec2f(170, 15));
}




//--------------------------------------------------------------
void ofApp::seedRandomPixelOrder() {

    randomPixelOrder.clear(); // creating a new pixelRandomOrder
    animateRandomPixelUntil = 0;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            randomPixelOrder.push_back(i);
        }
    }
    ofRandomize(randomPixelOrder);

}

//--------------------------------------------------------------
void ofApp::animateFillSimpleColors() {

    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel

        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(profileColor, fadeInTime, true); // fadintime was 100.f
        //if (i==0) {pixels[randomPixelOrder[i]] -> addCoutElapsedTime();}
        pixels[cPixel] -> addPause(timeoutTimeWhistles, false);
        //if (i==0) {pixels[randomPixelOrder[i]] -> addCoutElapsedTime();}
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }


}

//--------------------------------------------------------------
void ofApp::animateFillShiftPushColors() {

    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(shiftColor(profileColor), fadeInTime, true); // fadintime was 100.f
        pixels[cPixel] -> addPause(timeoutTimeWhistles, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }

}

//--------------------------------------------------------------
void ofApp::animateFillShiftHueColors() {

    vector<ofColor> tempToColors;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }

    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    int it = 0;

    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, true);
        pixels[cPixel] -> addFade(shiftHue(tempToColors[it]), fadeInTime/2, true);
        pixels[cPixel] -> addPause(timeoutTimeWhistles + fadeVariationTime * i, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
        it++;
    }

}

//--------------------------------------------------------------
void ofApp::animateFillDisco(int mode) {

    float pause;
    float fade = 0.f;
    float speedvar = 0.60;

    if (mode > 12) {
        mode = 12;
    }

    pause = 700.f - 50.f * mode;


    int animationamount = floor(timeoutTimeWhistles*speedvar/(pause+fade));
    //cout << "animationamount " << animationamount << endl;


    vector<ofColor> tempToColors;

    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }


    tempToColors[0] = ofColor::white;


    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> clearCuelist();
    }

    for (int j = 0; j < animationamount; j++) {
        int it = 0;
        for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
            int cPixel = randomPixelOrder[i]; //current pixel
            pixels[cPixel] -> addFade(tempToColors[it], fade, true);
            pixels[cPixel] -> addPause(pause, true);
            it++;
        }
        ofRandomize(tempToColors);
    }

    for (unsigned int i = 0; i < randomPixelOrder.size(); i++) {
        int cPixel = randomPixelOrder[i]; //current pixel
        pixels[cPixel] -> addPause(fadeVariationTime * i, false);
        pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
        pixels[cPixel] -> play();
    }
}




//--------------------------------------------------------------
void ofApp::animateFillPercentage(ofColor fillColor, float percentage) {
    
    int previousAnimateRandomPixelUntil = animateRandomPixelUntil; // in case this animation has ran before, we want to know
    animateRandomPixelUntil = floor(randomPixelOrder.size() * percentage);

    if (previousAnimateRandomPixelUntil <= animateRandomPixelUntil) { // progressing: more whistle certainty

        for (int i = 0; i < animateRandomPixelUntil; i++) {
            int cPixel = randomPixelOrder[i]; //current pixel
            if (i < previousAnimateRandomPixelUntil) { // previous pixels stay white but get a new fade-out value
                pixels[cPixel] -> clearPlaylist();
                pixels[cPixel] -> addPause(timeoutTimeStates + fadeVariationTime * i, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime + fadeVariationTime * i, false);
                pixels[cPixel] -> play();
            } else {
                pixels[cPixel] -> addPause(fadeVariationTime * (i - previousAnimateRandomPixelUntil), true);
                pixels[cPixel] -> addFade(fillColor, fadeInTime, true);
                pixels[cPixel] -> addPause(timeoutTimeStates, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
                pixels[cPixel] -> play();
            }
        }
    } else { // detector is less certain about the whistle - turn pixels to black again

        for (int i = previousAnimateRandomPixelUntil; i >= 0; i--) {
            int cPixel = randomPixelOrder[i]; //current pixel

            if (i >= animateRandomPixelUntil) {
                pixels[cPixel] -> addPause(fadeVariationTime * i, true);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, true);
                pixels[cPixel] -> play();
            } else {  // previous pixels stay white but get a new fade-out value
                pixels[cPixel] -> clearPlaylist();
                pixels[cPixel] -> addPause(timeoutTimeStates + fadeVariationTime * i, false);
                pixels[cPixel] -> addFade(ofColor::black, fadeOutTime, false);
                pixels[cPixel] -> play();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::animateBeakAndEyeAmbientAnim() {

    using namespace Playlist;

        pixels[eyePixel] -> addFade(ofColor::white, 10000.f, false);
        pixels[eyePixel] -> addFade(ofColor::black, 10000.f, false);
        pixels[eyePixel] -> play();


        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(100.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(100.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::white, false);
        pixels[beakPixel] -> addPause(50.f, false);
        pixels[beakPixel] -> addSetColor(ofColor::black, false);
        pixels[beakPixel] -> addPause(19050.f, false);
        pixels[beakPixel] -> play();

        animPlaylist.addKeyFrame(Action::pause(20200.f));
        animPlaylist.addKeyFrame(Action::event(this, "animateBeakAndEyeAmbientAnim"));
}

//--------------------------------------------------------------
bool ofApp::interactionAllowed() {
    if ((ofGetHours() > onHour && ofGetHours() <= offHour) || !limitHours) {
        return true;
   } else {
        return false;
    }
}


//--------------------------------------------------------------
void ofApp::onWhistle( double frequency, float certainty) {
    using namespace Playlist;

    lastWhistleFrequency = frequency;


    if (interactionAllowed()) {

        //        cout << "[notice ] onWhistle certainty: " << certainty << " isWhistleCertain: " << whistleIsCertain << " whistleState: " << whistleState << " time interval: " << whistleInterval << endl;


        if (certainty > 0.9) {

            if (whistleState >= 2 && whistleState < 4) {
                whistleState++;
            }

            // NEW* ----

            if (whistleState == 5) {
                discoLevel ++;
            }

            if (whistleState == 4) {
                if ((ofRandom(100) > (100 - discoChance)) && discoTried > 10 ) {
                    discoLevel = 0;
                    discoTried = 0;
                    whistleState = 5;
                } else {
                    discoTried++;
                }
            }

            if (certainty > 0.9 && certainty < 1 ) { // if it's almost certainly a whistle, that's ok.
                certainty = 1;
            }

            sendMessageToServer("whistle");
            pixels[eyePixel]->addBlink(2, eyeWhistleDetected);

        } else {
            pixels[eyePixel]->addBlink(2, eyeWhistleUncertain);
        }

        // Sparrows main animation
        switch (whistleState) {
            case 0: // creating new random values for the animation to look organic
            {
                seedRandomPixelOrder();
                logXML.save("log.xml");
                ofLogNotice("saving log.xml");
            }
            case 1: // animation of whistle states below 100% certainty.
            {
                //ofLogNotice("whistleState 1");
                whistleState = 1; // in case whistleState was 0, we're advancing a state (case 0 has no break)

                animateFillPercentage(ofColor::white, certainty);

                if (certainty < 0.9) {
                    break;
                }
            }
            case 2: { // animation of whistle state 100%: simple colour

                //ofLogNotice("whistleState 2");
                whistleState = 2;

                animateFillSimpleColors();

                controlPlaylist.update(); // is needed for the following clear() to work, since we already added elements in this same frame (we get into case#2 from case#1 when certainty is > 0.9)
                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles + fadeVariationTime * randomPixelOrder.size()));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;
            }

            case 3: // animation of whistle state 100%: first modification of colors
                //ofLogNotice("whistleState 3");

                // simple shift of color

                animateFillShiftPushColors();

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;

            case 4: // animation of whistle state 100%: second modification of colors

                //ofLogNotice("whistleState 4");

                animateFillShiftHueColors();

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;

            case 5: //

                ofLogNotice("whistleState 5");

                animateFillDisco(discoLevel);

                controlPlaylist.clear();
                controlPlaylist.addKeyFrame(Action::pause(timeoutTimeWhistles));
                controlPlaylist.addKeyFrame(Action::event(this, "whistleState 0"));

                break;
                // more cases will be added

            default:
                break;
        }

    } else {ofLog(OF_LOG_NOTICE, "whistle - but too late or too early");}
}



//--------------------------------------------------------------
void ofApp::onKeyframe(ofxPlaylistEventArgs& args){

    //ofLog(OF_LOG_VERBOSE) << "onKeyframe " << args.message << ": " << ofGetFrameNum();

    using namespace Playlist;

    // this check is only necessary if you want to be absolutely sure that
    // the onKeyFrame Event was sent by the same object as the receiver.
    if (args.pSender != static_cast<void*>(this)) return;


    if (args.message == "keepAlive") {
        if (whistlesToSend > 0) {
            sendMessageToServer("whistle_delayed");
        } else {
            sendMessageToServer("keepAlive");
            connectionPlaylist.clear();
            connectionPlaylist.addKeyFrame(Action::pause(30000.f));
            connectionPlaylist.addKeyFrame(Action::event(this, "keepAlive"));
        }
        
        if (whistlesToSend != logXML.getValue<int>("//unsent")){
            logXML.setValue("//unsent", ofToString(whistlesToSend));
            if (logXML.save("log.xml")){
                ofLogNotice("Logfile saved");
            } else {ofLogNotice("Error saving logfile");}
        }
    }
    
    else if (args.message == "animateBeakAndEyeAmbientAnim") {
        animateBeakAndEyeAmbientAnim();
    }
    
    else if (args.message == "whistleState 0") {
        whistleState = 0;
    }
    
    else if (args.message == "sendWhistle_delayed") {
        if (whistlesToSend > 0) {
            sendMessageToServer("whistle_delayed");
        }
    }
    
    else {
        ofLogNotice("onKeyframe: Unimplemented args.message received.");
    }
}

// communication functions
//--------------------------------------------------------------


//--------------------------------------------------------------
void ofApp::sendMessageToServer(string type){

    //return;
    
    //http://www.whistle.city/activityapi?objectId=6C3aCZoYJGA7yA6oc&activityType=whistle&gameType=1
    //http://www.whistle.city/activityapi?objectId=6C3aCZoYJGA7yA6oc&activityType=ping&gameType=1


    //ofLog() << "send servermessage " + type + " " + ofGetTimestampString("%Y-%n-%e T%H:%M:%S");

    if (type == "whistle") {
        whistlesToSend++; 
        //logXML.setValue("//unsent", ofToString(whistlesToSend));
        if (!isLoading) {
            freshMessage = true;
            //cout << "whistle: http://whistle.city/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType) << endl;
            ofLoadURLAsync("http://www.whistle.city/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType), "whistle");
            serverState = "Message sent " + type;
            isLoading = true;
        }
    }
    
    if (type == "whistle_delayed") {
        if (!isLoading) {
            ofLoadURLAsync("http://www.whistle.city/activityapi?objectId=" + objectID + "&activityType=whistle&gameType=" + ofToString(gameType), "whistle_delayed");
            serverState = "Message sent " + type;
            isLoading = true;
        }
    }
    
    if (type == "keepAlive") {
        if (!isLoading) {
            ofLoadURLAsync("http://www.whistle.city/statusapi?objectId=" + objectID, "keepAlive");
            isLoading = true;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response){

    using namespace Playlist;

	serverResponseRaw = "Raw: " + response.request.name + " – " + ofToString(response.status) + " – " + ofGetTimestampString("%H:%M:%S");
    
    if(response.request.name == "keepAlive"){ // subsequent connection tests
        
            if (response.status == 200 || response.status != 301) {
                serverState = "OK: www.whistle.city is alive";
                isLoading = false;
                connectionAlive = true;
                
                if (whistlesToSend > 0) {
                    connectionPlaylist.clear();
                    connectionPlaylist.addKeyFrame(Action::pause(1000.f));
                    connectionPlaylist.addKeyFrame(Action::event(this, "sendWhistle_delayed"));
                    connectionPlaylist.addKeyFrame(Action::pause(30000.f));
                    connectionPlaylist.addKeyFrame(Action::event(this, "keepAlive"));
                }
                pixels[maintenancePixel] -> clearPlaylist();
                pixels[maintenancePixel] -> setColor(ofColor::black);
            }
        
    }
    else if (response.status == 200) { // HTTP response of all other (whistle & delayed whistle) connections

		serverState = "OK: Response from www.whistle.city";
        isLoading = false;
        connectionAlive = true;
        serverConnectionRetries = 0;

        if (hasConnectionError) {
            hasConnectionError = false;
            hasSevereConnectionError = false; // sustained internet connectivity issues
            webserverDown = false;
            pixels[maintenancePixel] -> clearPlaylist();
            pixels[maintenancePixel] -> setColor(ofColor::black);
        }

        ofXml serverResponseXML;

        serverResponseXML.loadFromBuffer(response.data.getText());

        //ofLog() << "http: " << response.request.name << " " << response.status << " " << response.error << " Server: " << serverResponseXML.getValue("//status") << " time: " << ofGetTimestampString("%H:%M:%S"); //ofGetTimestampString("%Y-%n-%e T%H:%M:%S")

        if(serverResponseXML.exists("//status") && serverResponseXML.getValue("//status") != "") {

            // in case we want individual controls on
			/*
            if (serverResponseXML.exists("//onHour") && serverResponseXML.exists("//offHour") && serverResponseXML.getValue("//onHour") != "" && serverResponseXML.getValue("//offHour") != "") {
                onHour = serverResponseXML.getValue<int>("//onHour");
                offHour = serverResponseXML.getValue<int>("//offHour");
                settingsXML.setValue("//onHour", serverResponseXML.getValue("//onHour"));
                settingsXML.setValue("//offHour", serverResponseXML.getValue("//offHour"));
            }

            if (serverResponseXML.exists("//gameType") && serverResponseXML.getValue("//gameType") != "" && serverResponseXML.getValue("//gameType") != "0") {
                gameType = serverResponseXML.getValue<int>("//gameType");
                settingsXML.setValue("//gameType", serverResponseXML.getValue("//gameType"));
            }
            */
            
            if (serverResponseXML.exists("//profileColor") && serverResponseXML.getValue("//profileColor") != "") {
            	if (settingsXML.getValue("//profileColor") != serverResponseXML.getValue("//profileColor")) {
                	settingsXML.setValue("//profileColor", serverResponseXML.getValue("//profileColor"));
                	string bodycolorstring = serverResponseXML.getValue("//profileColor");
                	bodycolorstring = bodycolorstring.replace(0, 1, "0x");
                	profileColor = ofColor::fromHex(ofHexToInt(bodycolorstring));
                    settingsXML.save("sparrowsSettings.xml");
                }
            }

            // status OK
            if (serverResponseXML.getValue("//status") == "200" || serverResponseXML.getValue("//status") == "461" /*no active campaign*/) { //whistle was correctly received
                serverState = "OK: Whistle.city online";
                
                noActiveCampaign = false;

                if(response.request.name == "whistle" || response.request.name == "whistle_delayed"){
                    serverState = "OK: whistle.city received whistle";

                    if (freshMessage) { //no visual feedback for delayed whistles
                        freshMessage = false;
                        pixels[beakPixel] -> addBlink(2, status200);
                        pixels[maintenancePixel] -> addBlink(1, status200);
                    }
                    
                    if (whistlesToSend == 1) {
                        serverState = "OK: Finished sending whistles";
                    }
                    
                    whistlesToSend--;
                    //logXML.setValue("//unsent", ofToString(whistlesToSend));
                    
                }
                
                if (whistlesToSend > 0) {
                    serverState = "OK: Sending unsent whistles";
                    connectionPlaylist.clear();
                    connectionPlaylist.addKeyFrame(Action::pause(1000.f));
                    connectionPlaylist.addKeyFrame(Action::event(this, "sendWhistle_delayed"));
                    connectionPlaylist.addKeyFrame(Action::pause(30000.f));
                    connectionPlaylist.addKeyFrame(Action::event(this, "keepAlive"));
                }
                
                
            }
            /*
            else if (serverResponseXML.getValue("//status") == "461") { // status no active campaign

                noActiveCampaign = true;
                serverState = "OK: There is no active campaign";

                if (whistlesToSend > 0) { // unsent whistles from this and previous campain should be removed
                    whistlesToSend = 0;
                    logXML.setValue("//unsent", ofToString(whistlesToSend));
                    logXML.save("log.xml");
                }

                if(response.request.name == "whistle"){
                    pixels[beakPixel] -> addBlink(2, status461);
                    pixels[maintenancePixel] -> addBlink(1, status461);
                }
            }
             */
            
            else {
                serverState = "BAD: Unimplemented server code. Correct SparrowID?";
                //ofLogError("Unimplemented server return status: " + serverResponseXML.getValue("//status") + " error: " + serverResponseXML.getValue("//error"));
                pixels[maintenancePixel] -> clearPlaylist();
                pixels[maintenancePixel] -> setColor(ofColor::yellow);
            }
        }

    }
    //else if(response.status == -1 || response.status == 302) {
    else {
        // -1  wifi off (mac)
        // 302 no wifi authentification (hotspot)
        //
        ofRemoveAllURLRequests();  //when the connection is down, urlResponse is triggered multiple times a second with code -1
        ofLog() << "Connection error. time: " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");
        isLoading = false;

        serverState = "Connection down? Retrying. Code: " + ofToString(response.status) + " - " + ofGetTimestampString("%Y-%n-%e T%H:%M:%S");
        
        hasConnectionError = true;
        connectionAlive = false;
        
        connectionPlaylist.clear(); // if there is a connection error, try again in 30 seconds
        connectionPlaylist.addKeyFrame(Action::pause(5000.f));
        connectionPlaylist.addKeyFrame(Action::event(this, "keepAlive"));
        
        pixels[maintenancePixel] -> clearPlaylist();
        pixels[maintenancePixel] -> setColor(ofColor::red);
    }
    
    if(response.status != -1 && response.status != 302 && response.status != 200) {
            ofLog() << "Unimplemented server response.status: " << response.status << " Time: " << ofGetTimestampString("%Y-%n-%e T%H:%M:%S");
    }

    if (response.request.name == "whistle" && freshMessage) {
        freshMessage = false;
        pixels[beakPixel] -> addBlink(1, status400);
    }
}



// helper functions
//--------------------------------------------------------------
ofColor ofApp::shiftColor(ofColor inputColor) {
    ofFloatColor randomColor;
    float r = 0.2;
    randomColor.set(ofRandom(-r, r), ofRandom(-r, r), ofRandom(-r, r));
    return inputColor + randomColor;
}

ofColor ofApp::shiftHue(ofColor inputColor) {
    float hueAngle = inputColor.getHueAngle();
    hueAngle += ofRandom(0, 30);
    if (!ofInRange(hueAngle, 0, 360)) {
        hueAngle -= 360;
    }
    inputColor.setHueAngle(hueAngle);
    return inputColor;
}

void ofApp::newDiscoColors() { // disco mode!
    // this code is not ready for beaks, eyes and maintainance pixels at other places than at the last 3 pixels of the LED strand.

    vector<ofColor> tempToColors;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            tempToColors.push_back(pixels[i] -> colorTo);
        }
    }

    ofRandomize(tempToColors); // identical to std::random_shuffle (tempToColors.begin(), tempToColors.end());

    int it = 0;
    for (int i = 0; i < pixelAmount; i++) { // populate the pixelRandomOrder with body pixels
        if (pixels[i]->partType == "body") {
            pixels[i]->colorFrom = pixels[i]->colorTo;
            pixels[i]->fadeAmount = 0;
            pixels[i]->colorTo = shiftHue(tempToColors[it]);
            it++;
        }
    }
}

// debug and log functions

//--------------------------------------------------------------
void ofApp::whistleButtonPressed(){
    onWhistle(debugWhistleFrequency, debugWhistleCertainty);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    //*
// 	if(key == 's') {
// 		gui.saveToFile("settings.xml");
// 	}
// 	if(key == 'l') {
// 		gui.loadFromFile("settings.xml");
// 	}
    if(key == 'w') {
		onWhistle(1000, 1);
	}
	
	if(key == 's') {
		sendMessageToServer("whistle");
	}
    //*/

}

//--------------------------------------------------------------
void ofApp::exit()
{
  whistleButton.removeListener(this,&ofApp::whistleButtonPressed);
  ofUnregisterURLNotification(this);
  ofxKeyframeAnimUnRegisterEvents(this);
}

//--------------------------------------------------------------
/*
void ofApp::addLogItem(string datetime, string animation, string serverresponse, float frequency){

	return;

    ofXml logAddition;

    logAddition.addChild("w");
    logAddition.setTo("w");

    logAddition.addValue("dt", datetime); // ISO8601_FORMAT
    logAddition.addValue("f", frequency); // whistle frequency
    //logAddition.addValue("t", gameType);
    //logAddition.addValue("ID", objectID);
    logAddition.addValue("a", animation);
    logAddition.addValue("s", serverresponse);

    if(!logXML.exists("//log")) {
        logXML.addChild("log");
    }
    //cout << "addLog 1: " << logXML.getName() << endl;
    logXML.setTo("//log");
    logXML.addXml(logAddition);
    //cout << "addLog 2: " << logXML.getName() << endl;
    logXML.setToParent();
    //cout << "addLog 3: " << logXML.getName() << endl;


    // placing the save in the whistle update loop to occur after whistling stops, so that we dont' hog the system saving the logfile all too often.
    //logXML.save("log.xml");
}
*/


// future developement
//--------------------------------------------------------------

/*
 void ofApp::savePixelPositions(){

 //to implement
 // json writer: http://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
 //http://forum.openframeworks.cc/t/create-your-own-json-data-and-save-json-file/16560

 #include<json/writer.h>
 Code:

 Json::Value event;
 Json::Value vec(Json::arrayValue);
 vec.append(Json::Value(1));
 vec.append(Json::Value(2));
 vec.append(Json::Value(3));

 event["competitors"]["home"]["name"] = "Liverpool";
 event["competitors"]["away"]["code"] = 89223;
 event["competitors"]["away"]["name"] = "Aston Villa";
 event["competitors"]["away"]["code"]=vec;

 std::cout << event << std::endl;


 //https://github.com/jefftimesten/ofxJSON

 }
 */

// windowless: http://forum.openframeworks.cc/t/windowless-of/1876/4

//WHISTLEDETECTOR

void ofApp::setupDetector()
{
    #if soundLoad
    // Set detector parameters
    // Use default parameters
     detector.setHzPerWindow(130);
     detector.setWhistlePowerThresholdPercent(90);
     detector.setMsecsPerWhistleDuration(msecsPerWhistleDuration); //150
     detector.setHzPerWhistleFrequencyDeviation(130);
     detector.setFramePowerThreshold(0.002f);
     detector.setUseOvertoneDetector(true);

    // Open detector. As a result the detector starts working in separate thread and collects
    // detected whistles in internal buffer. ofxIntegratedWhistleDetector::getNextWhistle() must be
    // used to retrieve available whistles in FIFO order from the buffer
    detector.open();
    ofLog() << "detector opened";
    
    #endif
}

void ofApp::setupSequenceDetector()
{
    #if soundLoad
    // Set sequence detector parameters
    /* Use default parameters */
    sequenceDetector.setWhistleCountInSequence(4);
    sequenceDetector.setMsecsPerStateGap(1000);
    sequenceDetector.setMsecsPerStateTimeout(10 * 1000);
    sequenceDetector.setMsecsPerFinalStateTimeout(timeoutTimeWhistles);
    #endif
}

/*
string ofApp::getDetectorParamsAsString() const
{
    // Print detector parameters actual values (these are corrected in accordance with FFT
    // resolution and processing step)
    string msg = "Detector Parameters:\n";
    msg += "hzPerWindow = " + ofToString(detector.hzPerWindow()) + "\n";
    msg += "whistlePowerThresholdPercent = " + ofToString(detector.whistlePowerThresholdPercent()) + "\n";
    msg += "msecsPerWhistleDuration = " + ofToString(detector.msecsPerWhistleDuration()) + "\n";
    msg += "hzPerWhistleFrequencyDeviation = " + ofToString(detector.hzPerWhistleFrequencyDeviation()) + "\n";
    msg += "framePowerThreshold = " + ofToString(detector.framePowerThreshold()) +
    " - MIC & MIXER SETTINGS DEPENDENT, CALIBRATION MAY NEED!\n";
    msg += "useOvertoneDetector = " + ofToString(detector.useOvertoneDetector() ? "Yes" : "No") + "\n";
    return msg;
}

string ofApp::getSequenceDetectorParamsAsString() const
{
    // Print sequence detector parameters actual values
    string msg = "Sequence Detector (State Machine) Parameters:\n";
    msg += "whistleCountInSequence = " + ofToString(sequenceDetector.whistleCountInSequence()) + "\n";
    msg += "msecsPerStateGap = " + ofToString(sequenceDetector.msecsPerStateGap()) + "\n";
    msg += "msecsPerStateTimeout = " + ofToString(sequenceDetector.msecsPerStateTimeout()) + "\n";
    msg += "msecsPerFinalStateTimeout = " + ofToString(sequenceDetector.msecsPerFinalStateTimeout()) + "\n";
    return msg;
}

string ofApp::getTransitionsAsString() const
{
    string msg;
    for (deque<pair<ofxWhistleSequenceDetector::Transition,
         ofxIntegratedWhistleDetector::Whistle> >::const_iterator i = transitions.begin();
         i != transitions.end(); ++i) {
        const ofxWhistleSequenceDetector::Transition& transition = i->first;
        const ofxIntegratedWhistleDetector::Whistle& whistle = i->second;
        msg += "-> " + transition.toString() + ".\n   " +
        (whistle.isNull() ?
         string("Go to initial state due to timeout.") :
         whistle.toString()) +
        "\n";
    }
    return msg;
}
 */

//*/


//tween(duration, TweenTarget, targetValue, BezierTween, startValue, delay_duration);
//tween type
//      TWEEN_BACK:
//      TWEEN_BOUNCE:
//      TWEEN_CIRC:
//      TWEEN_CUBIC:
//      TWEEN_ELASTIC:
//      TWEEN_EXPO:
//      TWEEN_LIN:
//      TWEEN_QUAD:
//      TWEEN_QUART:
//      TWEEN_QUINT:
//      TWEEN_SIN:

//      //tween transitions
//      TWEEN_PAUSE
//      TWEEN_EASE_IN
//      TWEEN_EASE_IN_OUT
//      TWEEN_EASE_OUT
//eyePlaylist.addToKeyFrame(	Action::tween(200.f, &rectPos.y, targetPosition.y));

