/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include <stdio.h>

#include <ofMain.h>
#include <ofxGui.h>

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/

enum{
	kWindowWidth	= 940,
	kWindowHeight	= 750,
};


/************************************************************
************************************************************/

/**************************************************
**************************************************/
class GUI_GLOBAL{
private:
	/****************************************
	****************************************/
	
public:
	/****************************************
	****************************************/
	void setup(string GuiName, string FileName = "gui.xml", float x = 10, float y = 10);
	
	ofxGuiGroup Group_Video;
		ofxToggle Video_Button_Seek;
		ofxFloatSlider Video_SeekPos;
		
	ofxGuiGroup Group_Limit;
		ofxFloatSlider limit_points_coord_;
		ofxFloatSlider limit_points_to_send_;
		
	ofxGuiGroup Group_misc;
		ofxToggle b_always_send_udp_;
		ofxToggle b_send_udp_dynamic_param;
		
	ofxGuiGroup Group_ExtractColor;
		ofxFloatSlider col_sync_type;
		ofxFloatSlider a_col_fade;
		
		ofxFloatSlider default_hue_near;
		ofxFloatSlider default_hue_far;
		ofxFloatSlider default_saturation;
		ofxFloatSlider default_brightness;
		
		ofxToggle b_print_selected_range;
		
	ofxPanel gui;
	
	bool b_Disp = false;
};

/************************************************************
************************************************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt);
double LPF(double LastVal, double CurrentVal, double Alpha);
double sj_max(double a, double b);

bool checkIf_ContentsExist(char* ret, char* buf);
void Align_StringOfData(string& s);
void print_separatoin();
void ClearFbo(ofFbo& fbo);

/************************************************************
************************************************************/
extern GUI_GLOBAL* Gui_Global;

extern FILE* fp_Log;

extern int GPIO_0;
extern int GPIO_1;


/************************************************************
************************************************************/

