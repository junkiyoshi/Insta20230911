#include "ofApp.h"	

//--------------------------------------------------------------
Actor::Actor(vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	this->select_index = ofRandom(location_list.size());
	while (true) {

		auto itr = find(destination_list.begin(), destination_list.end(), this->select_index);
		if (itr == destination_list.end()) {

			destination_list.push_back(this->select_index);
			break;
		}

		this->select_index = (this->select_index + 1) % location_list.size();
	}

	this->next_index = this->select_index;
}

//--------------------------------------------------------------
void Actor::update(const int& frame_span, vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	if (ofGetFrameNum() % frame_span == 0) {

		auto tmp_index = this->select_index;
		this->select_index = this->next_index;
		int retry = next_index_list[this->select_index].size();
		this->next_index = next_index_list[this->select_index][(int)ofRandom(next_index_list[this->select_index].size())];
		while (--retry > 0) {

			auto destination_itr = find(destination_list.begin(), destination_list.end(), this->next_index);
			if (destination_itr == destination_list.end()) {

				if (tmp_index != this->next_index) {

					destination_list.push_back(this->next_index);
					break;
				}
			}

			this->next_index = next_index_list[this->select_index][(this->next_index + 1) % next_index_list[this->select_index].size()];
		}
		if (retry <= 0) {

			destination_list.push_back(this->select_index);
			this->next_index = this->select_index;
		}
	}

	auto param = ofGetFrameNum() % frame_span;
	auto distance = location_list[this->next_index] - location_list[this->select_index];
	this->location = location_list[this->select_index] + distance / frame_span * param;

	this->log.push_back(this->location);
	while (this->log.size() > 12) { this->log.erase(this->log.begin()); }
}

//--------------------------------------------------------------
glm::vec3 Actor::getLocation() {

	return this->location;
}

//--------------------------------------------------------------
vector<glm::vec3> Actor::getLog() {

	return this->log;
}

//--------------------------------------------------------------
void Actor::setColor(ofColor color) {

	this->color = color;
}

//--------------------------------------------------------------
ofColor Actor::getColor() {

	return this->color;
}

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(25);
	ofSetWindowTitle("openFrameworks");

	ofBackground(239);
	ofSetLineWidth(1.5);
	ofEnableDepthTest();

	int v_span = 3;
	int u_span = 15;
	int R = 331;
	int r = 60;
	for (int v = 0; v <= 360 * 6; v += v_span) {

		auto z = glm::vec3(0, 0, ofMap(v, 0, 360 * 6, -360 * 3, 360 * 3));
		for (int u = 0; u < 360; u += u_span) {

			this->location_list.push_back(this->make_point(R, r, u, v) + z);
		}
	}

	for (auto& location : this->location_list) {

		vector<int> next_index = vector<int>();
		int index = -1;
		for (auto& other : this->location_list) {

			index++;
			if (location == other) { continue; }

			float distance = glm::distance(location, other);
			auto threshold = ofMap(glm::length(glm::vec3(location.x, location.y, 0)), R - r, R + r, 19, 21);
			if (distance <= threshold) {

				next_index.push_back(index);
			}
		}

		this->next_index_list.push_back(next_index);
	}

	ofColor color;
	for (int i = 0; i < 3000; i++) {

		this->actor_list.push_back(make_unique<Actor>(this->location_list, this->next_index_list, this->destination_list));
		color.setHsb(ofRandom(255), 255, 255);
		this->actor_list.back()->setColor(color);
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	int frame_span = 2;
	int prev_index_size = 0;

	if (ofGetFrameNum() % frame_span == 0) {

		prev_index_size = this->destination_list.size();
	}

	for (auto& actor : this->actor_list) {

		actor->update(frame_span, this->location_list, this->next_index_list, this->destination_list);
	}

	if (prev_index_size != 0) {

		this->destination_list.erase(this->destination_list.begin(), this->destination_list.begin() + prev_index_size);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->cam.begin();
	ofRotateY(90);
	ofRotateZ(ofGetFrameNum() * 0.1);

	ofMesh frame;
	frame.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);

	for (auto& actor : this->actor_list) {

		auto index = frame.getNumVertices();
		frame.addVertices(actor->getLog());

		for (int i = index; i < frame.getNumVertices(); i++) {

			frame.addColor(ofColor(128, 128, 255, ofMap(i, index, frame.getNumVertices(), 32, 255)));
			if (i > index) {
			
				frame.addIndex(i); frame.addIndex(i - 1);
			}
		}
	}
	frame.draw();

	ofMesh face;
	int v_span = 3;
	int u_span = 15;
	int R = 330;
	int r = 58;
	for (int v = 0; v <= 360 * 6; v += v_span) {

		auto z = glm::vec3(0, 0, ofMap(v, 0, 360 * 6, -360 * 3, 360 * 3));
		auto next_z = glm::vec3(0, 0, ofMap(v + v_span, 0, 360 * 6, -360 * 3, 360 * 3));
		auto next_u = u_span;
		for (int u = 0; u < 360; u += u_span) {

			face.addVertex(this->make_point(R, r, u, v) + z);
			face.addVertex(this->make_point(R, r, u + u_span, v) + z);
			face.addVertex(this->make_point(R, r, next_u + u_span, v + v_span) + next_z);
			face.addVertex(this->make_point(R, r, next_u, v + v_span) + next_z);

			face.addIndex(face.getNumVertices() - 1); face.addIndex(face.getNumVertices() - 2); face.addIndex(face.getNumVertices() - 3);
			face.addIndex(face.getNumVertices() - 1); face.addIndex(face.getNumVertices() - 3); face.addIndex(face.getNumVertices() - 4);

			next_u += u_span;
		}
	}

	ofSetColor(239);
	face.draw();

	this->cam.end();
}

//--------------------------------------------------------------
glm::vec3 ofApp::make_point(float R, float r, float u, float v) {

	// 数学デッサン教室 描いて楽しむ数学たち　P.31

	u *= DEG_TO_RAD;
	v *= DEG_TO_RAD;

	auto x = (R + r * cos(u)) * cos(v);
	auto y = (R + r * cos(u)) * sin(v);
	auto z = r * sin(u);

	return glm::vec3(x, y, z);
}


//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}