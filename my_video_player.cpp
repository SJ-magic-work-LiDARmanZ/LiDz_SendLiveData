/************************************************************
************************************************************/
#include "my_video_player.h"

/************************************************************
************************************************************/

/******************************
******************************/
MyVideoPlayer::MyVideoPlayer(){
}

/******************************
******************************/
MyVideoPlayer::~MyVideoPlayer(){
}

/******************************
******************************/
void MyVideoPlayer::SetUp(string str_file_name, float volume, string str_syphon_name){
	/********************
	********************/
	if( !video_player_.load(str_file_name.c_str()) )	{ ERROR_MSG(); std::exit(1); }
	else												{ PrintVideoInfo(); }
	
	if( video_player_.getDuration() < 5.0f )	{ ERROR_MSG(); std::exit(1); }
	
	video_player_.setLoopState(OF_LOOP_NORMAL); // OF_LOOP_NONE, OF_LOOP_NORMAL
	video_player_.setSpeed(1);
	video_player_.setVolume(volume);
	video_player_.setPaused(false);
	video_player_.stop();
	SeekToZero();
	
	fbo_mov_.allocate( video_player_.getWidth(), video_player_.getHeight(), GL_RGBA );
	ClearFbo(fbo_mov_);
	
	syphon_server_.setName(str_syphon_name.c_str()); // server name
}

/******************************
******************************/
void MyVideoPlayer::PrintVideoInfo(){
	printf( "(%.2f, %.2f), total length = %.2f [sec]\n", video_player_.getWidth(), video_player_.getHeight(), video_player_.getDuration() );
	fflush(stdout);
}

/******************************
******************************/
void MyVideoPlayer::SeekToZero(){
	video_player_.setPosition(0);
}

/******************************
******************************/
void MyVideoPlayer::setPosition(float ratio){
	video_player_.setPosition(ratio);
}

/******************************
******************************/
void MyVideoPlayer::JustUpdate(){
	video_player_.update();
}

/******************************
******************************/
bool MyVideoPlayer::Update(bool b_send_syphon){
	/********************
	********************/
	bool b_loop = false;
	
	/********************
	********************/
	video_player_.update();
	if( video_player_.isFrameNew() ){
		ofEnableAntiAliasing();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA); // OF_BLENDMODE_DISABLED, OF_BLENDMODE_ALPHA, OF_BLENDMODE_ADD
		
		fbo_mov_.begin();
			ofBackground(0);
			ofSetColor(255);
			video_player_.draw(0, 0, fbo_mov_.getWidth(), fbo_mov_.getHeight());
		fbo_mov_.end();
		
		if(b_send_syphon){
			SendSyphon();
		}
		
		if( video_player_.getCurrentFrame() < last_frame_ ){
			b_loop = true;
		}
		last_frame_ = video_player_.getCurrentFrame();
	}
	
	/********************
	********************/
	return b_loop;
}

/******************************
******************************/
void MyVideoPlayer::SendSyphon(){
	ofTexture tex = fbo_mov_.getTexture();
	syphon_server_.publishTexture(&tex);
}

/******************************
******************************/
float MyVideoPlayer::GetPosition(){
	return video_player_.getPosition();
}

/******************************
******************************/
float MyVideoPlayer::GetCurrentTime_s(){
	return video_player_.getDuration() * video_player_.getPosition();
}

/******************************
******************************/
int MyVideoPlayer::GetCurrentFrame(){
	return video_player_.getCurrentFrame();
}

/******************************
******************************/
void MyVideoPlayer::SetFrame(int frame){
	video_player_.setFrame(frame);
}

/******************************
******************************/
void MyVideoPlayer::JumpToForward(MyVideoPlayer& to){
	video_player_.setFrame(to.GetCurrentFrame() + 10);
}

/******************************
******************************/
void MyVideoPlayer::AlignTime(MyVideoPlayer& to){
	video_player_.setFrame(to.GetCurrentFrame());
	// video_player_.setPosition(to.GetPosition());
}

/******************************
******************************/
void MyVideoPlayer::Draw(float x, float y, float w, float h){
	fbo_mov_.draw(x, y, w, h);
}

/******************************
******************************/
void MyVideoPlayer::TogglePlayPause(){
	if( video_player_.isPlaying() )	Pause();
	else							Play();
}

/******************************
******************************/
void MyVideoPlayer::Play(){
	video_player_.setPaused(false);
}

/******************************
******************************/
void MyVideoPlayer::Pause(){
	video_player_.setPaused(true);
}

/******************************
******************************/
ofFbo& MyVideoPlayer::GetFbo(){
	return fbo_mov_;
}

