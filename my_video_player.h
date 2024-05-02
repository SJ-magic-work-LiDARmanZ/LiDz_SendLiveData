/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <ofMain.h>
#include "ofxSyphon.h"
#include "sj_common.h"

/************************************************************
************************************************************/
class MyVideoPlayer{
private:
	/****************************************
	****************************************/
	ofVideoPlayer video_player_;
	ofFbo fbo_mov_;
	ofxSyphonServer syphon_server_;
	
	int last_frame_ = -1;
	
	/****************************************
	****************************************/
	void SendSyphon();
	void PrintVideoInfo();
	int GetCurrentFrame();
	
public:
	/****************************************
	****************************************/
	MyVideoPlayer();
	~MyVideoPlayer();
	
	void SetUp(string str_file_name, float volume, string str_syphon_name);
	void JustUpdate();
	bool Update(bool b_send_syphon);
	void Draw(float x, float y, float w, float h);
	
	void setPosition(float ratio);
	void SeekToZero();
	float GetCurrentTime_s();
	float GetPosition();
	void SetFrame(int frame);
	
	void AlignTime(MyVideoPlayer& to);
	void JumpToForward(MyVideoPlayer& to);
	
	void TogglePlayPause();
	void Play();
	void Pause();
	
	ofFbo& GetFbo();
};
