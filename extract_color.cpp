/************************************************************
************************************************************/
#include "extract_color.h"

/************************************************************
************************************************************/

/******************************
******************************/
ExtractColor::ExtractColor(){
	font_M_.load("font/RictyDiminishedDiscord-Regular.ttf", 25/* font size in pixels */, true/* _bAntiAliased */, true/* _bFullCharacterSet */, false/* makeContours */, 0.3f/* simplifyAmt */, 72/* dpi */);
}

/******************************
******************************/
ExtractColor::~ExtractColor(){
}

/******************************
******************************/
void ExtractColor::SetUp(ofFbo & fbo){
	fbo_size_.x = fbo.getWidth();
	fbo_size_.y = fbo.getHeight();
	
	pixels_.allocate(fbo_size_.x, fbo_size_.y, GL_RGBA);
	
	SetupUdp();
	
	SetRangeInFbo();
	state_ = State::Selected;
}

/******************************
******************************/
void ExtractColor::SetupUdp(){
	/********************
	********************/
	{
		ofxUDPSettings settings;
		settings.sendTo("127.0.0.1", 12349);
		settings.blocking = false;
		
		udp_send_sync_color_.Setup(settings);
	}
	{ // udp_receive_
		ofxUDPSettings settings;
		settings.receiveOn(12351);
		settings.blocking = false;
		
		udp_receive_.Setup(settings);
	}
}


/******************************
******************************/
bool ExtractColor::IsInRange(float x, float y){
	bool ret = false;
	
	float margin = 5;
	
	if( (kMovDrawedTo_.x + margin <= x) && (x <= kMovDrawedTo_.x + kMovDrawedSize_.x - margin) ){
		if( (kMovDrawedTo_.y + margin <= y) && (y <= kMovDrawedTo_.y + kMovDrawedSize_.y - margin) ){
			ret = true;
		}
	}
	
	return ret;
}

/******************************
******************************/
void ExtractColor::mouseDragged(int x, int y, int button){
	if(button != 0)	return;
	
	if( state_ == State::Selecting ){
		mouse_dragged_to_.x = x;
		mouse_dragged_to_.y = y;
	}
}

/******************************
******************************/
void ExtractColor::mousePressed(int x, int y, int button){
	if(button != 0)	return;
	
	if( IsInRange(x, y) ){
		mouse_dragged_from_.x = x;
		mouse_dragged_from_.y = y;
		
		mouse_dragged_to_ = mouse_dragged_from_;
		
		state_ = State::Selecting;
	}
}

/******************************
******************************/
void ExtractColor::mouseReleased(int x, int y, int button){
	if(button != 0)	return;
	
	if( state_ == State::Selecting ){
		if( IsInRange(x, y) ){
			/********************
			********************/
			mouse_dragged_to_.x = x;
			mouse_dragged_to_.y = y;
			
			/********************
			********************/
			glm::vec2 from;
			glm::vec2 to;
			
			from.x = min(mouse_dragged_from_.x, mouse_dragged_to_.x);
			from.y = min(mouse_dragged_from_.y, mouse_dragged_to_.y);
			
			to.x = max(mouse_dragged_from_.x, mouse_dragged_to_.x);
			to.y = max(mouse_dragged_from_.y, mouse_dragged_to_.y);
			
			if( (to.x - from.x + 1 < kMinRange_.x) || (to.y - from.y + 1 < kMinRange_.y) ){
				printf("selected range too small\n");
				state_ = State::WaitInput;
				return;
			}
			
			if(kMaxRange_.x < to.x - from.x + 1)	{ to.x = from.x + kMaxRange_.x - 1; }
			if(kMaxRange_.y < to.y - from.y + 1)	{ to.y = from.y + kMaxRange_.y - 1; }
			
			/* */
			mouse_dragged_from_	= from;
			mouse_dragged_to_	= to;
			
			/* */
			SetRangeInFbo();
			
			/********************
			********************/
			state_ = State::Selected;
			
		}else{
			state_ = State::WaitInput;
		}
	}
}

