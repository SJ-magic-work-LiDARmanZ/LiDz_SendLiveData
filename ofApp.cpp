/************************************************************
************************************************************/
#include "ofApp.h"

#include <algorithm> // to use std::min

/* for dir search */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <dirent.h>
#include <string>

// using namespace std;

/************************************************************
FilesInDir
************************************************************/

/******************************
******************************/
FilesInDir::~FilesInDir(){
	for(int i = 0; i < (int)f_log_.size(); i++){
		if(f_log_[i]) fclose(f_log_[i]);
	}
	
	for(int i = 0; i < file_names_.size(); i++){
		delete file_names_[i];
	}
	
	printf("> FilesInDir %s : dtr() \n", str_dir_.c_str());
	fflush(stdout);
}

/******************************
******************************/
void FilesInDir::MakeupFileTable(const string dirname)
{
	/********************
	********************/
	str_dir_ = dirname;
	printf("> search csv Files in %s\n", str_dir_.c_str());
	
	/********************
	********************/
	DIR *pDir;
	struct dirent *pEnt;
	struct stat wStat;
	string wPathName;

	pDir = opendir( str_dir_.c_str() );
	if ( NULL == pDir ) { ERROR_MSG(); std::exit(1); }
	
	pEnt = readdir( pDir );
	while ( pEnt ) {
		// .と..は処理しない
		if ( strcmp( pEnt->d_name, "." ) && strcmp( pEnt->d_name, ".." ) ) {
		
			wPathName = str_dir_ + "/" + pEnt->d_name;
			
			// ファイルの情報を取得
			if ( stat( wPathName.c_str(), &wStat ) ) {
				printf( "Failed to get stat %s \n", wPathName.c_str() );
				break;
			}
			
			if ( S_ISDIR( wStat.st_mode ) ) {
				// nothing.
			} else {
				vector<string> str = ofSplitString(pEnt->d_name, ".");
				if(str[str.size()-1] == "csv"){
					// string str_NewFileName = wPathName;
					// string str_NewFileName = pEnt->d_name;
					// string* str_NewFileName = new string(pEnt->d_name);	// data dir 直下にmp3置いている場合はこちらでOK
					string* str_NewFileName = new string(wPathName);		// 違う場合はFullPath.
					
					printf("%s\n", pEnt->d_name);
					
					file_names_.push_back(str_NewFileName);
				}
			}
		}
		
		pEnt = readdir( pDir ); // 次のファイルを検索する
	}

	closedir( pDir );
	
	/********************
	********************/
	if(file_names_.size() == 0){
		char buf[100];
		printf("> no files in %s\n", str_dir_.c_str());
		fflush(stdout);
		std::exit(1);
	}
 }

/******************************
******************************/
void FilesInDir::OpenFiles()
{
	/********************
	********************/
	printf("> open csv files in %s\n", str_dir_.c_str());
	
	/********************
	********************/
	f_log_.resize(file_names_.size());
	
	frame_id_.resize(file_names_.size());
	for(int i = 0; i < (int)frame_id_.size(); i++)	{ frame_id_[i] = -1; }
	
	file_fps_.resize(file_names_.size());
	
	/********************
	********************/
	for(int i = 0; i < (int)f_log_.size(); i++){
		/********************
		********************/
		f_log_[i] = fopen(file_names_[i]->c_str(), "r");
		if(!f_log_[i]){
			printf("> file open error : %s\n", file_names_[i]->c_str());
			std::exit(1);
		}else{
			printf("%3d:%s\n", i, file_names_[i]->c_str());
		}
		
		/********************
		********************/
		string str_line;
		GetOneLineFromFile(f_log_[i], str_line);
		Align_StringOfData(str_line);
		
		vector<string> block = ofSplitString(str_line, ",");
		if( (block.size() < 2) || (block[0] != "fps") ) { ERROR_MSG(); std::exit(1); }
		
		float fps = atof(block[1].c_str());
		if( (fps < 5) || (60 < fps) ) { ERROR_MSG(); std::exit(1); }
		file_fps_[i] = fps;
		printf("\tfps = %f\n", fps);
		
		/********************
		********************/
		if( feof(f_log_[i]) )	{ ERROR_MSG(); std::exit(1); }
	}
	printf("--------------------\n");
	fflush(stdout);
}

/******************************
******************************/
void FilesInDir::GetOneLineFromFile(FILE* fp, string& str){
	enum{
		kBufSize = 5000000,
	};
	char buf[kBufSize];
	if(fgets(buf, kBufSize, fp) == NULL){
		str = "";
	}else{
		str = buf;
	}
}

