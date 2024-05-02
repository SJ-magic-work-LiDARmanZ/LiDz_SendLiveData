/************************************************************
************************************************************/
#include "sj_common.h"

/************************************************************
************************************************************/
/********************
********************/
int GPIO_0 = 0;
int GPIO_1 = 0;

/********************
********************/
GUI_GLOBAL* Gui_Global = NULL;

FILE* fp_Log = nullptr;


/************************************************************
func
************************************************************/
/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt)
{
	double Alpha;
	if((Alpha_dt <= 0) || (Alpha_dt < dt))	Alpha = 1;
	else									Alpha = 1/Alpha_dt * dt;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha)
{
	if(Alpha < 0)		Alpha = 0;
	else if(1 < Alpha)	Alpha = 1;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double sj_max(double a, double b)
{
	if(a < b)	return b;
	else		return a;
}

/******************************
******************************/
bool checkIf_ContentsExist(char* ret, char* buf)
{
	if( (ret == NULL) || (buf == NULL)) return false;
	
	string str_Line = buf;
	Align_StringOfData(str_Line);
	vector<string> str_vals = ofSplitString(str_Line, ",");
	if( (str_vals.size() == 0) || (str_vals[0] == "") ){ // no_data or exist text but it's",,,,,,,".
		return false;
	}else{
		return true;
	}
}

/******************************
******************************/
void Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
******************************/
void print_separatoin()
{
	printf("---------------------------------\n");
}

/******************************
******************************/
void ClearFbo(ofFbo& fbo)
{
	fbo.begin();
		ofClear(0, 0, 0, 0);
	fbo.end();
}


/************************************************************
class
************************************************************/

/******************************
******************************/
void GUI_GLOBAL::setup(string GuiName, string FileName, float x, float y)
{
	/********************
	********************/
	gui.setup(GuiName.c_str(), FileName.c_str(), x, y);
	
	/********************
	********************/
	Group_Video.setup("Video");
		Group_Video.add(Video_Button_Seek.setup("Seek", false));
		Group_Video.add(Video_SeekPos.setup("SeekPos", 0, 0.0, 1.0));
	gui.add(&Group_Video);
	
	Group_Limit.setup("Limit");
		Group_Limit.add(limit_points_coord_.setup("limit_points_coord_", 0.0, 0.0, 2.0));
		Group_Limit.add(limit_points_to_send_.setup("limit_points_to_send_", 42500, 0.0, 57600));
	gui.add(&Group_Limit);
	
	Group_ExtractColor.setup("ExtractColor");
		Group_ExtractColor.add(col_sync_type.setup("sync_type", 2, 0.0, 5.0));
		Group_ExtractColor.add(a_col_fade.setup("a_col_fade", 0.1, 0.0, 1.0));
		
		Group_ExtractColor.add(default_hue_near.setup("default_hue_near", 0.0, 0.0, 255));
		Group_ExtractColor.add(default_hue_far.setup("default_hue_far", 166, 0.0, 255));
		Group_ExtractColor.add(default_saturation.setup("default_saturation", 191, 0.0, 255));
		Group_ExtractColor.add(default_brightness.setup("default_brightness", 204, 0.0, 255));
		
		Group_ExtractColor.add(b_print_selected_range.setup("print_selected_range", false));
	gui.add(&Group_ExtractColor);
	
	Group_misc.setup("misc");
		Group_misc.add(b_always_send_udp_.setup("b_always_send_udp", true));
		Group_misc.add(b_send_udp_dynamic_param.setup("b_send_udp_dynamic_param", true));
	gui.add(&Group_misc);

	/********************
	********************/
	gui.minimizeAll();
}