/******************************
******************************/
void ExtractColor::SetRangeInFbo(){
	float ratio = fbo_size_.x / kMovDrawedSize_.x;
	
	selected_range_in_fbo_from_	= (mouse_dragged_from_	- kMovDrawedTo_) * ratio;
	selected_range_in_fbo_to_	= (mouse_dragged_to_	- kMovDrawedTo_) * ratio;
	
	if(Gui_Global->b_print_selected_range){
		printf(">SetRangeInFbo\n");
		printf("mouse : (%7.2f, %7.2f) - (%7.2f, %7.2f)\n", mouse_dragged_from_.x, mouse_dragged_from_.y, mouse_dragged_to_.x, mouse_dragged_to_.y);
		printf("fbo   : (%7.2f, %7.2f) - (%7.2f, %7.2f)\n", selected_range_in_fbo_from_.x, selected_range_in_fbo_from_.y, selected_range_in_fbo_to_.x, selected_range_in_fbo_to_.y);
		printf("\n");
		fflush(stdout);
	}
}

/******************************
******************************/
void ExtractColor::CheckAndApply_NewRange(glm::vec2 from, glm::vec2 to){
	if( IsInRange(from.x, from.y) && IsInRange(to.x, to.y) ){
		mouse_dragged_from_	= from;
		mouse_dragged_to_	= to;
		
		SetRangeInFbo();
		
	}else{
		printf("move range denied : out of range\n");
		fflush(stdout);
	}
}

/******************************
******************************/
void ExtractColor::MoveRange_Centering(){
	if(state_ != State::Selected)	return;
	
	glm::vec2 from	= mouse_dragged_from_;
	glm::vec2 to 	= mouse_dragged_to_;
	
	float width = to.x - from.x;
	float x_center = kMovDrawedTo_.x + kMovDrawedSize_.x / 2;
	
	from.x	= x_center - width/2;
	to.x	= x_center + width/2;
	
	CheckAndApply_NewRange(from, to);
}

/******************************
******************************/
void ExtractColor::MoveRange_Down(int step){
	if(state_ != State::Selected)	return;
	
	glm::vec2 from	= mouse_dragged_from_;
	glm::vec2 to 	= mouse_dragged_to_;
	
	from.y += step;
	to.y += step;
	
	CheckAndApply_NewRange(from, to);
}

/******************************
******************************/
void ExtractColor::MoveRange_Up(int step){
	if(state_ != State::Selected)	return;
	
	glm::vec2 from	= mouse_dragged_from_;
	glm::vec2 to	= mouse_dragged_to_;
	
	from.y -= step;
	to.y -= step;
	
	CheckAndApply_NewRange(from, to);
}

/******************************
******************************/
void ExtractColor::MoveRange_Right(int step){
	if(state_ != State::Selected)	return;
	
	glm::vec2 from	= mouse_dragged_from_;
	glm::vec2 to 	= mouse_dragged_to_;
	
	from.x += step;
	to.x += step;
	
	CheckAndApply_NewRange(from, to);
}

/******************************
******************************/
void ExtractColor::MoveRange_Left(int step){
	if(state_ != State::Selected)	return;
	
	glm::vec2 from	= mouse_dragged_from_;
	glm::vec2 to 	= mouse_dragged_to_;
	
	from.x -= step;
	to.x -= step;
	
	CheckAndApply_NewRange(from, to);
}