/******************************
******************************/
void FilesInDir::DiscardOneLine(FILE* fp){
	string str_line;
	GetOneLineFromFile(fp, str_line);
}

/******************************
******************************/
void FilesInDir::Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
******************************/
bool FilesInDir::IsEof()
{
	return feof(f_log_[file_id_]);
}

/******************************
******************************/
void FilesInDir::FSeekToZero(){
	/********************
	■C++におけるrewindのお作法
		https://sleepy-yoshi.hatenablog.com/entry/20120508/p1
	********************/
	clearerr(f_log_[file_id_]);
	fseek( f_log_[file_id_], 0, SEEK_SET );
	frame_id_[file_id_] = -1;
	DiscardOneLine(f_log_[file_id_]);
	
	if( feof(f_log_[file_id_]) ) { ERROR_MSG(); std::exit(1); } // means : no data
	
	/*
	f_log_[file_id_].clear();
	f_log_[file_id_].seekg(0, std::ios::beg);
	frame_id_[file_id_] = -1;
	
	DiscardOneLine(f_log_[file_id_]);
	
	if(f_log_[file_id_].eof()) { ERROR_MSG(); std::exit(1); } // means : no data
	*/
}

/******************************
******************************/
void FilesInDir::GetLine(string& str_line, int num_frames_ahead, bool b_log){
	enum{
		kBufSize = 5000000,
	};
	char buf[kBufSize];
	
	int t_from;
	if(b_log)	t_from = ofGetElapsedTimeMillis();
	
	if( feof(f_log_[file_id_]) ){
		str_line = "";
	}else{
		bool b_succeed = true;
		str_line = "";
		
		for(int num_read = 0; num_read < num_frames_ahead; num_read++){
			if( fgets(buf, kBufSize, f_log_[file_id_]) == NULL ){
				b_succeed = false;
				break;
			}else{
				frame_id_[file_id_]++;
			}
		}
		
		if(b_succeed) str_line = buf;
		Align_StringOfData(str_line);
	}
	
	if(b_log){
		printf( "> GetLine\n" );
		printf( "took %d[ms] to read %d[frames] = %.2f[sec]\n", (int)ofGetElapsedTimeMillis() - t_from, num_frames_ahead, (float)num_frames_ahead / file_fps_[file_id_] );
		printf( "%.2f [ms/frame]\n", (float)(ofGetElapsedTimeMillis() - t_from) / num_frames_ahead );
		printf( "%.2f [sec/10 min]\n", ( (float)(ofGetElapsedTimeMillis() - t_from) / num_frames_ahead ) * 10/*fps*/ * 600/* sec/10min */ / 1000/*to sec*/ );
		fflush(stdout);
	}
}

/******************************
******************************/
void FilesInDir::IdNext(){
	file_id_++;
	if(f_log_.size() <= file_id_) file_id_ = 0;
}

/******************************
******************************/
void FilesInDir::IdPrev(){
	file_id_--;
	if(file_id_ < 0) file_id_ = f_log_.size() - 1;
}

/******************************
******************************/
float FilesInDir::GetFps(){
	return file_fps_[file_id_];
}

/******************************
******************************/
int FilesInDir::GetId(){
	return file_id_;
}

/******************************
******************************/
bool FilesInDir::SetId(int id){
	if( (0 <= id) && (id < f_log_.size()) ){
		file_id_ = id;
		return true;
	}else{
		return false;
	}
}

/******************************
******************************/
int FilesInDir::GetFrame_Id(){
	return frame_id_[file_id_];
}

/******************************
******************************/
int FilesInDir::GetFileTime(){
	return (int)(frame_id_[file_id_] * 1000.0f / file_fps_[file_id_]);
}

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int max_points_in_1_frame, int send_at_once, int num_log_dirs)
: kSendAtOnce_(send_at_once)
, kNumDirs_( (num_log_dirs <= kMaxNumDirs_) ? num_log_dirs : kMaxNumDirs_ )
, num_points_in_this_frame_(0)
{
	/********************
	********************/
	font_M_.load("font/RictyDiminishedDiscord-Regular.ttf", 25/* font size in pixels */, true/* _bAntiAliased */, true/* _bFullCharacterSet */, false/* makeContours */, 0.3f/* simplifyAmt */, 72/* dpi */);
	
	/********************
	********************/
	positions_.resize(max_points_in_1_frame);
	
	/********************
	********************/
	for(int i = 0; i < kNumDirs_; i++){
		char buf[100];
		snprintf(buf, std::size(buf), "../../../data/Log/Log_%d", i);
		files_in_dir_[i].MakeupFileTable(buf);
		files_in_dir_[i].OpenFiles();
	}
}

