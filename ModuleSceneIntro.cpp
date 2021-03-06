#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "PugiXml\src\pugixml.hpp"
#include "Time.h"
#include <Random>



ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	srand(time(nullptr));

	App->window->SetTitle("Infinity Racing");

	countdown.Start();
	added_time = 0;

	LoadSegments();
	LoadRecord();

	App->camera->LookAt(App->player->pos);

	prev_base_pos = 0;

	AddRoadSegment(false);
	for (uint i = 0; i < view_distance_segments; i++) {
		AddRoadSegment();
	}

	segments_completed = 0;

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	SetRecord();
	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	float camera_speed = 5.0f;
	
	curr_time = (20 - countdown.ReadSec() + added_time);

	if (App->player->pos.y < -30 || App->input->GetKey(SDL_SCANCODE_R) == KEY_STATE::KEY_DOWN || curr_time <= 0)
	{
		App->Restart();
		App->player->in_intro = true;
		
	}

	vec3 diff_pos;
	diff_pos.p -= 20.0f;
	diff_pos.y += 8.0f;
	vec3 camera_pos = App->player->pos;
	float player_y_rot, player_x_rot, player_z_rot;
	App->player->vehicle->GetRotation(player_x_rot, player_y_rot, player_z_rot);
	diff_pos = rotate(diff_pos, player_x_rot * 180 / M_PI, {0, 1, 0});

	camera_pos += diff_pos;

	if (segments_completed > record)
	{
		record = segments_completed;
	}

	App->camera->Move(normalize(camera_pos - App->camera->Position) * dt * camera_speed * length(camera_pos - App->camera->Position));
	//App->camera->Position.y = App->player->pos.y + 8.0f;
	App->camera->LookAt(App->player->pos);
	
	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	if (body1->s_type != NONE && body2->s_type != NONE) {
		LOG("Sensor hit!");
		if (body1->collision_listeners.getFirst()->data == this) {
			body1->Destroy();
			body1->SetAsSensor(false);
			added_time += 1;
			segments_completed++;
		}
		else {
			body2->Destroy();
			body2->SetAsSensor(false);
			added_time += 1;
			segments_completed++;
		}
		AddRoadSegment();
	}
}

void ModuleSceneIntro::AddRoadSegment(bool obstacles)
{
	int segment_index = rand() % segments.count();

	SegmentInfo info;
	info = segments.at(segment_index)->data;

	Cube base, left, right;
	base.color = White;
	base.size = { segment_width, 0, segment_distance };
	base.SetPos(0, 0, prev_base_pos);
	App->physics->AddBody(base, 0.0f);

	left.size = { 20, 50, segment_distance };
	left.SetPos(segment_width / 2, -6, prev_base_pos);
	left.SetRotation(300, { 0,0,1 });
	App->physics->AddBody(left, 0.0f);

	right.size = { 20, 50, segment_distance };
	right.SetPos(-segment_width / 2, -6, prev_base_pos);
	right.SetRotation(60, { 0,0,1 });
	App->physics->AddBody(right, 0.0f);

	Cube s;
	s.size = vec3(segment_width + 80, 120, 1);
	s.SetPos(0, 60, prev_base_pos + segment_distance / 2);
	s.color.Set(1.0f, 1.0f, 1.0f, 2.0f);
	sensor = App->physics->AddBody(s, 0.0f);
	sensor->SetAsSensor(true);
	sensor->collision_listeners.add(this);
	sensor->SetVisible(false);
	sensor->s_type = LIMIT;
	
	if (obstacles) {
		App->physics->AddSensor({ (float)(rand() % (int)(segment_width - segment_width/2)), 3.0f, (float)(rand() % (int)segment_distance - (segment_distance / 2)) + prev_base_pos }, (rand()%2));

		for (uint i = 0; i < info.num_obstacles; i++) {
			Cube c;
			Sphere s;
			Cylinder cy;
			PhysBody3D* b1 = nullptr, *b2 = nullptr;
			ObstacleInfo o_info = info.obstacles[i];
			switch (o_info.type) {
			case WALL:
				c.color = Red;
				c.size = o_info.dims;
				c.SetPos(o_info.pos.x, o_info.pos.y, o_info.pos.z + prev_base_pos);
				c.SetRotation(o_info.rotation.x, { 1,0,0 });
				c.SetRotation(o_info.rotation.y, { 0,1,0 });
				c.SetRotation(o_info.rotation.z, { 0,0,1 });
				App->physics->AddBody(c, 0.0f);
				break;
			case BOULDER:
				s.color = Red;
				s.SetPos(o_info.pos.x, o_info.pos.y, o_info.pos.z + prev_base_pos);
				s.SetRotation(o_info.rotation.x, { 1,0,0 });
				s.SetRotation(o_info.rotation.y, { 0,1,0 });
				s.SetRotation(o_info.rotation.z, { 0,0,1 });
				s.radius = o_info.radius;
				App->physics->AddBody(s, 10000.0f);
				break;
			case CYLINDER:
				cy.color = Red;
				cy.height = o_info.dims.y;
				cy.radius = o_info.radius;
				cy.SetPos(o_info.pos.x, o_info.pos.y, o_info.pos.z + prev_base_pos);
				cy.SetRotation(o_info.rotation.x, { 1,0,0 });
				cy.SetRotation(o_info.rotation.y, { 0,1,0 });
				cy.SetRotation(o_info.rotation.z, { 0,0,1 });
				App->physics->AddBody(cy, 10000.0f);
				break;
			case PENDULUM:
				s.color = Red;
				s.SetPos(o_info.pos.x, o_info.pos.y, o_info.pos.z + prev_base_pos);
				s.SetRotation(o_info.rotation.x, { 1,0,0 });
				s.SetRotation(o_info.rotation.y, { 0,1,0 });
				s.SetRotation(o_info.rotation.z, { 0,0,1 });
				s.radius = o_info.radius;
				b1 = App->physics->AddBody(s, 10000.0f);

				c.size = { 1,1,1 };
				c.SetPos(0, 50, o_info.pos.z + prev_base_pos);
				b2 = App->physics->AddBody(c, 0.0f);

				App->physics->AddConstraintHinge(*b1, *b2, { 0, 50 - o_info.pos.y - 1, 0 }, { 0, 0, 0 }, { 0,0,1 }, { 0,0,1 }, true);
				break;
			}
		}
	}

	prev_base_pos += segment_distance;
}