/******************************
******************************/
void ExtractColor::PrepAndSendUdp(){
	/********************
	********************/
	string message = "/ExtractColorInfo,";
	
	const int kBufSize = 100;
	char buf[kBufSize];
	
	if( (int)Gui_Global->col_sync_type == 0 ){
		float hue_near		= Gui_Global->default_hue_near/255;
		float hue_far		= Gui_Global->default_hue_far/255;
		float saturation	= Gui_Global->default_saturation/255;
		float brightness 	= Gui_Global->default_brightness/255;
		
		snprintf(buf, std::size(buf), "%f,%f,%f,%f", hue_near, hue_far, saturation, brightness);
	}else{
		float h_near = (float)(result_col_.getHue())/255;
		float h_far = h_near + 0.1;
		if(1.0 < h_far) h_far = 1.0;
		
		snprintf(buf, std::size(buf), "%f,%f,%f,%f", h_near, h_far, (float)(result_col_.getSaturation())/255, (float)(result_col_.getBrightness())/255);
	}
	
	message += buf;
	
	udp_send_sync_color_.Send(message.c_str(), message.length());
}

/******************************
******************************/
void ExtractColor::SetResultColor(const ofColor & read_col){
	switch((int)Gui_Global->col_sync_type){
		case 0:	// no sync
		case 1:	// hue only
		{
			float hue			= Gui_Global->a_col_fade * read_col.getHue() + (1 - Gui_Global->a_col_fade) * result_col_.getHue();
			float saturation	= Gui_Global->default_saturation;
			float brightness	= Gui_Global->default_brightness;
			
			result_col_.setHsb(hue, saturation, brightness);
		}
			break;
			
		case 2:	// hue brightness
		{
			float hue			= Gui_Global->a_col_fade * read_col.getHue()		+ (1 - Gui_Global->a_col_fade) * result_col_.getHue();
			float saturation	= Gui_Global->default_saturation;
			float brightness	= Gui_Global->a_col_fade * read_col.getBrightness()	+ (1 - Gui_Global->a_col_fade) * result_col_.getBrightness();
			result_col_.setHsb(hue, saturation, brightness);
		}
			break;
			
		case 3:	// hue saturation
		{
			float hue			= Gui_Global->a_col_fade * read_col.getHue()		+ (1 - Gui_Global->a_col_fade) * result_col_.getHue();
			float saturation	= Gui_Global->a_col_fade * read_col.getSaturation()	+ (1 - Gui_Global->a_col_fade) * result_col_.getSaturation();
			float brightness	= Gui_Global->default_brightness;
			
			result_col_.setHsb(hue, saturation, brightness);
		}
			break;
			
		case 4:
		{
			my_HSB hsb_from;
			result_col_.getHsb(hsb_from.h, hsb_from.s, hsb_from.b);
			
			my_HSB hsb_to;
			read_col.getHsb(hsb_to.h, hsb_to.s, hsb_to.b);
			
			hsb_from.Fade(hsb_to, Gui_Global->a_col_fade);
			
			result_col_.setHsb(hsb_from.h, hsb_from.s, hsb_from.b);
		}
			break;
			
		case 5:
			result_col_ = Gui_Global->a_col_fade * read_col + (1 - Gui_Global->a_col_fade) * result_col_;
			break;
			
		default:
			break;
	}
}

/******************************
******************************/
void ExtractColor::TryToReceiveUdpMessage(){
	/********************
	********************/
	char udp_message[kUdpBufSize_];
	
	udp_receive_.Receive(udp_message, kUdpBufSize_);
	string str_message = udp_message;
	
	/********************
	********************/
	while(str_message!=""){
		vector<string> block = ofSplitString(str_message, ",");
		
		if( (2 <= block.size()) && (block[0] == "/BoostLed") ){
			StateChart_BoostLed( atoi(block[1].c_str()) );
		}
		
		/********************
		********************/
		udp_receive_.Receive(udp_message, kUdpBufSize_);
		str_message = udp_message;
	}
}