/******************************
******************************/
ofApp::~ofApp(){
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("LiDz_SendLiveData");
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetWindowShape(kWindowWidth, kWindowHeight);
	ofSetEscapeQuitsApp(false);
	
	ofEnableAntiAliasing();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD

	/********************
	********************/
	setup_Gui();
	
	/********************
	********************/
	img_back_.load("ScreenDesign/img_back.png");
	
	SetupUdp();
	
	my_video_player_[(int)VideoType::Edited].SetUp("mov/mov_Edited.mp4", 1.0, "LiveVideo_Edited");
	my_video_player_[(int)VideoType::LightMain].SetUp("mov/mov_LightMain.mp4", 0.0, "LiveVideo_LightMain");
	
	/********************
	********************/
	extract_color.SetUp(my_video_player_[(int)VideoType::LightMain].GetFbo());
}

/******************************
******************************/
void ofApp::SetupUdp(){
	/********************
	********************/
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12345);
		settings.blocking = false;
		
		udp_send_.Setup(settings);
	}
	
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12349);
		settings.blocking = false;
		
		udp_send_pointsize_.Setup(settings);
	}
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("param", "gui.xml", 672, 234);
}

/******************************
******************************/
void ofApp::CalElapsedTime(){
	for(int i = 0; i < (int)VideoType::Num; i++){
		t_mov_[i].Set( my_video_player_[i].GetCurrentTime_s() );
	}
	
	t_lidar_.Set( (float)files_in_dir_[dir_id_].GetFileTime() / 1000 );
}

/******************************
******************************/
void ofApp::ResetAllVideo(){
	for(int i = 0; i < (int)VideoType::Num; i++ ){
		my_video_player_[i].SeekToZero();
	}
	
	files_in_dir_[dir_id_].FSeekToZero();
	
	b_skip_ = true;
	
	if( Gui_Global->b_always_send_udp_ ) PrepAndSendUdp();
}

/******************************
******************************/
void ofApp::update(){
	/********************
	********************/
	if(Gui_Global->Video_Button_Seek){
		printf("> Enter seek\n");
		fflush(stdout);
		
		Gui_Global->Video_Button_Seek = false;
		float pos = Gui_Global->Video_SeekPos;
		
		// my_video_player_[0].Pause();
		my_video_player_[0].setPosition(pos);
		// my_video_player_[1].setPosition(pos);
		
		b_seek_ = true;
	}
	
	/********************
	********************/
	bool b_loop = my_video_player_[0].Update(true); // loop ?
	my_video_player_[1].Update(true);
	
	if(b_loop){
		printf("> video loop\n");
		fflush(stdout);
		
		files_in_dir_[dir_id_].FSeekToZero();
	}
	
	/********************
	********************/
	float v_sec[int(VideoType::Num)];
	for(int i = 0; i < (int)VideoType::Num; i++ )	{ v_sec[i] = my_video_player_[i].GetCurrentTime_s(); }
	
	if( 0.08f < abs(v_sec[0] - v_sec[1]) )	{ my_video_player_[1].AlignTime( my_video_player_[0] ); }
	
	
	int frame_id_to_send = int( files_in_dir_[dir_id_].GetFps() * v_sec[0] );
	int num_frames_ahead = frame_id_to_send - files_in_dir_[dir_id_].GetFrame_Id();
	
	if(0 < num_frames_ahead){
		string str_line;
		files_in_dir_[dir_id_].GetLine(str_line, num_frames_ahead, b_seek_);
		b_seek_ = false;
		
		if( !CheckIfContentsExist(str_line) ){
			if(b_skip_) { ERROR_MSG(); std::exit(1); }
			
			printf("> lidar loop\n");
			
			my_video_player_[0].SeekToZero();
			files_in_dir_[dir_id_].FSeekToZero();
			if( Gui_Global->b_always_send_udp_ ) PrepAndSendUdp();
			
			b_skip_ = true;
		}else{
			b_skip_ = false;
			UpdatePosition_with_ReadString(str_line);
			PrepAndSendUdp();
		}
	}else{
		if( Gui_Global->b_always_send_udp_ ) PrepAndSendUdp();
	}
	
	/********************
	********************/
	CalElapsedTime();
	
	/********************
	********************/
	extract_color.Update(my_video_player_[(int)VideoType::LightMain].GetFbo());
}

