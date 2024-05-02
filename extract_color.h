/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <ofMain.h>
#include "sj_common.h"

#include "ofxNetwork.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
struct my_HSB{
	float h;
	float s;
	float b;
	
	void Fade(my_HSB to, float alpha){
		h = alpha * to.h + (1 - alpha) * h;
		s = alpha * to.s + (1 - alpha) * s;
		b = alpha * to.b + (1 - alpha) * b;
	}
};

/**************************************************
**************************************************/
class ExtractColor{
private:
	/****************************************
	****************************************/
	/********************
	********************/
	enum class State{
		WaitInput,
		Selecting,
		Selected,
	};
	State state_ = State::WaitInput;
	
	/********************
	********************/
	ofPixels pixels_;
	
	const glm::vec2 kResultDrawTo_		= glm::vec2(670, 660);
	const glm::vec2 kResultDrawSize_	= glm::vec2(80, 80);
	
	const glm::vec2 kMovDrawedTo_	= glm::vec2(10, 380);
	const glm::vec2 kMovDrawedSize_ = glm::vec2(640, 360);
	
	/********************
	********************/
	glm::vec2 fbo_size_;
	
	glm::vec2 mouse_dragged_from_	= glm::vec2(251.5, 437);
	glm::vec2 mouse_dragged_to_		= glm::vec2(392.5, 576);
	
	const glm::vec2 kMinRange_ = glm::vec2(10, 1);
	const glm::vec2 kMaxRange_ = glm::vec2(1920, 200);
	
	glm::vec2 selected_range_in_fbo_from_;
	glm::vec2 selected_range_in_fbo_to_;
	
	ofColor result_col_ = ofColor(255, 255, 255);
	
	/********************
	********************/
	const int kUdpBufSize_ = 100000;
	ofxUDPManager udp_receive_;
	ofxUDPManager udp_send_sync_color_;
	
	ofTrueTypeFont font_M_;
	
	/********************
	********************/
	enum class State_BoostLed{
		Normal,
		Boost,
	};
	State_BoostLed state_boost_led = State_BoostLed::Normal;
	float col_sync_type_saved_;
	
	
	/****************************************
	****************************************/
	void SetupUdp();
	bool IsInRange(float x, float y);
	void SetRangeInFbo();
	void PrepAndSendUdp();
	void SetResultColor(const ofColor & read_col);
	void TryToReceiveUdpMessage();
	void StateChart_BoostLed(int boost_led_type_id);
	
public:
	/****************************************
	****************************************/
	ExtractColor();
	~ExtractColor();
	
	void SetUp(ofFbo & fbo);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	
	void CheckAndApply_NewRange(glm::vec2 from, glm::vec2 to);
	void MoveRange_Centering();
	void MoveRange_Down(int step);
	void MoveRange_Up(int step);
	void MoveRange_Right(int step);
	void MoveRange_Left(int step);
	
	void Update(ofFbo & fbo);
	void Draw();
};