void ModuleSceneIntro::LoadRecord()
{
	pugi::xml_document segment_doc;
	pugi::xml_parse_result result = segment_doc.load_file("Record.xml");
	if (result != NULL)
		record = segment_doc.child("Record").attribute("record").as_int(0);
}

void ModuleSceneIntro::SetRecord()
{
	pugi::xml_document segment_doc;

	pugi::xml_node record_node = segment_doc.append_child("Record");
	record_node.append_attribute("record") = record;

	segment_doc.save_file("Record.xml");
}

void ModuleSceneIntro::LoadSegments()
{
	pugi::xml_document segment_doc;
	pugi::xml_parse_result result = segment_doc.load_file("Segments.xml");

	if (result == NULL) {
		LOG("Could not load map xml file Segments.xml. pugi error: %s", result.description());
	}
	else {
		pugi::xml_node segments_list_node = segment_doc.child("Segments");
		segment_distance = segments_list_node.attribute("Length").as_float(100.0f);
		segment_width = segments_list_node.attribute("Width").as_float(80.0f);
		view_distance_segments = segments_list_node.attribute("ViewDistance").as_int(0);
		int j = 0;
		for (pugi::xml_node segment_node : segments_list_node.children()) {
			LOG("Loading segment %d", ++j);
			SegmentInfo info;
			info.rotation.x = segment_node.attribute("RotationX").as_float(0.0f);
			info.rotation.y = segment_node.attribute("RotationY").as_float(0.0f);
			info.rotation.z = segment_node.attribute("RotationZ").as_float(0.0f);

			pugi::xml_node obstacle_info_list_node = segment_node.child("Obstacles");
			info.num_obstacles = obstacle_info_list_node.attribute("Number").as_uint(0);
			if (info.num_obstacles > 0) {
				info.obstacles = (ObstacleInfo*)malloc(info.num_obstacles * sizeof ObstacleInfo);
				uint i = 0;
				for (pugi::xml_node obstacle_node : obstacle_info_list_node.children()) {
					ObstacleInfo o_info;
					o_info.dims.x = obstacle_node.attribute("Width").as_float(0.0f);
					o_info.dims.y = obstacle_node.attribute("Height").as_float(0.0f);
					o_info.dims.z = obstacle_node.attribute("Depth").as_float(0.0f);
					o_info.dynamic = obstacle_node.attribute("Dynamic").as_bool(false);
					o_info.pos.x = obstacle_node.attribute("PosX").as_float(0.0f);
					o_info.pos.y = obstacle_node.attribute("PosY").as_float(0.0f);
					o_info.pos.z = obstacle_node.attribute("PosZ").as_float(0.0f);
					o_info.radius = obstacle_node.attribute("Radius").as_float(0.0f);
					o_info.rotation.x = obstacle_node.attribute("RotationX").as_float(0.0f);
					o_info.rotation.y = obstacle_node.attribute("RotationY").as_float(0.0f);
					o_info.rotation.z = obstacle_node.attribute("RotationZ").as_float(0.0f);
					o_info.type = (ObstacleType)obstacle_node.attribute("Type").as_int(0);
					info.obstacles[i] = o_info;
					LOG("	Loading obstacle %d", ++i);
				}
			}
			segments.add(info);
		}
	}
}