/******************************
******************************/
void ofApp::UpdatePosition_with_ReadString(const string& str_line){
	/********************
	********************/
	vector<string> block = ofSplitString(str_line, ",");
	num_points_in_this_frame_ = (int)block.size() / 3;
	if( ((int)block.size() < 3) || ((int)block.size() % 3 != 0) || ((int)positions_.size() < (int)block.size()/3) ) { ERROR_MSG(); std::exit(1); }
	
	/********************
	********************/
	for(int i = 0; i < positions_.size(); i++){
		float x, y, z;
		if( i < num_points_in_this_frame_ ){
			x = atof(block[i * 3 + 0].c_str());
			y = atof(block[i * 3 + 1].c_str());
			z = atof(block[i * 3 + 2].c_str());
			
		}else{
			float far = 10000;
			x = far;
			y = far;
			z = far;
		}
		
		positions_[i].set(x, y, z);
	}
	
	/********************
	********************/
	switch( (int)Gui_Global->limit_points_coord_ ){
		case 0:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByX); // 比較関数cmpを使用してsort
			break;
			
		case 1:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByZ); // 比較関数cmpを使用してsort
			break;
			
		case 2:
			std::sort(positions_.begin(), positions_.end(), CmpVector3ByDistance); // 比較関数cmpを使用してsort
			break;
	}
	
	/********************
	********************/
	num_points_to_send_ = std::min( num_points_in_this_frame_, (int)Gui_Global->limit_points_to_send_ );
	
	/********************
	********************/
	num_packets_ = num_points_to_send_ / kSendAtOnce_;
	if(num_points_to_send_ % kSendAtOnce_ != 0) num_packets_++;
}

/******************************
******************************/
bool ofApp::CmpVector3ByX(const ofVec3f &a, const ofVec3f &b)
{
	return std::abs(a.x) < std::abs(b.x);	// 昇順
}

/******************************
******************************/
bool ofApp::CmpVector3ByZ(const ofVec3f &a, const ofVec3f &b)
{
	return std::abs(a.z) < std::abs(b.z);	// 昇順
}

/******************************
******************************/
bool ofApp::CmpVector3ByDistance(const ofVec3f &a, const ofVec3f &b)
{
	// return std::abs(a.x * a.x + a.z * a.z) < std::abs(b.x * b.x + b.z * b.z);	// 昇順
	return a.squareDistance( ofVec3f(0, 0, 0) ) < b.squareDistance( ofVec3f(0, 0, 0) ); // 昇順
}

/******************************
******************************/
void ofApp::Align_StringOfData(string& s)
{
	size_t pos;
	while((pos = s.find_first_of(" 　\t\n\r")) != string::npos){ // 半角・全角space, \t 改行 削除
		s.erase(pos, 1);
	}
}

/******************************
description
	str_line must be aligned before call.
******************************/
bool ofApp::CheckIfContentsExist(const string& str_line)
{
	vector<string> block = ofSplitString(str_line, ",");
	if( (block.size() == 0) || (block[0] == "") ){ // no_data or exist text but it's",,,,,,,".
		return false;
	}else{
		return true;
	}
}

