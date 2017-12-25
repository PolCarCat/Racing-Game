#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"


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

	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(App->player->pos);

	s.size = vec3(5, 3, 1);
	s.SetPos(0, 2.5f, 20);

	//sensor = App->physics->AddBody(s, 0.0f);
	//sensor->SetAsSensor(true);
	//sensor->collision_listeners.add(this);

	Cube random_c;
	random_c.size = { 20 ,40 ,5 };
	random_c.SetPos(0, 0, 50);
	random_c.SetRotation(75, { 1,0,0 });

	left_ramp.size = { 20,50,600 };
	left_ramp.SetPos(60, 0, 0);
	left_ramp.SetRotation(300, { 0,0,1 });
	App->physics->AddBody(left_ramp, 0);


	
	right_ramp.size = { 20,50,600 };
	right_ramp.SetPos(-60, 0, 0);
	right_ramp.SetRotation(60, { 0,0,1 });
	App->physics->AddBody(right_ramp, 0);


	PhysBody3D* random = App->physics->AddBody(random_c, 0);


	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	Plane p(0, 1, 0, 0);
	vec3 camera_pos = App->player->pos;
	camera_pos.p -= 20;
	camera_pos.y = 10;

	left_ramp.Render();
	right_ramp.Render();
	p.axis = true;
	p.wire = true;
	//p.Render();
	App->camera->LookAt(App->player->pos);
	App->camera->Position = camera_pos;
	//sensor->GetTransform(&s.transform);
	//s.Render();

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	LOG("Hit!");
}
