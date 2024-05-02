/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxNetwork.h"

#include "my_video_player.h"
#include "sj_common.h"

#include "extract_color.h"

/************************************************************
************************************************************/

/**************************************************
**************************************************/
class ElapsedTime{
private:
	int ms_;
	int sec_;
	int minute_;
	int hour_;
	
public:
	/******************************
	******************************/
	void Set(float sec){
		/********************
		********************/
		ms_	= (int)((sec - (int)sec) * 1000);
		
		/********************
		********************/
		sec_	= (int)sec;
		
		hour_	=  sec_ / (60 * 60);
		sec_	-= hour_ * (60 * 60);
		
		minute_	=  sec_ / 60;
		sec_	-= minute_ * 60;
	}
	
	int get_ms()	{ return ms_; }
	int get_100ms()	{ return ms_ / 100; }
	
	int get_sec()	{ return sec_; }
	int get_min()	{ return minute_; }
	int get_hour()	{ return hour_; }
};

/**************************************************
**************************************************/
class FilesInDir{
private:
	string str_dir_ = "not set yet";
	
	// vector<std::ifstream> f_log_;
	vector<FILE*> f_log_;
	vector<string*> file_names_;
	vector<int> frame_id_;
	vector<float> file_fps_;
	
	int file_id_ = 0;
	
	void Align_StringOfData(string& s);
	void DiscardOneLine(FILE* fp);
	
public:
	~FilesInDir();
	void MakeupFileTable(const string dirname);
	void OpenFiles();
	bool IsEof();
	void FSeekToZero();
	void GetOneLineFromFile(FILE* fp, string& str);
	void GetLine(string& str_line, int num_frames_ahead, bool b_log);
	void IdNext();
	void IdPrev();
	bool SetId(int id);
	int GetId();
	int GetFrame_Id();
	int GetFileTime();
	float GetFps();
};

/**************************************************
**************************************************/
struct PointSize{
	float no_sync_;
	float h_;
	float l_;
	
	PointSize(float no_sync, float h, float l)
	: no_sync_(no_sync)
	, h_(h)
	, l_(l)
	{
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		kPlayVideoWidth		= 640,
		kPlayVideoHeight	= 360,
	};
	
	enum class VideoType{
		Edited,
		LightMain,
		
		Num,
	};
	
	/****************************************
	****************************************/
	ofImage img_back_;
	MyVideoPlayer my_video_player_[(int)VideoType::Num];
	
	ElapsedTime t_mov_[(int)VideoType::Num];
	ElapsedTime t_lidar_;
	
	ofTrueTypeFont font_M_;
	
	/********************
	********************/
	enum{
		kMaxNumDirs_ = 10,
	};
	const int kNumDirs_;
	FilesInDir files_in_dir_[kMaxNumDirs_];
	
	int dir_id_ = 0;
	
	PointSize point_size_[kMaxNumDirs_] = {
		PointSize(0.035, 0.133, 0.03),	// 0
		
		PointSize(0.035, 0.043, 0.02),	// 1
		PointSize(0.035, 0.07, 0.03),	// 2
		PointSize(0.035, 0.07, 0.03),	// 3
		PointSize(0.035, 0.07, 0.03),	// 4
		PointSize(0.035, 0.07, 0.03),	// 5
		PointSize(0.035, 0.07, 0.03),	// 6
		PointSize(0.035, 0.07, 0.03),	// 7
		PointSize(0.035, 0.07, 0.03),	// 8
		PointSize(0.035, 0.07, 0.03),	// 9
	};
	
	const glm::vec3 point_ofs_[kMaxNumDirs_] = {
		glm::vec3(0, 0, 0), 	// 0
		
		glm::vec3(0, 0, 0), 	// 1
		glm::vec3(0, 0, 0), 	// 2
		glm::vec3(0, 0, 0), 	// 3
		glm::vec3(0, 0, 0), 	// 4
		glm::vec3(0, 0, 0), 	// 5
		glm::vec3(0, 0, 0), 	// 6
		glm::vec3(0, 0, 0), 	// 7
		glm::vec3(0, 0, 0), 	// 8
		glm::vec3(0, 0, 0), 	// 9
	};
	
	const glm::vec3 rot_deg_[kMaxNumDirs_] = {
		glm::vec3(0, 180, 0), 	// 0
		
		glm::vec3(0, 180, 0), 	// 1
		glm::vec3(0, 180, 0), 	// 2
		glm::vec3(0, 180, 0), 	// 3
		glm::vec3(0, 180, 0), 	// 4
		glm::vec3(0, 180, 0), 	// 5
		glm::vec3(0, 180, 0), 	// 6
		glm::vec3(0, 180, 0), 	// 7
		glm::vec3(0, 180, 0), 	// 8
		glm::vec3(0, 180, 0), 	// 9
	};
	
	/********************
	********************/
	const int kSendAtOnce_;
	
	int num_points_in_this_frame_;
	int num_points_to_send_;
	int num_packets_;
	
	vector<ofVec3f> positions_;
	
	/********************
	********************/
	const int kUdpBufSize_ = 100000;
	
	ofxUDPManager udp_send_;
	ofxUDPManager udp_send_pointsize_;
	bool b_skip_ = false;
	bool b_seek_ = false;
	
	/********************
	********************/
	ExtractColor extract_color;
	bool b_shift = false;
	
	/********************
	********************/
	const float kMusicStartPos_[4] = {
		0,
		0.234840,
		0.426739,
		0.708808
	};
	
	/****************************************
	****************************************/
	void ResetAllVideo();
	void CalElapsedTime();
	void SetupUdp();
	void UpdatePosition_with_ReadString(const string& str_line);
	void Align_StringOfData(string& s);
	bool CheckIfContentsExist(const string& str_line);
	void PrepAndSendUdp();
	
	static bool CmpVector3ByX(const ofVec3f &a, const ofVec3f &b);
	static bool CmpVector3ByZ(const ofVec3f &a, const ofVec3f &b);
	static bool CmpVector3ByDistance(const ofVec3f &a, const ofVec3f &b);
	
public:
	/****************************************
	****************************************/
	void setup_Gui();
	
	/****************************************
	****************************************/
	ofApp(int max_points_in_1_frame, int send_at_once, int num_log_dirs);
	~ofApp();
	
	void setup() override;
	void update() override;
	void draw() override;
	void exit() override;
	
	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mouseMoved(int x, int y ) override;
	void mouseDragged(int x, int y, int button) override;
	void mousePressed(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
	void mouseEntered(int x, int y) override;
	void mouseExited(int x, int y) override;
	void windowResized(int w, int h) override;
	void dragEvent(ofDragInfo dragInfo) override;
	void gotMessage(ofMessage msg) override;
};