/******************************
******************************/
void ofApp::PrepAndSendUdp(){
	/********************
	********************/
	const int kBufSize = 100;
	
	for(int ofs = 0, id = 0; ofs < num_points_to_send_; ofs += kSendAtOnce_, id++){
		int count = std::min(kSendAtOnce_, num_points_to_send_ - ofs); // positions_.size() は unsigned なので、castしないと、build通らない
		
		string message = "/pos,";
		if(Gui_Global->b_send_udp_dynamic_param) message = "/pos_and_dynamic_param,";
		
		{
			char buf[kBufSize];
			snprintf(buf, std::size(buf), "%d,%d,%d,%d,%d,", 0/* grop_id */, num_points_to_send_, num_packets_, id, ofs); // debug as realtime
			message += buf;
		}
		
		if(Gui_Global->b_send_udp_dynamic_param){
			char buf[kBufSize];
			snprintf(buf, std::size(buf), "%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", point_size_[dir_id_].no_sync_, point_size_[dir_id_].h_, point_size_[dir_id_].l_, point_ofs_[dir_id_].x, point_ofs_[dir_id_].y, point_ofs_[dir_id_].z, rot_deg_[dir_id_].x, rot_deg_[dir_id_].y, rot_deg_[dir_id_].z);
			
			message += buf;
		}
		
		for(int i = 0; i < count; i++){
			char buf[kBufSize];
			snprintf(buf, std::size(buf), "%.2f,%.2f,%.2f", positions_[i + ofs].x, positions_[i + ofs].y, positions_[i + ofs].z );
			message += buf;
			
			if(i < count - 1)	{ message += ","; }
		}
		
		udp_send_.Send(message.c_str(), message.length());
	}
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	ofBackground(30);
	
	/********************
	********************/
	ofSetColor(255);
	ofFill();
	
	img_back_.draw(0, 0);
	my_video_player_[(int)VideoType::Edited].Draw(10, 10, kPlayVideoWidth, kPlayVideoHeight);
	my_video_player_[(int)VideoType::LightMain].Draw(10, 380, kPlayVideoWidth, kPlayVideoHeight);
	
	/********************
	********************/
	extract_color.Draw();
	
	/********************
	********************/
	ofSetColor(255);
	ofFill();
	
	char buf[500];
	
	// mov[0]
	snprintf(buf, std::size(buf), "mov[0]");
	font_M_.drawString(buf, 670, 40);
	
	snprintf(buf, std::size(buf), "%02d:%02d:%02d %03d", t_mov_[0].get_hour(), t_mov_[0].get_min(), t_mov_[0].get_sec(), t_mov_[0].get_ms() );
	font_M_.drawString(buf, 761, 40);
	
	// mov[1]
	snprintf(buf, std::size(buf), "mov[1]");
	font_M_.drawString(buf, 670, 71);
	
	snprintf(buf, std::size(buf), "%02d:%02d:%02d %03d", t_mov_[1].get_hour(), t_mov_[1].get_min(), t_mov_[1].get_sec(), t_mov_[1].get_ms() );
	font_M_.drawString(buf, 761, 71);
	
	// lidar
	snprintf(buf, std::size(buf), "lidar");
	font_M_.drawString(buf, 670, 123);
	
	snprintf(buf, std::size(buf), "%02d:%02d:%02d %03d", t_lidar_.get_hour(), t_lidar_.get_min(), t_lidar_.get_sec(), t_lidar_.get_ms() );
	font_M_.drawString(buf, 761, 123);
	
	snprintf(buf, std::size(buf), "= %.2f", my_video_player_[0].GetPosition() );
	font_M_.drawString(buf, 836, 165);
	
	/********************
	********************/
	Gui_Global->gui.draw();
}

/******************************
******************************/
void ofApp::exit(){

}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch( key ){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				int id = key - '0';
				if(id < sizeof(kMusicStartPos_)/sizeof(float)){
					my_video_player_[0].setPosition(kMusicStartPos_[id]);
					my_video_player_[1].setPosition(kMusicStartPos_[id]);
				}else{
					printf("set position : out of range.\n");
					fflush(stdout);
				}
			}
			break;
			
		case ' ':
			for( int i = 0; i < (int)VideoType::Num; i++ ){
				my_video_player_[i].TogglePlayPause();
			}
			break;
			
		case 'a':
			my_video_player_[0].SeekToZero();
			break;
			
		case 'o':
			Gui_Global->gui.maximizeAll();
			break;
			
		case 'm':
			Gui_Global->gui.minimizeAll();
			break;
			
		case 'c':
			extract_color.MoveRange_Centering();
			break;
			
		case OF_KEY_UP:
			if(b_shift)	extract_color.MoveRange_Up(10);
			else		extract_color.MoveRange_Up(1);
			
			break;
			
		case OF_KEY_DOWN:
			if(b_shift)	extract_color.MoveRange_Down(10);
			else		extract_color.MoveRange_Down(1);
			break;
			
		case OF_KEY_RIGHT:
			if(b_shift)	extract_color.MoveRange_Right(10);
			else		extract_color.MoveRange_Right(1);

			break;
			
		case OF_KEY_LEFT:
			if(b_shift)	extract_color.MoveRange_Left(10);
			else		extract_color.MoveRange_Left(1);
			break;
		
		case OF_KEY_SHIFT:
			b_shift = true;
			break;
	}
}

/******************************
******************************/
void ofApp::keyReleased(int key){
	switch(key){
		case OF_KEY_SHIFT:
			b_shift = false;
			break;
		
	}
}

/******************************
******************************/
void ofApp::mouseMoved(int x, int y ){

}

/******************************
******************************/
void ofApp::mouseDragged(int x, int y, int button){
	extract_color.mouseDragged(x, y, button);
}

/******************************
******************************/
void ofApp::mousePressed(int x, int y, int button){
	if(b_shift)	extract_color.mousePressed(x, y, button);
}

/******************************
******************************/
void ofApp::mouseReleased(int x, int y, int button){
	extract_color.mouseReleased(x, y, button);
}

/******************************
******************************/
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

/******************************
******************************/
void ofApp::mouseEntered(int x, int y){

}

/******************************
******************************/
void ofApp::mouseExited(int x, int y){

}

/******************************
******************************/
void ofApp::windowResized(int w, int h){

}

/******************************
******************************/
void ofApp::gotMessage(ofMessage msg){

}

/******************************
******************************/
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