/******************************
******************************/
void ExtractColor::StateChart_BoostLed(int boost_led_type_id){
	switch(state_boost_led){
		case State_BoostLed::Normal:
			if( (boost_led_type_id == 2)/* strobe */ || (boost_led_type_id == 3)/* half */ || (boost_led_type_id == 4)/* full */ ){
				col_sync_type_saved_ = Gui_Global->col_sync_type;
				state_boost_led = State_BoostLed::Boost;
				
				if( (Gui_Global->col_sync_type == 0) || (Gui_Global->col_sync_type == 1) ){
					// no change.
				}else{
					Gui_Global->col_sync_type = 1;
				}
			}
			
			break;
			
		case State_BoostLed::Boost:
			if( (boost_led_type_id == 0)/* normal */ || (boost_led_type_id == 1)/* FadeToNormal */ || (boost_led_type_id == 5)/* off */ ){
				Gui_Global->col_sync_type = col_sync_type_saved_;
				state_boost_led = State_BoostLed::Normal;
			}
			break;
	}
}

/******************************
******************************/
void ExtractColor::Update(ofFbo & fbo){
	/********************
	********************/
	if(state_ != State::Selected){
		PrepAndSendUdp();
		TryToReceiveUdpMessage();
		return;
	}
	
	/********************
	********************/
	fbo.readToPixels(pixels_);
	ofColor read_col;
	read_col.setHsb(0, 0, 0);
	
	for( int y =  (int)selected_range_in_fbo_from_.y; y <= (int)selected_range_in_fbo_to_.y; y++ ){
		for( int x =  (int)selected_range_in_fbo_from_.x; x <= (int)selected_range_in_fbo_to_.x; x++ ){
			ofColor col = pixels_.getColor(x, y);
			if( read_col.getSaturation() < col.getSaturation() ){
				read_col = col;
			}
		}
	}
	
	/********************
	********************/
	SetResultColor(read_col);
	
	PrepAndSendUdp();
	
	TryToReceiveUdpMessage();
}

/******************************
******************************/
void ExtractColor::Draw(){
	ofPushStyle();
		
		// 選択範囲
		if(state_ == State::Selecting){
			ofNoFill();
			ofSetColor(0, 255, 0, 150);
			ofDrawRectangle( mouse_dragged_from_.x, mouse_dragged_from_.y, mouse_dragged_to_.x - mouse_dragged_from_.x, mouse_dragged_to_.y - mouse_dragged_from_.y );
			
		}else if(state_ == State::Selected){
			ofNoFill();
			ofSetColor(0, 255, 0, 200);
			ofDrawRectangle( mouse_dragged_from_.x, mouse_dragged_from_.y, mouse_dragged_to_.x - mouse_dragged_from_.x, mouse_dragged_to_.y - mouse_dragged_from_.y );
		}
		
		// sending color
		ofSetColor(result_col_.r, result_col_.g, result_col_.b);
		ofFill();
		ofDrawRectangle(kResultDrawTo_.x, kResultDrawTo_.y, kResultDrawSize_.x, kResultDrawSize_.y);
		
		// col_sync_type
		{
			if(state_boost_led == State_BoostLed::Boost)	{ ofSetColor(255, 0, 0); }
			else											{ ofSetColor(255); }
			
			char buf[500];
			switch((int)Gui_Global->col_sync_type){
				case 0:
					snprintf(buf, std::size(buf), "no sync");
					break;
				case 1:
					snprintf(buf, std::size(buf), "hue");
					break;
				case 2:
					snprintf(buf, std::size(buf), "hb");
					break;
				case 3:
					snprintf(buf, std::size(buf), "hs");
					break;
				case 4:
					snprintf(buf, std::size(buf), "hsb fade by hsb");
					break;
				case 5:
					snprintf(buf, std::size(buf), "hsb fade by rgb");
					break;
				default:
					snprintf(buf, std::size(buf), "unknown");
					break;
			}
			
			font_M_.drawString(buf, 672, 223);
		}
		
		// disp : not selected
		if(state_ == State::WaitInput){
			ofSetColor(255);
			
			char buf[500];
			snprintf(buf, std::size(buf), "not selected");
			font_M_.drawString(buf, 670, 640);
			
			
			ofSetColor(255, 0, 0);
			ofNoFill();
			ofSetLineWidth(2);
			ofDrawLine(670, 660, 750, 740);
			ofDrawLine(670, 740, 750, 660);
		}
	
	ofPopStyle();
}
